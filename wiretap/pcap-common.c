/* pcap-common.c
 * Code common to pcap and pcapng file formats
 *
 * Wiretap Library
 * Copyright (c) 1998 by Gilbert Ramirez <gram@alumni.rice.edu>
 *
 * File format support for pcapng file format
 * Copyright (c) 2007 by Ulf Lamping <ulf.lamping@web.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"
#include "pcap-common.h"

#include <stdlib.h>
#include <string.h>
#include "wtap-int.h"
#include "file_wrappers.h"
#include "atm.h"
#include "erf_record.h"
#include "pcap-encap.h"

#include <wsutil/ws_roundup.h>

/*
 * On some systems, the FDDI MAC addresses are bit-swapped.
 *
 * XXX - what we *really* need to know is whether the addresses are
 * bit-swapped *in a particular capture*, which depends on the system
 * on which it was captured, not on the system that's reading it.
 * Unfortunately, we can't determine that.
 */
#if !defined(ultrix) && !defined(__alpha) && !defined(__bsdi__)
#define BIT_SWAPPED_MAC_ADDRS
#endif

/*
 * Map link-layer header types (LINKTYPE_ values) to Wiretap encapsulations.
 *
 * Either LBL NRG wasn't an adequate central registry (e.g., because of
 * the slow rate of releases from them), or nobody bothered using them
 * as a central registry, as many different groups have patched libpcap
 * (and BPF, on the BSDs) to add new encapsulation types, and have ended
 * up using the same DLT_ values for different encapsulation types.
 *
 * The Tcpdump Group now maintains the list of link-layer header types;
 * they introduced a separate namespace of LINKTYPE_ values for the
 * values to be used in capture files, and have libpcap map between
 * those values in capture file headers and the DLT_ values that the
 * pcap_datalink() and pcap_open_dead() APIs use.  See
 * https://www.tcpdump.org/linktypes.html for a list of LINKTYPE_ values.
 *
 * In most cases, the corresponding LINKTYPE_ and DLT_ values are the
 * same.  In the cases where the same link-layer header type was given
 * different values in different OSes, a new LINKTYPE_ value was defined,
 * different from all of the existing DLT_ values.
 *
 * This table maps LINKTYPE_ values to the corresponding Wiretap
 * encapsulation.  For cases where multiple DLT_ values were in use,
 * it also checks what <pcap.h> defineds to determine how to interpret
 * them, so that if a file was written by a version of libpcap prior
 * to the introduction of the LINKTYPE_ values, and has a DLT_ value
 * from the OS on which it was written rather than a LINKTYPE_ value
 * as its linktype value in the file header, we map the numerical
 * DLT_ value, as interpreted by the libpcap with which we're building
 * Wireshark/Wiretap interprets them (which, if it doesn't support
 * them at all, means we don't support them either - any capture files
 * using them are foreign, and we don't hazard a guess as to which
 * platform they came from; we could, I guess, choose the most likely
 * platform), to the corresponding Wiretap encapsulation.
 *
 * Note: if you need a new encapsulation type for libpcap files, do
 * *N*O*T* use *ANY* of the values listed here!  I.e., do *NOT*
 * add a new encapsulation type by changing an existing entry;
 * leave the existing entries alone.
 *
 * Instead, send mail to tcpdump-workers@lists.tcpdump.org, asking for
 * a new LINKTYPE_/DLT_ value, and specifying the purpose of the new
 * value.  When you get the new LINKTYPE_/DLT_ value, use that numerical
 * value in the "linktype_value" field of "pcap_to_wtap_map[]".
 */

static const struct {
	int	linktype_value;
	int	wtap_encap_value;
} pcap_to_wtap_map[] = {
	/*
	 * These are the values that are almost certainly the same
	 * in all libpcaps (I've yet to find one where the values
	 * in question are used for some purpose other than the
	 * one below, but...), and thus assigned as LINKTYPE_ values,
	 * and that Wiretap and Wireshark currently support.
	 */
	{ 0,		WTAP_ENCAP_NULL },	/* null encapsulation */
	{ 1,		WTAP_ENCAP_ETHERNET },
	{ 2,		WTAP_ENCAP_3MB_ETHERNET },
	{ 3,		WTAP_ENCAP_AX25 },
	{ 6,		WTAP_ENCAP_TOKEN_RING },	/* IEEE 802 Networks - assume token ring */
	{ 7,		WTAP_ENCAP_ARCNET },
	{ 8,		WTAP_ENCAP_SLIP },
	{ 9,		WTAP_ENCAP_PPP },
#ifdef BIT_SWAPPED_MAC_ADDRS
	{ 10,		WTAP_ENCAP_FDDI_BITSWAPPED },
#else
	{ 10,		WTAP_ENCAP_FDDI },
#endif

	{ 32,		WTAP_ENCAP_REDBACK },

	/*
	 * 50 is DLT_PPP_SERIAL in NetBSD; it appears that DLT_PPP
	 * on BSD (at least according to standard tcpdump) has, as
	 * the first octet, an indication of whether the packet was
	 * transmitted or received (rather than having the standard
	 * PPP address value of 0xff), but that DLT_PPP_SERIAL puts
	 * a real live PPP header there, or perhaps a Cisco PPP header
	 * as per section 4.3.1 of RFC 1547 (implementations of this
	 * exist in various BSDs in "sys/net/if_spppsubr.c", and
	 * I think also exist either in standard Linux or in
	 * various Linux patches; the implementations show how to handle
	 * Cisco keepalive packets).
	 *
	 * However, I don't see any obvious place in FreeBSD "if_ppp.c"
	 * where anything other than the standard PPP header would be
	 * passed up.  I see some stuff that sets the first octet
	 * to 0 for incoming and 1 for outgoing packets before applying
	 * a BPF filter to see whether to drop packets whose protocol
	 * field has the 0x8000 bit set, i.e. network control protocols -
	 * those are handed up to userland - but that code puts the
	 * address field back before passing the packet up.
	 *
	 * I also don't see anything immediately obvious that munges
	 * the address field for sync PPP, either.
	 *
	 * Wireshark currently assumes that if the first octet of a
	 * PPP frame is 0xFF, it's the address field and is followed
	 * by a control field and a 2-byte protocol, otherwise the
	 * address and control fields are absent and the frame begins
	 * with a protocol field.  If we ever see a BSD/OS PPP
	 * capture, we'll have to handle it differently, and we may
	 * have to handle standard BSD captures differently if, in fact,
	 * they don't have 0xff 0x03 as the first two bytes - but, as per
	 * the two paragraphs preceding this, it's not clear that
	 * the address field *is* munged into an incoming/outgoing
	 * field when the packet is handed to the BPF device.
	 *
	 * For now, we just map DLT_PPP_SERIAL to WTAP_ENCAP_PPP, as
	 * we treat WTAP_ENCAP_PPP packets as if those beginning with
	 * 0xff have the standard RFC 1662 "PPP in HDLC-like Framing"
	 * 0xff 0x03 address/control header, and DLT_PPP_SERIAL frames
	 * appear to contain that unless they're Cisco frames (if we
	 * ever see a capture with them, we'd need to implement the
	 * RFC 1547 stuff, and the keepalive protocol stuff).
	 *
	 * We may have to distinguish between "PPP where if it doesn't
	 * begin with 0xff there's no HDLC encapsulation and the frame
	 * begins with the protocol field" (which is how we handle
	 * WTAP_ENCAP_PPP now) and "PPP where there's either HDLC
	 * encapsulation or Cisco PPP" (which is what DLT_PPP_SERIAL
	 * is) at some point.
	 *
	 * XXX - NetBSD has DLT_HDLC, which appears to be used for
	 * Cisco HDLC.  Ideally, they should use DLT_PPP_SERIAL
	 * only for real live HDLC-encapsulated PPP, not for Cisco
	 * HDLC.
	 */
	{ 50,		WTAP_ENCAP_PPP },

	/*
	 * Used by NetBSD and OpenBSD pppoe(4).
	 */
	{ 51,		WTAP_ENCAP_PPP_ETHER },

	/*
	 * Apparently used by the Axent Raptor firewall (now Symantec
	 * Enterprise Firewall).
	 * Thanks, Axent, for not reserving that type with tcpdump.org
	 * and not telling anybody about it.
	 */
	{ 99,		WTAP_ENCAP_SYMANTEC },

	/*
	 * These are the values that libpcap 0.5 and later use in
	 * capture file headers, in an attempt to work around the
	 * confusion decried above, and that Wiretap and Wireshark
	 * currently support.  I.e., they're the LINKTYPE_ values
	 * for RFC 1483 ATM and "raw IP", respectively, not the
	 * DLT_ values for them on all platforms.
	 */
	{ 100,		WTAP_ENCAP_ATM_RFC1483 },
	{ 101,		WTAP_ENCAP_RAW_IP },
#if 0
	/*
	 * More values used by libpcap 0.5 as DLT_ values and used by the
	 * current CVS version of libpcap in capture file headers.
	 * They are not yet handled in Wireshark.
	 * If we get a capture that contains them, we'll implement them.
	 */
	{ 102,		WTAP_ENCAP_SLIP_BSDOS },
	{ 103,		WTAP_ENCAP_PPP_BSDOS },
#endif

	/*
	 * These ones are handled in Wireshark, though.
	 */
	{ 104,		WTAP_ENCAP_CHDLC },	/* Cisco HDLC */
	{ 105,		WTAP_ENCAP_IEEE_802_11 }, /* IEEE 802.11 */
	{ 106,		WTAP_ENCAP_LINUX_ATM_CLIP },
	{ 107,		WTAP_ENCAP_FRELAY },	/* Frame Relay */
	{ 108,		WTAP_ENCAP_LOOP },	/* OpenBSD loopback */
	{ 109,		WTAP_ENCAP_ENC },	/* OpenBSD IPSEC enc */
#if 0
	{ 110,		WTAP_ENCAP_LANE_802_3 },/* ATM LANE 802.3 */
	{ 111,		WTAP_ENCAP_HIPPI },	/* NetBSD HIPPI */
#endif
	{ 112,		WTAP_ENCAP_CHDLC },	/* NetBSD HDLC framing */

	/*
	 * Linux "cooked mode" captures, used by the current CVS version
	 * of libpcap
	 * OR
	 * it could be a packet in Cisco's ERSPAN encapsulation which uses
	 * this number as well (why can't people stick to protocols when it
	 * comes to allocating/using DLT types).
	 */
	{ 113,		WTAP_ENCAP_SLL },	/* Linux cooked capture v1 */

	{ 114,		WTAP_ENCAP_LOCALTALK },	/* Localtalk */

	/*
	 * The tcpdump.org version of libpcap uses 117, rather than 17,
	 * for OpenBSD packet filter logging, so as to avoid conflicting
	 * with DLT_LANE8023 in SuSE 6.3 libpcap.
	 */
	{ 117,		WTAP_ENCAP_PFLOG },

	{ 118,		WTAP_ENCAP_CISCO_IOS },
	{ 119,		WTAP_ENCAP_IEEE_802_11_PRISM }, /* 802.11 plus Prism monitor mode radio header */
	{ 121,		WTAP_ENCAP_HHDLC },	/* HiPath HDLC */
	{ 122,		WTAP_ENCAP_IP_OVER_FC },   /* RFC 2625 IP-over-FC */
	{ 123,		WTAP_ENCAP_ATM_PDUS },  /* SunATM */
	{ 127,		WTAP_ENCAP_IEEE_802_11_RADIOTAP },  /* 802.11 plus radiotap radio header */
	{ 128,		WTAP_ENCAP_TZSP },	/* Tazmen Sniffer Protocol */
	{ 129,		WTAP_ENCAP_ARCNET_LINUX },
	{ 130,		WTAP_ENCAP_JUNIPER_MLPPP }, /* Juniper MLPPP on ML-, LS-, AS- PICs */
	{ 131,		WTAP_ENCAP_JUNIPER_MLFR }, /* Juniper MLFR (FRF.15) on ML-, LS-, AS- PICs */
	{ 133,		WTAP_ENCAP_JUNIPER_GGSN},
	/*
	 * Values 132 and 134 not listed here are reserved for use
	 * in Juniper hardware.
	 */
	{ 135,		WTAP_ENCAP_JUNIPER_ATM2 }, /* various encapsulations captured on the ATM2 PIC */
	{ 136,		WTAP_ENCAP_JUNIPER_SVCS }, /* various encapsulations captured on the services PIC */
	{ 137,		WTAP_ENCAP_JUNIPER_ATM1 }, /* various encapsulations captured on the ATM1 PIC */

	{ 138,		WTAP_ENCAP_APPLE_IP_OVER_IEEE1394 },
						/* Apple IP-over-IEEE 1394 */

	{ 139,		WTAP_ENCAP_MTP2_WITH_PHDR },
	{ 140,		WTAP_ENCAP_MTP2 },
	{ 141,		WTAP_ENCAP_MTP3 },
	{ 142,		WTAP_ENCAP_SCCP },
	{ 143,		WTAP_ENCAP_DOCSIS },
	{ 144,		WTAP_ENCAP_IRDA },	/* IrDA capture */

	/* Reserved for private use. */
	{ 147,		WTAP_ENCAP_USER0 },
	{ 148,		WTAP_ENCAP_USER1 },
	{ 149,		WTAP_ENCAP_USER2 },
	{ 150,		WTAP_ENCAP_USER3 },
	{ 151,		WTAP_ENCAP_USER4 },
	{ 152,		WTAP_ENCAP_USER5 },
	{ 153,		WTAP_ENCAP_USER6 },
	{ 154,		WTAP_ENCAP_USER7 },
	{ 155,		WTAP_ENCAP_USER8 },
	{ 156,		WTAP_ENCAP_USER9 },
	{ 157,		WTAP_ENCAP_USER10 },
	{ 158,		WTAP_ENCAP_USER11 },
	{ 159,		WTAP_ENCAP_USER12 },
	{ 160,		WTAP_ENCAP_USER13 },
	{ 161,		WTAP_ENCAP_USER14 },
	{ 162,		WTAP_ENCAP_USER15 },

	{ 163,		WTAP_ENCAP_IEEE_802_11_AVS },  /* 802.11 plus AVS radio header */

	/*
	 * 164 is reserved for Juniper-private chassis-internal
	 * meta-information such as QoS profiles, etc..
	 */

	{ 165,		WTAP_ENCAP_BACNET_MS_TP },

	/*
	 * 166 is reserved for a PPP variant in which the first byte
	 * of the 0xff03 header, the 0xff, is replaced by a direction
	 * byte.  I don't know whether any captures look like that,
	 * but it is used for some Linux IP filtering (ipfilter?).
	 */

	/* Ethernet PPPoE frames captured on a service PIC */
	{ 167,		WTAP_ENCAP_JUNIPER_PPPOE },

	/*
	 * 168 is reserved for more Juniper private-chassis-
	 * internal meta-information.
	 */

	{ 169,		WTAP_ENCAP_GPRS_LLC },

	/* ITU-T G.7041/Y.1303 Generic Framing Procedure. */
	{ 170,		WTAP_ENCAP_GFP_T },
	{ 171,		WTAP_ENCAP_GFP_F },

	/* Registered by Gcom, Inc. */
	{ 172,		WTAP_ENCAP_GCOM_TIE1 },
	{ 173,		WTAP_ENCAP_GCOM_SERIAL },

	{ 177,		WTAP_ENCAP_LINUX_LAPD },

	/* Ethernet frames prepended with meta-information */
	{ 178,		WTAP_ENCAP_JUNIPER_ETHER },
	/* PPP frames prepended with meta-information */
	{ 179,		WTAP_ENCAP_JUNIPER_PPP },
	/* Frame-Relay frames prepended with meta-information */
	{ 180,		WTAP_ENCAP_JUNIPER_FRELAY },
	/* C-HDLC frames prepended with meta-information */
	{ 181,		WTAP_ENCAP_JUNIPER_CHDLC },
	/* VOIP Frames prepended with meta-information */
	{ 183,		WTAP_ENCAP_JUNIPER_VP },
	/* Virtual Network Frames prepended with meta-information */
	{ 184,		WTAP_ENCAP_JUNIPER_VN },
	/* USB packets from FreeBSD's USB BPF tap */
	{ 186,		WTAP_ENCAP_USB_FREEBSD },
	/* Bluetooth HCI UART transport (part H:4) frames, like hcidump */
	{ 187,		WTAP_ENCAP_BLUETOOTH_H4 },
	/* IEEE 802.16 MAC Common Part Sublayer */
	{ 188,		WTAP_ENCAP_IEEE802_16_MAC_CPS },
	/* USB packets with Linux-specified header */
	{ 189,		WTAP_ENCAP_USB_LINUX },
	/* CAN 2.0b frame */
	{ 190,		WTAP_ENCAP_CAN20B },
	/* Per-Packet Information header */
	{ 192,		WTAP_ENCAP_PPI },
	/* IEEE 802.15.4 Wireless PAN */
	{ 195,		WTAP_ENCAP_IEEE802_15_4 },
	/* SITA File Encapsulation */
	{ 196,		WTAP_ENCAP_SITA },
	/* Endace Record File Encapsulation */
	{ 197,		WTAP_ENCAP_ERF },
	/* IPMB/I2C with Kontron pseudo-header */
	{ 199,		WTAP_ENCAP_IPMB_KONTRON },
	/* Juniper-private data link type, used for capturing data on a secure tunnel interface. */
	{ 200,		WTAP_ENCAP_JUNIPER_ST },
	/* Bluetooth HCI UART transport (part H:4) frames, like hcidump */
	{ 201,		WTAP_ENCAP_BLUETOOTH_H4_WITH_PHDR },
	/* AX.25 packet with a 1-byte KISS header */
	{ 202,		WTAP_ENCAP_AX25_KISS },
	/* LAPD frame */
	{ 203,		WTAP_ENCAP_LAPD },
	/* PPP with pseudoheader */
	{ 204,		WTAP_ENCAP_PPP_WITH_PHDR },
	/* I2C with a Linux-specific header (defined by Pigeon Point Systems) */
	{ 209,		WTAP_ENCAP_I2C_LINUX },
	/* FlexRay frame */
	{ 210,		WTAP_ENCAP_FLEXRAY },
	/* MOST frame */
	{ 211,		WTAP_ENCAP_MOST },
	/* LIN frame */
	{ 212,		WTAP_ENCAP_LIN },
	/* X2E Xoraya serial frame */
	{ 213,		WTAP_ENCAP_X2E_SERIAL },
	/* X2E Xoraya frame */
	{ 214,		WTAP_ENCAP_X2E_XORAYA },
	/* IEEE 802.15.4 Wireless PAN non-ASK PHY */
	{ 215,		WTAP_ENCAP_IEEE802_15_4_NONASK_PHY },
	/* USB packets with padded Linux-specified header */
	{ 220,		WTAP_ENCAP_USB_LINUX_MMAPPED },
	/* Fibre Channel FC-2 frame */
	{ 224,		WTAP_ENCAP_FIBRE_CHANNEL_FC2 },
	/* Fibre Channel FC-2 frame with Delimiter */
	{ 225,		WTAP_ENCAP_FIBRE_CHANNEL_FC2_WITH_FRAME_DELIMS },
	/* Solaris IPNET */
	{ 226,		WTAP_ENCAP_IPNET },
	/* SocketCAN frame */
	{ 227,		WTAP_ENCAP_SOCKETCAN },
	/* Raw IPv4 */
	{ 228,		WTAP_ENCAP_RAW_IP4 },
	/* Raw IPv6 */
	{ 229,		WTAP_ENCAP_RAW_IP6 },
	/* IEEE 802.15.4 Wireless PAN no fcs */
	{ 230,		WTAP_ENCAP_IEEE802_15_4_NOFCS },
	/* D-BUS */
	{ 231,		WTAP_ENCAP_DBUS },
	/* DVB-CI (Common Interface) */
	{ 235,		WTAP_ENCAP_DVBCI },
	/* MUX27010 */
	{ 236,		WTAP_ENCAP_MUX27010 },
	/* STANAG 5066 - DTS(Data Transfer Sublayer) PDU */
	{ 237,		WTAP_ENCAP_STANAG_5066_D_PDU },
	/* NFLOG */
	{ 239,		WTAP_ENCAP_NFLOG },
	/* netANALYZER pseudo-header followed by Ethernet with CRC */
	{ 240,		WTAP_ENCAP_NETANALYZER },
	/* netANALYZER pseudo-header in transparent mode */
	{ 241,		WTAP_ENCAP_NETANALYZER_TRANSPARENT },
	/* IP-over-Infiniband, as specified by RFC 4391 section 6 */
	{ 242,		WTAP_ENCAP_IP_OVER_IB_PCAP },
	/* ISO/IEC 13818-1 MPEG2-TS packets */
	{ 243,		WTAP_ENCAP_MPEG_2_TS },
	/* NFC LLCP */
	{ 245,		WTAP_ENCAP_NFC_LLCP },
	/* SCTP */
	{ 248,		WTAP_ENCAP_SCTP},
	/* USBPcap */
	{ 249,		WTAP_ENCAP_USBPCAP},
	/* RTAC SERIAL */
	{ 250,		WTAP_ENCAP_RTAC_SERIAL},
	/* Bluetooth Low Energy Link Layer */
	{ 251,		WTAP_ENCAP_BLUETOOTH_LE_LL},
	/* Wireshark Upper PDU export */
	{ 252,		WTAP_ENCAP_WIRESHARK_UPPER_PDU},
	/* Netlink Protocol (nlmon devices) */
	{ 253,		WTAP_ENCAP_NETLINK },
	/* Bluetooth Linux Monitor */
	{ 254,		WTAP_ENCAP_BLUETOOTH_LINUX_MONITOR },
	/* Bluetooth BR/EDR Baseband RF captures */
	{ 255,		WTAP_ENCAP_BLUETOOTH_BREDR_BB },
	/* Bluetooth Low Energy Link Layer RF captures */
	{ 256,		WTAP_ENCAP_BLUETOOTH_LE_LL_WITH_PHDR },

	/* Apple PKTAP */
	{ 258,		WTAP_ENCAP_PKTAP },

	/* Ethernet Passive Optical Network */
	{ 259,		WTAP_ENCAP_EPON },

	/* IPMI Trace Data Collection */
	{ 260,		WTAP_ENCAP_IPMI_TRACE },

	/* ISO 14443 contactless smartcard standards */
	{ 264,		WTAP_ENCAP_ISO14443 },

	/* USB packets from Darwin (macOS, iOS) BPF tap */
	{ 266,          WTAP_ENCAP_USB_DARWIN },

	/* IBM SDLC frames containing SNA PDUs */
	{ 268,		WTAP_ENCAP_SDLC },

	/* LoRaTap */
	{ 270,		WTAP_ENCAP_LORATAP },

	/* Linux vsock */
	{ 271,		WTAP_ENCAP_VSOCK },

	/* nRF Sniffer for Bluetooth LE */
	{ 272,		WTAP_ENCAP_NORDIC_BLE },

	/* DOCSIS31 XRA31 Sniffer */
	{ 273,		WTAP_ENCAP_DOCSIS31_XRA31 },

	/* mPackets as specified by 802.3br */
	{ 274,		WTAP_ENCAP_ETHERNET_MPACKET },

	/* DisplayPort AUX channel monitor */
	{ 275,		WTAP_ENCAP_DPAUXMON },

	/* Linux cooked capture v2 */
	{ 276,		WTAP_ENCAP_SLL2 },

	/* Elektrobit High Speed Capture and Replay */
	{ 279,		WTAP_ENCAP_EBHSCR },

	/* VPP dispatch trace */
	{ 280,		WTAP_ENCAP_VPP },

	/* IEEE 802.15.4 TAP */
	{ 283,		WTAP_ENCAP_IEEE802_15_4_TAP },

	/* Z-Wave Serial API */
	{ 287,		WTAP_ENCAP_ZWAVE_SERIAL },

	/* USB 2.0/1.1/1.0 packets as transmitted over the cable */
	{ 288,		WTAP_ENCAP_USB_2_0 },

	/* ATSC Link-Layer Protocol (A/330) packets */
	{ 289,		WTAP_ENCAP_ATSC_ALP },

	/* Event Tracing for Windows records */
	{ 290,		WTAP_ENCAP_ETW },

	/* Serial NCP (Network Co-Processor) protocol for Zigbee stack ZBOSS */
	{ 292,		WTAP_ENCAP_ZBNCP },

	/* USB 2.0/1.1/1.0 packets captured on Low/Full/High speed link */
	{ 293,		WTAP_ENCAP_USB_2_0_LOW_SPEED },
	{ 294,		WTAP_ENCAP_USB_2_0_FULL_SPEED },
	{ 295,		WTAP_ENCAP_USB_2_0_HIGH_SPEED },

	/* Auerswald log file captured from any supported Auerswald device */
	{ 296,		WTAP_ENCAP_AUERSWALD_LOG },

	/* Silicon Labs debug channel */
	{ 298,		WTAP_ENCAP_SILABS_DEBUG_CHANNEL },

	/* Ultra-wideband (UWB) controller interface protocol (UCI) */
	{ 299,		WTAP_ENCAP_FIRA_UCI },

	/* MDB (Multi-Drop Bus) */
	{ 300,		WTAP_ENCAP_MDB },

	/* DECT_NR (DECT-2020 New Radio (NR) MAC layer) */
	{ 301,		WTAP_ENCAP_DECT_NR },

	/*
	 * To repeat:
	 *
	 * If you need a new encapsulation type for pcap and pcapng files,
	 * do *N*O*T* use *ANY* of the values listed here!  I.e., do *NOT*
	 * add a new encapsulation type by changing an existing entry;
	 * leave the existing entries alone.
	 *
	 * Instead, send mail to tcpdump-workers@lists.tcpdump.org, asking
	 * for a new DLT_ value, and specifying the purpose of the new value.
	 * When you get the new DLT_ value, use that numerical value in
	 * the "linktype_value" field of "pcap_to_wtap_map[]".
	 */

	/*
	 * The following are entries for libpcap type values that have
	 * different meanings on different OSes.  I.e., these are DLT_
	 * values that are different on different OSes, and that have
	 * a separate LINKTYPE_ value assigned to them.
	 *
	 * We put these *after* the entries for the LINKTYPE_ values for
	 * those Wiretap encapsulation types, so that, when writing a
	 * pcap or pcapng file, Wireshark writes the LINKTYPE_ value,
	 * not the OS's DLT_ value, as the file's link-layer header type
	 * for pcap or the interface's link-layer header type.
	 */

	/*
	 * 11 is DLT_ATM_RFC1483 on most platforms; the only version of
	 * libpcap I've seen that define anything other than DLT_ATM_RFC1483
	 * as 11 is the BSD/OS one, which defines DLT_FR as 11.  We handle
	 * it as Frame Relay on BSD/OS and LLC-encapsulated ATM on all other
	 * platforms.
	 */
#if defined(__bsdi__) /* BSD/OS */
	{ 11,		WTAP_ENCAP_FRELAY },
#else
	{ 11,		WTAP_ENCAP_ATM_RFC1483 },
#endif

	/*
	 * 12 is DLT_RAW on most platforms, but it's DLT_C_HDLC on
	 * BSD/OS, and DLT_LOOP on OpenBSD.
	 *
	 * We don't yet handle DLT_C_HDLC, but we can handle DLT_LOOP
	 * (it's just like DLT_NULL, only with the AF_ value in network
	 * rather than host byte order - Wireshark figures out the
	 * byte order from the data, so we don't care what byte order
	 * it's in), so, on OpenBSD, interpret 12 as WTAP_ENCAP_LOOP,
	 * otherwise, if we're not on BSD/OS, interpret it as
	 * WTAP_ENCAP_RAW_IP.
	 */
#if defined(__OpenBSD__)
	{ 12,		WTAP_ENCAP_LOOP },
#elif defined(__bsdi__) /* BSD/OS */
	/*
	 * Put entry for Cisco HDLC here.
	 * XXX - is this just WTAP_ENCAP_CHDLC, i.e. does the frame
	 * start with a 4-byte Cisco HDLC header?
	 */
#else
	{ 12,		WTAP_ENCAP_RAW_IP },
#endif

	/*
	 * 13 is DLT_SLIP_BSDOS on FreeBSD and NetBSD, but those OSes
	 * don't actually generate it.  I infer that BSD/OS translates
	 * DLT_SLIP from the kernel BPF code to DLT_SLIP_BSDOS in
	 * libpcap, as the BSD/OS link-layer header is different;
	 * however, in BSD/OS, DLT_SLIP_BSDOS is 15.
	 *
	 * From this, I infer that there's no point in handling 13
	 * as DLT_SLIP_BSDOS.
	 *
	 * 13 is DLT_ATM_RFC1483 on BSD/OS.
	 *
	 * 13 is DLT_ENC in OpenBSD, which is, I suspect, some kind
	 * of decrypted IPsec traffic.
	 *
	 * We treat 13 as WTAP_ENCAP_ENC on all systems except those
	 * that define DLT_ATM_RFC1483 as 13 - presumably only
	 * BSD/OS does so - so that, on BSD/OS systems, we still
	 * treat 13 as WTAP_ENCAP_ATM_RFC1483, but, on all other
	 * systems, we can read OpenBSD DLT_ENC captures.
	 */
#if defined(__bsdi__) /* BSD/OS */
	{ 13,		WTAP_ENCAP_ATM_RFC1483 },
#else
	{ 13,		WTAP_ENCAP_ENC },
#endif

	/*
	 * 14 is DLT_PPP_BSDOS on FreeBSD and NetBSD, but those OSes
	 * don't actually generate it.  I infer that BSD/OS translates
	 * DLT_PPP from the kernel BPF code to DLT_PPP_BSDOS in
	 * libpcap, as the BSD/OS link-layer header is different;
	 * however, in BSD/OS, DLT_PPP_BSDOS is 16.
	 *
	 * From this, I infer that there's no point in handling 14
	 * as DLT_PPP_BSDOS.
	 *
	 * 14 is DLT_RAW on BSD/OS and OpenBSD.
	 */
	{ 14,		WTAP_ENCAP_RAW_IP },

	/*
	 * 15 is:
	 *
	 *	DLT_SLIP_BSDOS on BSD/OS;
	 *
	 *	DLT_HIPPI on NetBSD;
	 *
	 *	DLT_LANE8023 with Alexey Kuznetzov's patches for
	 *	Linux libpcap;
	 *
	 *	DLT_I4L_RAWIP with the ISDN4Linux patches for libpcap
	 *	(and on SuSE 6.3);
	 *
	 * but we don't currently handle any of those.
	 */

	/*
	 * 16 is:
	 *
	 *	DLT_PPP_BSDOS on BSD/OS;
	 *
	 *	DLT_HDLC on NetBSD (Cisco HDLC);
	 *
	 *	DLT_CIP with Alexey Kuznetzov's patches for
	 *	Linux libpcap - this is WTAP_ENCAP_LINUX_ATM_CLIP;
	 *
	 *	DLT_I4L_IP with the ISDN4Linux patches for libpcap
	 *	(and on SuSE 6.3).
	 */
#if defined(__NetBSD__)
	{ 16,		WTAP_ENCAP_CHDLC },
#elif !defined(__bsdi__)
	/*
	 * If you care about the two different Linux interpretations
	 * of 16, fix it yourself.
	 */
	{ 16,		WTAP_ENCAP_LINUX_ATM_CLIP },
#endif

	/*
	 * 17 is DLT_LANE8023 in SuSE 6.3 libpcap; we don't currently
	 * handle it.
	 * It is also used as the PF (Packet Filter) logging format beginning
	 * with OpenBSD 3.0; we use 17 for PF logs on OpenBSD and don't
	 * use it otherwise.
	 */
#if defined(__OpenBSD__)
	{ 17,		WTAP_ENCAP_OLD_PFLOG },
#endif

	/*
	 * 18 is DLT_CIP in SuSE 6.3 libpcap; if it's the same as the
	 * DLT_CIP of 16 that the Alexey Kuznetzov patches for
	 * libpcap/tcpdump define, it's WTAP_ENCAP_LINUX_ATM_CLIP.
	 * I've not found any version of libpcap that uses it for any
	 * other purpose - hopefully nobody will do so in the future.
	 */
	{ 18,		WTAP_ENCAP_LINUX_ATM_CLIP },

	/*
	 * 19 is DLT_ATM_CLIP in the libpcap/tcpdump patches in the
	 * recent versions I've seen of the Linux ATM distribution;
	 * I've not yet found any version of libpcap file that uses it
	 * for any other purpose - hopefully nobody will do so in
	 * the future.
	 */
	{ 19,		WTAP_ENCAP_LINUX_ATM_CLIP },

	/*
	 * To repeat:
	 *
	 * If you need a new encapsulation type for pcap and pcapng files,
	 * do *N*O*T* use *ANY* of the values listed here!  I.e., do *NOT*
	 * add a new encapsulation type by changing an existing entry;
	 * leave the existing entries alone.
	 *
	 * Instead, send mail to tcpdump-workers@lists.tcpdump.org, asking
	 * for a new DLT_ value, and specifying the purpose of the new value.
	 * When you get the new DLT_ value, use that numerical value in
	 * the "linktype_value" field of "pcap_to_wtap_map[]".
	 */
};
#define NUM_PCAP_ENCAPS array_length(pcap_to_wtap_map)

int
wtap_pcap_encap_to_wtap_encap(int encap)
{
	unsigned int i;

	for (i = 0; i < NUM_PCAP_ENCAPS; i++) {
		if (pcap_to_wtap_map[i].linktype_value == encap)
			return pcap_to_wtap_map[i].wtap_encap_value;
	}
	return WTAP_ENCAP_UNKNOWN;
}

int
wtap_wtap_encap_to_pcap_encap(int encap)
{
	unsigned int i;

	switch (encap) {

	case WTAP_ENCAP_FDDI:
	case WTAP_ENCAP_FDDI_BITSWAPPED:
		/*
		 * Special-case WTAP_ENCAP_FDDI and
		 * WTAP_ENCAP_FDDI_BITSWAPPED; both of them get mapped
		 * to DLT_FDDI (even though that may mean that the bit
		 * order in the FDDI MAC addresses is wrong; so it goes
		 * - libpcap format doesn't record the byte order,
		 * so that's not fixable).
		 *
		 * The pcap_to_wtap_map[] table will only have an
		 * entry for one of the above, which is why we have
		 * to special-case them.
		 */
		return 10;	/* that's DLT_FDDI */

	case WTAP_ENCAP_NETTL_FDDI:
		/*
		 * This will discard the nettl information, as that's
		 * in the pseudo-header.
		 *
		 * XXX - what about Ethernet and Token Ring?
		 */
		return 10;	/* that's DLT_FDDI */

	case WTAP_ENCAP_FRELAY_WITH_PHDR:
		/*
		 * This will discard the pseudo-header information.
		 */
		return 107;

	case WTAP_ENCAP_IEEE_802_11_WITH_RADIO:
		/*
		 * Map this to DLT_IEEE802_11, for now, even though
		 * that means the radio information will be lost.
		 * We should try to map those values to radiotap
		 * values and write this out as a radiotap file,
		 * if possible.
		 */
		return 105;
	}

	for (i = 0; i < NUM_PCAP_ENCAPS; i++) {
		if (pcap_to_wtap_map[i].wtap_encap_value == encap)
			return pcap_to_wtap_map[i].linktype_value;
	}
	return -1;
}

/*
 * For most encapsulations, we use WTAP_MAX_PACKET_SIZE_STANDARD, as
 * that should be enough for most link-layer types, and shouldn't be
 * too big.
 *
 * For some link-layer types, we use larger types, because, for each
 * of them, the maximum packet size is larger than the standard
 * maximum, and is bigger than we'd want for all link-layer types - files
 * with that snapshot length might cause some programs reading them to
 * allocate a huge and wasteful buffer and, at least on 32-bit platforms,
 * run the risk of running out of memory.
 */
unsigned
wtap_max_snaplen_for_encap(int wtap_encap)
{
	switch (wtap_encap) {

	case WTAP_ENCAP_DBUS:
		return WTAP_MAX_PACKET_SIZE_DBUS;

	case WTAP_ENCAP_EBHSCR:
		return WTAP_MAX_PACKET_SIZE_EBHSCR;

	case WTAP_ENCAP_USBPCAP:
	case WTAP_ENCAP_USB_LINUX:
	case WTAP_ENCAP_USB_LINUX_MMAPPED:
	case WTAP_ENCAP_USB_DARWIN:
	case WTAP_ENCAP_USB_FREEBSD:
		return WTAP_MAX_PACKET_SIZE_USBPCAP;

	default:
		return WTAP_MAX_PACKET_SIZE_STANDARD;
	}
}

/*
 * Various pseudo-headers that appear at the beginning of packet data.
 *
 * We represent them as sets of offsets, as they might not be aligned on
 * an appropriate structure boundary in the buffer, and as that makes them
 * independent of the way the compiler might align fields.
 */

/*
 * The link-layer header on Nokia IPSO ATM packets.
 */
#define NOKIAATM_FLAGS	0	/* destination - 1 byte */
#define NOKIAATM_VPI	1	/* VPI - 1 byte */
#define NOKIAATM_VCI	2	/* VCI - 2 bytes */
#define NOKIAATM_LEN	4	/* length of the header */

static int
pcap_read_nokiaatm_pseudoheader(FILE_T fh,
    union wtap_pseudo_header *pseudo_header, unsigned packet_size,
    int *err, char **err_info)
{
	uint8_t	atm_phdr[NOKIAATM_LEN];
	uint8_t	vpi;
	uint16_t	vci;

	if (packet_size < NOKIAATM_LEN) {
		/*
		 * Uh-oh, the packet isn't big enough to even
		 * have a pseudo-header.
		 */
		*err = WTAP_ERR_BAD_FILE;
		*err_info = ws_strdup_printf("pcap/pcapng: Nokia IPSO ATM file has a %u-byte packet, too small to have even an ATM pseudo-header",
		    packet_size);
		return -1;
	}
	if (!wtap_read_bytes(fh, atm_phdr, NOKIAATM_LEN, err, err_info))
		return -1;

	vpi = atm_phdr[NOKIAATM_VPI];
	vci = pntoh16(&atm_phdr[NOKIAATM_VCI]);

	pseudo_header->atm.vpi = vpi;
	pseudo_header->atm.vci = vci;
	pseudo_header->atm.channel = (atm_phdr[NOKIAATM_FLAGS] & 0x80) ? 0 : 1;

	/* We don't have this information */
	pseudo_header->atm.flags = 0;
	pseudo_header->atm.cells = 0;
	pseudo_header->atm.aal5t_u2u = 0;
	pseudo_header->atm.aal5t_len = 0;
	pseudo_header->atm.aal5t_chksum = 0;

	return NOKIAATM_LEN;
}

/*
 * The link-layer header on SunATM packets.
 */
#define SUNATM_FLAGS	0	/* destination and traffic type - 1 byte */
#define SUNATM_VPI	1	/* VPI - 1 byte */
#define SUNATM_VCI	2	/* VCI - 2 bytes */
#define SUNATM_LEN	4	/* length of the header */

static int
pcap_read_sunatm_pseudoheader(FILE_T fh,
    union wtap_pseudo_header *pseudo_header, unsigned packet_size,
    int *err, char **err_info)
{
	uint8_t	atm_phdr[SUNATM_LEN];
	uint8_t	vpi;
	uint16_t	vci;

	if (packet_size < SUNATM_LEN) {
		/*
		 * Uh-oh, the packet isn't big enough to even
		 * have a pseudo-header.
		 */
		*err = WTAP_ERR_BAD_FILE;
		*err_info = ws_strdup_printf("pcap/pcapng: SunATM file has a %u-byte packet, too small to have even an ATM pseudo-header",
		    packet_size);
		return -1;
	}
	if (!wtap_read_bytes(fh, atm_phdr, SUNATM_LEN, err, err_info))
		return -1;

	vpi = atm_phdr[SUNATM_VPI];
	vci = pntoh16(&atm_phdr[SUNATM_VCI]);

	switch (atm_phdr[SUNATM_FLAGS] & 0x0F) {

	case 0x01:	/* LANE */
		pseudo_header->atm.aal = AAL_5;
		pseudo_header->atm.type = TRAF_LANE;
		break;

	case 0x02:	/* RFC 1483 LLC multiplexed traffic */
		pseudo_header->atm.aal = AAL_5;
		pseudo_header->atm.type = TRAF_LLCMX;
		break;

	case 0x05:	/* ILMI */
		pseudo_header->atm.aal = AAL_5;
		pseudo_header->atm.type = TRAF_ILMI;
		break;

	case 0x06:	/* Q.2931 */
		pseudo_header->atm.aal = AAL_SIGNALLING;
		pseudo_header->atm.type = TRAF_UNKNOWN;
		break;

	case 0x03:	/* MARS (RFC 2022) */
		pseudo_header->atm.aal = AAL_5;
		pseudo_header->atm.type = TRAF_UNKNOWN;
		break;

	case 0x04:	/* IFMP (Ipsilon Flow Management Protocol; see RFC 1954) */
		pseudo_header->atm.aal = AAL_5;
		pseudo_header->atm.type = TRAF_UNKNOWN;	/* XXX - TRAF_IPSILON? */
		break;

	default:
		/*
		 * Assume it's AAL5, unless it's VPI 0 and VCI 5, in which
		 * case assume it's AAL_SIGNALLING; we know nothing more
		 * about it.
		 *
		 * XXX - is this necessary?  Or are we guaranteed that
		 * all signalling traffic has a type of 0x06?
		 *
		 * XXX - is this guaranteed to be AAL5?  Or, if the type is
		 * 0x00 ("raw"), might it be non-AAL5 traffic?
		 */
		if (vpi == 0 && vci == 5)
			pseudo_header->atm.aal = AAL_SIGNALLING;
		else
			pseudo_header->atm.aal = AAL_5;
		pseudo_header->atm.type = TRAF_UNKNOWN;
		break;
	}
	pseudo_header->atm.subtype = TRAF_ST_UNKNOWN;

	pseudo_header->atm.vpi = vpi;
	pseudo_header->atm.vci = vci;
	pseudo_header->atm.channel = (atm_phdr[SUNATM_FLAGS] & 0x80) ? 0 : 1;

	/* We don't have this information */
	pseudo_header->atm.flags = 0;
	pseudo_header->atm.cells = 0;
	pseudo_header->atm.aal5t_u2u = 0;
	pseudo_header->atm.aal5t_len = 0;
	pseudo_header->atm.aal5t_chksum = 0;

	return SUNATM_LEN;
}

static bool
pcap_write_sunatm_pseudoheader(wtap_dumper *wdh,
    const union wtap_pseudo_header *pseudo_header, int *err)
{
	uint8_t atm_hdr[SUNATM_LEN];

	/*
	 * Write the ATM header.
	 */
	atm_hdr[SUNATM_FLAGS] =
	    (pseudo_header->atm.channel == 0) ? 0x80 : 0x00;
	switch (pseudo_header->atm.aal) {

	case AAL_SIGNALLING:
		/* Q.2931 */
		atm_hdr[SUNATM_FLAGS] |= 0x06;
		break;

	case AAL_5:
		switch (pseudo_header->atm.type) {

		case TRAF_LANE:
			/* LANE */
			atm_hdr[SUNATM_FLAGS] |= 0x01;
			break;

		case TRAF_LLCMX:
			/* RFC 1483 LLC multiplexed traffic */
			atm_hdr[SUNATM_FLAGS] |= 0x02;
			break;

		case TRAF_ILMI:
			/* ILMI */
			atm_hdr[SUNATM_FLAGS] |= 0x05;
			break;
		}
		break;
	}
	atm_hdr[SUNATM_VPI] = (uint8_t)pseudo_header->atm.vpi;
	phtons(&atm_hdr[SUNATM_VCI], pseudo_header->atm.vci);
	if (!wtap_dump_file_write(wdh, atm_hdr, sizeof(atm_hdr), err))
		return false;
	return true;
}

/*
 * The fake link-layer header of IrDA packets as introduced by Jean Tourrilhes
 * to libpcap.
 */
#define IRDA_SLL_PKTTYPE_OFFSET		0	/* packet type - 2 bytes */
/* 12 unused bytes */
#define IRDA_SLL_PROTOCOL_OFFSET	14	/* protocol, should be ETH_P_LAPD - 2 bytes */
#define IRDA_SLL_LEN			16	/* length of the header */

static int
pcap_read_irda_pseudoheader(FILE_T fh, union wtap_pseudo_header *pseudo_header,
    unsigned packet_size, int *err, char **err_info)
{
	uint8_t	irda_phdr[IRDA_SLL_LEN];

	if (packet_size < IRDA_SLL_LEN) {
		/*
		 * Uh-oh, the packet isn't big enough to even
		 * have a pseudo-header.
		 */
		*err = WTAP_ERR_BAD_FILE;
		*err_info = ws_strdup_printf("pcap/pcapng: IrDA file has a %u-byte packet, too small to have even an IrDA pseudo-header",
		    packet_size);
		return -1;
	}
	if (!wtap_read_bytes(fh, irda_phdr, IRDA_SLL_LEN, err, err_info))
		return -1;

	if (pntoh16(&irda_phdr[IRDA_SLL_PROTOCOL_OFFSET]) != 0x0017) {
		*err = WTAP_ERR_BAD_FILE;
		if (err_info != NULL)
			*err_info = g_strdup("pcap/pcapng: IrDA capture has a packet with an invalid sll_protocol field");
		return -1;
	}

	pseudo_header->irda.pkttype = pntoh16(&irda_phdr[IRDA_SLL_PKTTYPE_OFFSET]);

	return IRDA_SLL_LEN;
}

static bool
pcap_write_irda_pseudoheader(wtap_dumper *wdh,
    const union wtap_pseudo_header *pseudo_header, int *err)
{
	uint8_t irda_hdr[IRDA_SLL_LEN];

	/*
	 * Write the IrDA header.
	 */
	memset(irda_hdr, 0, sizeof(irda_hdr));
	phtons(&irda_hdr[IRDA_SLL_PKTTYPE_OFFSET], pseudo_header->irda.pkttype);
	phtons(&irda_hdr[IRDA_SLL_PROTOCOL_OFFSET], 0x0017);
	if (!wtap_dump_file_write(wdh, irda_hdr, sizeof(irda_hdr), err))
		return false;
	return true;
}

/*
 * A header containing additional MTP information.
 */
#define MTP2_SENT_OFFSET		0	/* 1 byte */
#define MTP2_ANNEX_A_USED_OFFSET	1	/* 1 byte */
#define MTP2_LINK_NUMBER_OFFSET		2	/* 2 bytes */
#define MTP2_HDR_LEN			4	/* length of the header */

static int
pcap_read_mtp2_pseudoheader(FILE_T fh, union wtap_pseudo_header *pseudo_header,
    unsigned packet_size, int *err, char **err_info)
{
	uint8_t mtp2_hdr[MTP2_HDR_LEN];

	if (packet_size < MTP2_HDR_LEN) {
		/*
		 * Uh-oh, the packet isn't big enough to even
		 * have a pseudo-header.
		 */
		*err = WTAP_ERR_BAD_FILE;
		*err_info = ws_strdup_printf("pcap/pcapng: MTP2 file has a %u-byte packet, too small to have even an MTP2 pseudo-header",
		    packet_size);
		return -1;
	}
	if (!wtap_read_bytes(fh, mtp2_hdr, MTP2_HDR_LEN, err, err_info))
		return -1;

	pseudo_header->mtp2.sent         = mtp2_hdr[MTP2_SENT_OFFSET];
	pseudo_header->mtp2.annex_a_used = mtp2_hdr[MTP2_ANNEX_A_USED_OFFSET];
	pseudo_header->mtp2.link_number  = pntoh16(&mtp2_hdr[MTP2_LINK_NUMBER_OFFSET]);

	return MTP2_HDR_LEN;
}

static bool
pcap_write_mtp2_pseudoheader(wtap_dumper *wdh,
    const union wtap_pseudo_header *pseudo_header, int *err)
{
	uint8_t mtp2_hdr[MTP2_HDR_LEN];

	/*
	 * Write the MTP2 header.
	 */
	memset(&mtp2_hdr, 0, sizeof(mtp2_hdr));
	mtp2_hdr[MTP2_SENT_OFFSET] = pseudo_header->mtp2.sent;
	mtp2_hdr[MTP2_ANNEX_A_USED_OFFSET] = pseudo_header->mtp2.annex_a_used;
	phtons(&mtp2_hdr[MTP2_LINK_NUMBER_OFFSET],
	    pseudo_header->mtp2.link_number);
	if (!wtap_dump_file_write(wdh, mtp2_hdr, sizeof(mtp2_hdr), err))
		return false;
	return true;
}

/*
 * The fake link-layer header of LAPD packets.
 */
#ifndef ETH_P_LAPD
#define ETH_P_LAPD 0x0030
#endif

#define LAPD_SLL_PKTTYPE_OFFSET		0	/* packet type - 2 bytes */
#define LAPD_SLL_HATYPE_OFFSET		2	/* hardware address type - 2 bytes */
#define LAPD_SLL_HALEN_OFFSET		4	/* hardware address length - 2 bytes */
#define LAPD_SLL_ADDR_OFFSET		6	/* address - 8 bytes */
#define LAPD_SLL_PROTOCOL_OFFSET	14	/* protocol, should be ETH_P_LAPD - 2 bytes */
#define LAPD_SLL_LEN			16	/* length of the header */

static int
pcap_read_lapd_pseudoheader(FILE_T fh, union wtap_pseudo_header *pseudo_header,
    unsigned packet_size, int *err, char **err_info)
{
	uint8_t	lapd_phdr[LAPD_SLL_LEN];

	if (packet_size < LAPD_SLL_LEN) {
		/*
		 * Uh-oh, the packet isn't big enough to even
		 * have a pseudo-header.
		 */
		*err = WTAP_ERR_BAD_FILE;
		*err_info = ws_strdup_printf("pcap/pcapng: LAPD file has a %u-byte packet, too small to have even a LAPD pseudo-header",
		    packet_size);
		return -1;
	}
	if (!wtap_read_bytes(fh, lapd_phdr, LAPD_SLL_LEN, err, err_info))
		return -1;

	if (pntoh16(&lapd_phdr[LAPD_SLL_PROTOCOL_OFFSET]) != ETH_P_LAPD) {
		*err = WTAP_ERR_BAD_FILE;
		if (err_info != NULL)
			*err_info = g_strdup("pcap/pcapng: LAPD capture has a packet with an invalid sll_protocol field");
		return -1;
	}

	pseudo_header->lapd.pkttype = pntoh16(&lapd_phdr[LAPD_SLL_PKTTYPE_OFFSET]);
	pseudo_header->lapd.we_network = !!lapd_phdr[LAPD_SLL_ADDR_OFFSET+0];

	return LAPD_SLL_LEN;
}

static bool
pcap_write_lapd_pseudoheader(wtap_dumper *wdh,
    const union wtap_pseudo_header *pseudo_header, int *err)
{
	uint8_t lapd_hdr[LAPD_SLL_LEN];

	/*
	 * Write the LAPD header.
	 */
	memset(&lapd_hdr, 0, sizeof(lapd_hdr));
	phtons(&lapd_hdr[LAPD_SLL_PKTTYPE_OFFSET], pseudo_header->lapd.pkttype);
	phtons(&lapd_hdr[LAPD_SLL_PROTOCOL_OFFSET], ETH_P_LAPD);
	lapd_hdr[LAPD_SLL_ADDR_OFFSET + 0] =
	    pseudo_header->lapd.we_network?0x01:0x00;
	if (!wtap_dump_file_write(wdh, lapd_hdr, sizeof(lapd_hdr), err))
		return false;
	return true;
}

/*
 * A header containing additional SITA WAN information.
 */
#define SITA_FLAGS_OFFSET		0	/* 1 byte */
#define SITA_SIGNALS_OFFSET		1	/* 1 byte */
#define SITA_ERRORS1_OFFSET		2	/* 1 byte */
#define SITA_ERRORS2_OFFSET		3	/* 1 byte */
#define SITA_PROTO_OFFSET		4	/* 1 byte */
#define SITA_HDR_LEN			5	/* length of the header */

static int
pcap_read_sita_pseudoheader(FILE_T fh, union wtap_pseudo_header *pseudo_header,
    unsigned packet_size, int *err, char **err_info)
{
	uint8_t	sita_phdr[SITA_HDR_LEN];

	if (packet_size < SITA_HDR_LEN) {
		/*
		 * Uh-oh, the packet isn't big enough to even
		 * have a pseudo-header.
		 */
		*err = WTAP_ERR_BAD_FILE;
		*err_info = ws_strdup_printf("pcap/pcapng: SITA file has a %u-byte packet, too small to have even a SITA pseudo-header",
		    packet_size);
		return -1;
	}
	if (!wtap_read_bytes(fh, sita_phdr, SITA_HDR_LEN, err, err_info))
		return -1;

	pseudo_header->sita.sita_flags   = sita_phdr[SITA_FLAGS_OFFSET];
	pseudo_header->sita.sita_signals = sita_phdr[SITA_SIGNALS_OFFSET];
	pseudo_header->sita.sita_errors1 = sita_phdr[SITA_ERRORS1_OFFSET];
	pseudo_header->sita.sita_errors2 = sita_phdr[SITA_ERRORS2_OFFSET];
	pseudo_header->sita.sita_proto   = sita_phdr[SITA_PROTO_OFFSET];

	return SITA_HDR_LEN;
}

static bool
pcap_write_sita_pseudoheader(wtap_dumper *wdh,
    const union wtap_pseudo_header *pseudo_header, int *err)
{
	uint8_t sita_hdr[SITA_HDR_LEN];

	/*
	 * Write the SITA header.
	 */
	memset(&sita_hdr, 0, sizeof(sita_hdr));
	sita_hdr[SITA_FLAGS_OFFSET]   = pseudo_header->sita.sita_flags;
	sita_hdr[SITA_SIGNALS_OFFSET] = pseudo_header->sita.sita_signals;
	sita_hdr[SITA_ERRORS1_OFFSET] = pseudo_header->sita.sita_errors1;
	sita_hdr[SITA_ERRORS2_OFFSET] = pseudo_header->sita.sita_errors2;
	sita_hdr[SITA_PROTO_OFFSET]   = pseudo_header->sita.sita_proto;
	if (!wtap_dump_file_write(wdh, sita_hdr, sizeof(sita_hdr), err))
		return false;
	return true;
}

/*
 * Pseudo-header at the beginning of DLT_BLUETOOTH_HCI_H4_WITH_PHDR frames.
 * Values in network byte order.
 */
struct pcap_bt_phdr {
	uint32_t direction;     /* Bit 0 hold the frame direction. */
};

#define LIBPCAP_BT_PHDR_SENT    0
#define LIBPCAP_BT_PHDR_RECV    1

static int
pcap_read_bt_pseudoheader(FILE_T fh, union wtap_pseudo_header *pseudo_header,
    unsigned packet_size, int *err, char **err_info)
{
	struct pcap_bt_phdr phdr;

	if (packet_size < sizeof (struct pcap_bt_phdr)) {
		/*
		 * Uh-oh, the packet isn't big enough to even
		 * have a pseudo-header.
		 */
		*err = WTAP_ERR_BAD_FILE;
		*err_info = ws_strdup_printf("pcap/pcapng: Bluetooth file has a %u-byte packet, too small to have even a pseudo-header",
		    packet_size);
		return -1;
	}
	if (!wtap_read_bytes(fh, &phdr, sizeof (struct pcap_bt_phdr),
	    err, err_info))
		return -1;
	pseudo_header->p2p.sent = ((g_ntohl(phdr.direction) & LIBPCAP_BT_PHDR_RECV) == 0)? true: false;
	return (int)sizeof (struct pcap_bt_phdr);
}

static bool
pcap_write_bt_pseudoheader(wtap_dumper *wdh,
    const union wtap_pseudo_header *pseudo_header, int *err)
{
	uint32_t direction;
	struct pcap_bt_phdr bt_hdr;

	direction = pseudo_header->p2p.sent ? LIBPCAP_BT_PHDR_SENT : LIBPCAP_BT_PHDR_RECV;
	bt_hdr.direction = GUINT32_TO_BE(direction);
	if (!wtap_dump_file_write(wdh, &bt_hdr, sizeof bt_hdr, err))
		return false;
	return true;
}

/*
 * Pseudo-header at the beginning of DLT_BLUETOOTH_LINUX_MONITOR frames.
 * Values in network byte order.
 */
struct pcap_bt_monitor_phdr {
	uint16_t adapter_id;
	uint16_t opcode;
};

static int
pcap_read_bt_monitor_pseudoheader(FILE_T fh,
    union wtap_pseudo_header *pseudo_header, unsigned packet_size,
    int *err, char **err_info)
{
	struct pcap_bt_monitor_phdr phdr;

	if (packet_size < sizeof (struct pcap_bt_monitor_phdr)) {
		/*
		 * Uh-oh, the packet isn't big enough to even
		 * have a pseudo-header.
		 */
		*err = WTAP_ERR_BAD_FILE;
		*err_info = ws_strdup_printf("pcap/pcapng: Bluetooth monitor file has a %u-byte packet, too small to have even a pseudo-header",
		    packet_size);
		return -1;
	}
	if (!wtap_read_bytes(fh, &phdr, sizeof (struct pcap_bt_monitor_phdr),
	    err, err_info))
		return -1;

	pseudo_header->btmon.adapter_id = g_ntohs(phdr.adapter_id);
	pseudo_header->btmon.opcode = g_ntohs(phdr.opcode);
	return (int)sizeof (struct pcap_bt_monitor_phdr);
}

static bool
pcap_write_bt_monitor_pseudoheader(wtap_dumper *wdh,
    const union wtap_pseudo_header *pseudo_header, int *err)
{
	struct pcap_bt_monitor_phdr bt_monitor_hdr;

	bt_monitor_hdr.adapter_id = GUINT16_TO_BE(pseudo_header->btmon.adapter_id);
	bt_monitor_hdr.opcode = GUINT16_TO_BE(pseudo_header->btmon.opcode);

	if (!wtap_dump_file_write(wdh, &bt_monitor_hdr, sizeof bt_monitor_hdr, err))
		return false;
	return true;
}

/*
 * The NFC LLCP per-packet header.
 */
#define LLCP_ADAPTER_OFFSET		0
#define LLCP_FLAGS_OFFSET		1
#define LLCP_HEADER_LEN			2

static int
pcap_read_llcp_pseudoheader(FILE_T fh,
    union wtap_pseudo_header *pseudo_header, unsigned packet_size,
    int *err, char **err_info)
{
	uint8_t phdr[LLCP_HEADER_LEN];

	if (packet_size < LLCP_HEADER_LEN) {
		*err = WTAP_ERR_BAD_FILE;
		*err_info = ws_strdup_printf("pcap/pcapng: NFC LLCP file has a %u-byte packet, too small to have even a pseudo-header",
		    packet_size);
		return -1;
	}
	if (!wtap_read_bytes(fh, phdr, LLCP_HEADER_LEN, err, err_info))
		return -1;
	pseudo_header->llcp.adapter = phdr[LLCP_ADAPTER_OFFSET];
	pseudo_header->llcp.flags = phdr[LLCP_FLAGS_OFFSET];
	return LLCP_HEADER_LEN;
}

static bool
pcap_write_llcp_pseudoheader(wtap_dumper *wdh,
    const union wtap_pseudo_header *pseudo_header, int *err)
{
	uint8_t phdr[LLCP_HEADER_LEN];

	phdr[LLCP_ADAPTER_OFFSET] = pseudo_header->llcp.adapter;
	phdr[LLCP_FLAGS_OFFSET] = pseudo_header->llcp.flags;
	if (!wtap_dump_file_write(wdh, &phdr, sizeof phdr, err))
		return false;
	return true;
}

/*
 * Pseudo-header at the beginning of DLT_PPP_WITH_DIR frames.
 */
struct pcap_ppp_phdr {
	uint8_t direction;
};

/*
 * Pseudo-header at the beginning of DLT_PPP_WITH_DIR frames.
 */
static int
pcap_read_ppp_pseudoheader(FILE_T fh, union wtap_pseudo_header *pseudo_header,
    unsigned packet_size, int *err, char **err_info)
{
	struct pcap_ppp_phdr phdr;

	if (packet_size < sizeof (struct pcap_ppp_phdr)) {
		/*
		 * Uh-oh, the packet isn't big enough to even
		 * have a pseudo-header.
		 */
		*err = WTAP_ERR_BAD_FILE;
		*err_info = ws_strdup_printf("pcap/pcapng: PPP file has a %u-byte packet, too small to have even a pseudo-header",
		    packet_size);
		return -1;
	}
	if (!wtap_read_bytes(fh, &phdr, sizeof (struct pcap_ppp_phdr),
	    err, err_info))
		return -1;
	/* Any non-zero value means "sent" */
	pseudo_header->p2p.sent = (phdr.direction != 0) ? true: false;
	return (int)sizeof (struct pcap_ppp_phdr);
}

static bool
pcap_write_ppp_pseudoheader(wtap_dumper *wdh,
    const union wtap_pseudo_header *pseudo_header, int *err)
{
	struct pcap_ppp_phdr ppp_hdr;

	/* Any non-zero value means "sent" */
	ppp_hdr.direction = (pseudo_header->p2p.sent ? 1 : 0);
	if (!wtap_dump_file_write(wdh, &ppp_hdr, sizeof ppp_hdr, err))
		return false;
	return true;
}

static int
pcap_read_erf_pseudoheader(FILE_T fh, wtap_rec *rec,
    union wtap_pseudo_header *pseudo_header, unsigned packet_size,
    int *err, char **err_info)
{
	uint8_t erf_hdr[sizeof(struct erf_phdr)];
	uint8_t erf_subhdr[sizeof(union erf_subhdr)];
	int phdr_len;

	if (packet_size < sizeof(struct erf_phdr)) {
		/*
		 * Uh-oh, the packet isn't big enough to even
		 * have a pseudo-header.
		 */
		*err = WTAP_ERR_BAD_FILE;
		*err_info = ws_strdup_printf("pcap/pcapng: ERF file has a %u-byte packet, too small to have even an ERF pseudo-header",
		    packet_size);
		return -1;
	}
	if (!wtap_read_bytes(fh, erf_hdr, sizeof(struct erf_phdr), err, err_info))
		return -1;
	phdr_len = (int)sizeof(struct erf_phdr);
	pseudo_header->erf.phdr.ts = pletoh64(&erf_hdr[0]); /* timestamp */
	pseudo_header->erf.phdr.type = erf_hdr[8];
	pseudo_header->erf.phdr.flags = erf_hdr[9];
	pseudo_header->erf.phdr.rlen = pntoh16(&erf_hdr[10]);
	pseudo_header->erf.phdr.lctr = pntoh16(&erf_hdr[12]);
	pseudo_header->erf.phdr.wlen = pntoh16(&erf_hdr[14]);

	/* The high 32 bits of the timestamp contain the integer number of seconds
	 * while the lower 32 bits contain the binary fraction of the second.
	 * This allows an ultimate resolution of 1/(2^32) seconds, or approximately 233 picoseconds */
	if (rec) {
		uint64_t ts = pseudo_header->erf.phdr.ts;
		rec->ts.secs = (time_t) (ts >> 32);
		ts = ((ts & 0xffffffff) * 1000 * 1000 * 1000);
		ts += (ts & 0x80000000) << 1; /* rounding */
		rec->ts.nsecs = ((uint32_t) (ts >> 32));
		if (rec->ts.nsecs >= 1000000000) {
			rec->ts.nsecs -= 1000000000;
			rec->ts.secs += 1;
		}

		/*
		 * This time stamp came from the ERF header, not from the
		 * pcap packet header or pcapng block header, so its
		 * precision is that of ERF time stamps, not the pcap
		 * file's time stamp or the pcapng interface's time
		 * stamp.
		 */
		rec->tsprec = WTAP_TSPREC_NSEC;
	}

	/*
	 * If the type of record given in the pseudo header indicates
	 * the presence of an extension header, then read all the
	 * extension headers.
	 */
	if (pseudo_header->erf.phdr.type & 0x80) {
		int i = 0, max = array_length(pseudo_header->erf.ehdr_list);
		uint8_t erf_exhdr[8];
		uint8_t type;

		do {
			if (phdr_len > INT_MAX - 8) {
				*err = WTAP_ERR_BAD_FILE;
				*err_info = ws_strdup_printf("pcap/pcapng: ERF file has a packet larger than %d bytes",
				    INT_MAX);
				return -1;
			}
			if (packet_size < (unsigned)phdr_len + 8) {
				*err = WTAP_ERR_BAD_FILE;
				*err_info = ws_strdup_printf("pcap/pcapng: ERF file has a %u-byte packet, too small to include the extension headers",
				    packet_size);
				return -1;
			}
			if (!wtap_read_bytes(fh, erf_exhdr, 8, err, err_info))
				return -1;
			type = erf_exhdr[0];
			if (i < max) {
				uint64_t erf_exhdr_sw;

				erf_exhdr_sw = pntoh64(erf_exhdr);
				memcpy(&pseudo_header->erf.ehdr_list[i].ehdr, &erf_exhdr_sw, sizeof(erf_exhdr_sw));
			}
			phdr_len += 8;
			i++;
		} while (type & 0x80);
	}

	/* check the optional subheader */
	switch (pseudo_header->erf.phdr.type & 0x7F) {
	case ERF_TYPE_MC_HDLC:
	case ERF_TYPE_MC_RAW:
	case ERF_TYPE_MC_ATM:
	case ERF_TYPE_MC_RAW_CHANNEL:
	case ERF_TYPE_MC_AAL5:
	case ERF_TYPE_MC_AAL2:
	case ERF_TYPE_COLOR_MC_HDLC_POS:
		/* Extract the Multi Channel header to include it in the pseudo header part */
		if (phdr_len > INT_MAX - (int)sizeof(erf_mc_header_t)) {
			*err = WTAP_ERR_BAD_FILE;
			*err_info = ws_strdup_printf("pcap/pcapng: ERF file has a packet larger than %d bytes",
			    INT_MAX);
			return -1;
		}
		if (packet_size < (unsigned)(phdr_len + (int)sizeof(erf_mc_header_t))) {
			*err = WTAP_ERR_BAD_FILE;
			*err_info = ws_strdup_printf("pcap/pcapng: ERF file has a %u-byte packet, too small to include the Multi Channel header",
			    packet_size);
			return -1;
		}
		if (!wtap_read_bytes(fh, erf_subhdr, sizeof(erf_mc_header_t), err, err_info))
			return -1;
		pseudo_header->erf.subhdr.mc_hdr = pntoh32(&erf_subhdr[0]);
		phdr_len += sizeof(erf_mc_header_t);
		break;
	case ERF_TYPE_AAL2:
		/* Extract the AAL2 header to include it in the pseudo header part */
		if (phdr_len > INT_MAX - (int)sizeof(erf_aal2_header_t)) {
			*err = WTAP_ERR_BAD_FILE;
			*err_info = ws_strdup_printf("pcap/pcapng: ERF file has a packet larger than %d bytes",
			    INT_MAX);
			return -1;
		}
		if (packet_size < (unsigned)(phdr_len + (int)sizeof(erf_aal2_header_t))) {
			*err = WTAP_ERR_BAD_FILE;
			*err_info = ws_strdup_printf("pcap/pcapng: ERF file has a %u-byte packet, too small to include the AAL2 header",
			    packet_size);
			return -1;
		}
		if (!wtap_read_bytes(fh, erf_subhdr, sizeof(erf_aal2_header_t), err, err_info))
			return -1;
		pseudo_header->erf.subhdr.aal2_hdr = pntoh32(&erf_subhdr[0]);
		phdr_len += sizeof(erf_aal2_header_t);
		break;
	case ERF_TYPE_ETH:
	case ERF_TYPE_COLOR_ETH:
	case ERF_TYPE_DSM_COLOR_ETH:
	case ERF_TYPE_COLOR_HASH_ETH:
		/* Extract the Ethernet additional header to include it in the pseudo header part */
		if (phdr_len > INT_MAX - (int)sizeof(erf_eth_header_t)) {
			*err = WTAP_ERR_BAD_FILE;
			*err_info = ws_strdup_printf("pcap/pcapng: ERF file has a packet larger than %d bytes",
			    INT_MAX);
			return -1;
		}
		if (packet_size < (unsigned)(phdr_len + (int)sizeof(erf_eth_header_t))) {
			*err = WTAP_ERR_BAD_FILE;
			*err_info = ws_strdup_printf("pcap/pcapng: ERF file has a %u-byte packet, too small to include the Ethernet additional header",
			    packet_size);
			return -1;
		}
		if (!wtap_read_bytes(fh, erf_subhdr, sizeof(erf_eth_header_t), err, err_info))
			return -1;
		memcpy(&pseudo_header->erf.subhdr.eth_hdr, erf_subhdr, sizeof pseudo_header->erf.subhdr.eth_hdr);
		phdr_len += sizeof(erf_eth_header_t);
		break;
	default:
		/* No optional pseudo header for this ERF type */
		break;
	}
	return phdr_len;
}

static bool
pcap_write_erf_pseudoheader(wtap_dumper *wdh,
    const union wtap_pseudo_header *pseudo_header, int *err)
{
	uint8_t erf_hdr[sizeof(struct erf_phdr)];
	uint8_t erf_subhdr[sizeof(union erf_subhdr)];

	/*
	 * Write the ERF header.
	 */
	memset(&erf_hdr, 0, sizeof(erf_hdr));
	phtolell(&erf_hdr[0], pseudo_header->erf.phdr.ts);
	erf_hdr[8] = pseudo_header->erf.phdr.type;
	erf_hdr[9] = pseudo_header->erf.phdr.flags;

	/*
	 * Recalculate rlen as padding (and maybe extension headers)
	 * have been stripped from caplen.
	 *
	 * XXX: Since we don't have rec->rec_header.packet_header.caplen
	 * here, assume caplen was calculated correctly and
	 * recalculate from wlen.
	 */
	phtons(&erf_hdr[10],
	    MIN(pseudo_header->erf.phdr.rlen, pseudo_header->erf.phdr.wlen + pcap_get_phdr_size(WTAP_ENCAP_ERF, pseudo_header)));

	phtons(&erf_hdr[12], pseudo_header->erf.phdr.lctr);
	phtons(&erf_hdr[14], pseudo_header->erf.phdr.wlen);
	if (!wtap_dump_file_write(wdh, erf_hdr,  sizeof(struct erf_phdr), err))
		return false;

	/*
	 * Now write out the extension headers.
	 */
	if (pseudo_header->erf.phdr.type & 0x80) {
		int i = 0, max = array_length(pseudo_header->erf.ehdr_list);
		uint8_t erf_exhdr[8];
		uint8_t type;

		do {
			phtonll(erf_exhdr, pseudo_header->erf.ehdr_list[i].ehdr);
			type = erf_exhdr[0];
			/* Clear more extension headers bit if > 8 */
			if(i == max-1)
				erf_exhdr[0] = erf_exhdr[0] & 0x7F;
			if (!wtap_dump_file_write(wdh, erf_exhdr, 8, err))
				return false;
			i++;
		} while (type & 0x80 && i < max);
	}

	/*
	 * Now write out the subheader, if any
	 */
	switch (pseudo_header->erf.phdr.type & 0x7F) {
	case ERF_TYPE_MC_HDLC:
	case ERF_TYPE_MC_RAW:
	case ERF_TYPE_MC_ATM:
	case ERF_TYPE_MC_RAW_CHANNEL:
	case ERF_TYPE_MC_AAL5:
	case ERF_TYPE_MC_AAL2:
	case ERF_TYPE_COLOR_MC_HDLC_POS:
		phtonl(&erf_subhdr[0], pseudo_header->erf.subhdr.mc_hdr);
		if (!wtap_dump_file_write(wdh, erf_subhdr,
		    sizeof(struct erf_mc_hdr), err))
			return false;
		break;
	case ERF_TYPE_AAL2:
		phtonl(&erf_subhdr[0], pseudo_header->erf.subhdr.aal2_hdr);
		if (!wtap_dump_file_write(wdh, erf_subhdr,
		    sizeof(struct erf_aal2_hdr), err))
			return false;
		break;
	case ERF_TYPE_ETH:
	case ERF_TYPE_COLOR_ETH:
	case ERF_TYPE_DSM_COLOR_ETH:
	case ERF_TYPE_COLOR_HASH_ETH:
		memcpy(&erf_subhdr[0], &pseudo_header->erf.subhdr.eth_hdr, sizeof pseudo_header->erf.subhdr.eth_hdr);
		if (!wtap_dump_file_write(wdh, erf_subhdr,
		    sizeof(struct erf_eth_hdr), err))
			return false;
		break;
	default:
		break;
	}
	return true;
}

/*
 * I2C-with=Linux-pseudoheader link-layer on-disk format, as defined by
 * Pigeon Point Systems.
 */
struct i2c_linux_file_hdr {
	uint8_t bus;
	uint8_t flags[4];
};

static int
pcap_read_i2c_linux_pseudoheader(FILE_T fh, union wtap_pseudo_header *pseudo_header,
    unsigned packet_size, int *err, char **err_info)
{
	struct i2c_linux_file_hdr i2c_linux_hdr;

	if (packet_size < sizeof (struct i2c_linux_file_hdr)) {
		/*
		 * Uh-oh, the packet isn't big enough to even
		 * have a pseudo-header.
		 */
		*err = WTAP_ERR_BAD_FILE;
		*err_info = ws_strdup_printf("pcap/pcapng: I2C file has a %u-byte packet, too small to have even a I2C pseudo-header",
		    packet_size);
		return -1;
	}
	if (!wtap_read_bytes(fh, &i2c_linux_hdr, sizeof (i2c_linux_hdr), err, err_info))
		return -1;

	pseudo_header->i2c.is_event = i2c_linux_hdr.bus & 0x80 ? 1 : 0;
	pseudo_header->i2c.bus = i2c_linux_hdr.bus & 0x7f;
	pseudo_header->i2c.flags = pntoh32(&i2c_linux_hdr.flags);

	return (int)sizeof (struct i2c_linux_file_hdr);
}

static bool
pcap_write_i2c_linux_pseudoheader(wtap_dumper *wdh,
    const union wtap_pseudo_header *pseudo_header, int *err)
{
	struct i2c_linux_file_hdr i2c_linux_hdr;

	/*
	 * Write the I2C Linux-specific pseudo-header.
	 */
	memset(&i2c_linux_hdr, 0, sizeof(i2c_linux_hdr));
	i2c_linux_hdr.bus = pseudo_header->i2c.bus |
			(pseudo_header->i2c.is_event ? 0x80 : 0x00);
	phtonl((uint8_t *)&i2c_linux_hdr.flags, pseudo_header->i2c.flags);
	if (!wtap_dump_file_write(wdh, &i2c_linux_hdr, sizeof(i2c_linux_hdr), err))
		return false;
	return true;
}

/*
 * The link-layer header on Nokia IPSO packets.
 */
#define NOKIA_LEN	4	/* length of the header */

static bool
pcap_read_nokia_pseudoheader(FILE_T fh,
    union wtap_pseudo_header *pseudo_header, int *err, char **err_info)
{
	uint8_t	phdr[NOKIA_LEN];


	/* backtrack to read the 4 mysterious bytes that aren't considered
	* part of the packet size
	*/
	if (file_seek(fh, -NOKIA_LEN, SEEK_CUR, err) == -1)
	{
		*err = file_error(fh, err_info);
		if (*err == 0)
			*err = WTAP_ERR_SHORT_READ;
		return false;
	}

	if (!wtap_read_bytes(fh, phdr, NOKIA_LEN, err, err_info))
		return false;

	memcpy(pseudo_header->nokia.stuff, phdr, NOKIA_LEN);

	return true;
}

/*
 * When not using the memory-mapped interface to capture USB events,
 * code that reads those events can use the MON_IOCX_GET ioctl to
 * read a 48-byte header consisting of a "struct linux_usb_phdr", as
 * defined below, followed immediately by one of:
 *
 *	8 bytes of a "struct usb_device_setup_hdr", if "setup_flag"
 *	in the preceding "struct linux_usb_phdr" is 0;
 *
 *	in Linux 2.6.30 or later, 8 bytes of a "struct iso_rec", if
 *	this is an isochronous transfer;
 *
 *	8 bytes of junk, otherwise.
 *
 * In Linux 2.6.31 and later, it can also use the MON_IOCX_GETX ioctl
 * to read a 64-byte header; that header consists of the 48 bytes
 * above, followed immediately by 16 bytes of a "struct linux_usb_phdr_ext",
 * as defined below.
 *
 * In Linux 2.6.21 and later, there's a memory-mapped interface to
 * capture USB events.  In that interface, the events in the memory-mapped
 * buffer have a 64-byte header, followed immediately by the data.
 * In Linux 2.6.21 through 2.6.30.x, the 64-byte header is the 48-byte
 * header described above, followed by 16 bytes of zeroes; in Linux
 * 2.6.31 and later, the 64-byte header is the 64-byte header described
 * above.
 *
 * See linux/Documentation/usb/usbmon.txt and libpcap/pcap/usb.h for details.
 *
 * With WTAP_ENCAP_USB_LINUX, packets have the 48-byte header; with
 * WTAP_ENCAP_USB_LINUX_MMAPPED, they have the 64-byte header.  There
 * is no indication of whether the header has the "struct iso_rec", or
 * whether the last 16 bytes of a 64-byte header are all zeros or are
 * a "struct linux_usb_phdr_ext".
 */

/*
 * URB transfer_type values
 */
#define URB_ISOCHRONOUS   0x0
#define URB_INTERRUPT     0x1
#define URB_CONTROL       0x2
#define URB_BULK          0x3

/*
 * Information from the URB for Isochronous transfers.
 *
 * This structure is 8 bytes long.
 */
struct iso_rec {
	int32_t error_count;
	int32_t numdesc;
};

/*
 * Header prepended by Linux kernel to each USB event.
 *
 * (Setup flag is '-', 'D', 'Z', or 0.  Data flag is '<', '>', 'Z', or 0.)
 *
 * The values are in *host* byte order.
 */
struct linux_usb_phdr {
	uint64_t id;             /* urb id, to link submission and completion events */
	uint8_t event_type;      /* Submit ('S'), Completed ('C'), Error ('E') */
	uint8_t transfer_type;   /* ISO (0), Intr, Control, Bulk (3) */
	uint8_t endpoint_number; /* Endpoint number (0-15) and transfer direction */
	uint8_t device_address;  /* 0-127 */
	uint16_t bus_id;
	int8_t setup_flag;       /* 0, if the urb setup header is meaningful */
	int8_t data_flag;        /* 0, if urb data is present */
	int64_t ts_sec;
	int32_t ts_usec;
	int32_t status;
	uint32_t urb_len;        /* whole len of urb this event refers to */
	uint32_t data_len;       /* amount of urb data really present in this event */

	/*
	 * Packet-type-dependent data.
	 * USB setup information of setup_flag is true.
	 * Otherwise, some isochronous transfer information.
	 */
	union {
		uint8_t data[8];
		struct iso_rec iso;
	} s;

	/*
	 * This data is provided by Linux 2.6.31 and later kernels.
	 *
	 * For WTAP_ENCAP_USB_LINUX, it's not in the pseudo-header, so
	 * the pseudo-header is always 48 bytes long, including the
	 * packet-type-dependent data.
	 *
	 * For WTAP_ENCAP_USB_LINUX_MMAPPED, the pseudo-header is always
	 * 64 bytes long, with the packet-type-dependent data preceding
	 * these last 16 bytes.  In pre-2.6.31 kernels, it's zero padding;
	 * in 2.6.31 and later, it's the following data.
	 */
	int32_t interval;    /* only for Interrupt and Isochronous events */
	int32_t start_frame; /* for Isochronous */
	uint32_t xfer_flags; /* copy of URB's transfer_flags */
	uint32_t ndesc;      /* actual number of isochronous descriptors */
};

/*
 * event_type values
 */
#define URB_SUBMIT        'S'
#define URB_COMPLETE      'C'
#define URB_ERROR         'E'

/*
 * URB transfer_type values
 */
#define URB_ISOCHRONOUS   0x0
#define URB_INTERRUPT     0x1
#define URB_CONTROL       0x2
#define URB_BULK          0x3
#define URB_UNKNOWN       0xFF

#define URB_TRANSFER_IN   0x80		/* to host */

struct linux_usb_isodesc {
	int32_t iso_status;
	uint32_t iso_off;
	uint32_t iso_len;
	uint32_t _pad;
};

/*
 * USB setup header as defined in USB specification
 * See usb_20.pdf, Chapter 9.3 'USB Device Requests' for details.
 * https://www.usb.org/document-library/usb-20-specification
 *
 * This structure is 8 bytes long.
 */
struct usb_device_setup_hdr {
	int8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
};

/*
 * Offset of the *end* of a field within a particular structure.
 */
#define END_OFFSETOF(basep, fieldp) \
	(((char *)(void *)(fieldp)) - ((char *)(void *)(basep)) + \
	    sizeof(*fieldp))

/*
 * Is that offset within the bounds of the packet?
 */
#define WITHIN_PACKET(basep, fieldp) \
	(packet_size >= END_OFFSETOF((basep), (fieldp)))

#define CHECK_AND_SWAP16(fieldp) \
	{ \
		if (!WITHIN_PACKET(usb_phdr, fieldp)) \
			return; \
		PBSWAP16((uint8_t *)fieldp); \
	}

#define CHECK_AND_SWAP32(fieldp) \
	{ \
		if (!WITHIN_PACKET(usb_phdr, fieldp)) \
			return; \
		PBSWAP32((uint8_t *)fieldp); \
	}

#define CHECK_AND_SWAP64(fieldp) \
	{ \
		if (!WITHIN_PACKET(usb_phdr, fieldp)) \
			return; \
		PBSWAP64((uint8_t *)fieldp); \
	}

/*
 * Offset and length of the CAN ID field in the CAN classic/CAN FD
 * SocketCAN header.
 */
#define CAN_CANFD_CAN_ID_OFFSET		0
#define CAN_CANFD_CAN_ID_LEN		4

/*
 * Offsets and lengths of fields in the CAN XL SocketCAN header.
 */
#define CANXL_PRIORITY_VCID_OFFSET	0
#define CANXL_PRIORITY_VCID_LEN		4
#define CANXL_FLAGS_OFFSET		(CANXL_PRIORITY_VCID_OFFSET + CANXL_PRIORITY_VCID_LEN)
#define CANXL_FLAGS_LEN			1
#define CANXL_SDU_TYPE_OFFSET		(CANXL_FLAGS_OFFSET + CANXL_FLAGS_LEN)
#define CANXL_SDU_TYPE_LEN		1
#define CANXL_PAYLOAD_LENGTH_OFFSET	(CANXL_SDU_TYPE_OFFSET + CANXL_SDU_TYPE_LEN)
#define CANXL_PAYLOAD_LENGTH_LEN	2
#define CANXL_ACCEPTANCE_FIELD_OFFSET	(CANXL_PAYLOAD_LENGTH_OFFSET + CANXL_PAYLOAD_LENGTH_LEN)
#define CANXL_ACCEPTANCE_FIELD_LEN	4

/*
 * CAN fake link-layer headers in Linux cooked packets.
 */
#define LINUX_SLL_PROTOCOL_OFFSET	14	/* protocol */
#define LINUX_SLL_LEN			16	/* length of the header */

#define LINUX_SLL2_PROTOCOL_OFFSET	0	/* protocol */
#define LINUX_SLL2_LEN			20	/* length of the header */

/*
 * The protocols we have to check for.
 */
#define LINUX_SLL_P_CAN			0x000C	/* Controller Area Network classic */
#define LINUX_SLL_P_CANFD		0x000D	/* Controller Area Network flexible data rate */
#define LINUX_SLL_P_CANXL		0x000E	/* Controller Area Network extended length */

static void
pcap_byteswap_can_socketcan_pseudoheader(unsigned packet_size, uint16_t protocol,
    uint8_t *pd)
{
	switch (protocol) {

	case LINUX_SLL_P_CAN:
	case LINUX_SLL_P_CANFD:
		/*
		 * CAN classic or CAN FD; byte-swap the ID/flags field
		 * into our host byte order.
		 *
		 * Make sure we have the entire field.
		 */
		if (packet_size < (CAN_CANFD_CAN_ID_OFFSET + CAN_CANFD_CAN_ID_LEN)) {
			/* Not enough data to have the full CAN ID */
			return;
		}
		PBSWAP32(&pd[CAN_CANFD_CAN_ID_OFFSET]);
		break;

	case LINUX_SLL_P_CANXL:
		/*
		 * CAN classic or CAN FD; byte-swap the priority-and-VCID
		 * field, the payload length, ad the acceptance field
		 * into our host byte order.
		 */
		if (packet_size < (CANXL_PRIORITY_VCID_OFFSET + CANXL_PRIORITY_VCID_LEN)) {
			/* Not enough data to have the full priority/VCID field */
			return;
		}
		PBSWAP32(&pd[CANXL_PRIORITY_VCID_OFFSET]);
		if (packet_size < (CANXL_PAYLOAD_LENGTH_OFFSET + CANXL_PAYLOAD_LENGTH_LEN)) {
			/* Not enough data to have the full payload length field */
			return;
		}
		PBSWAP16(&pd[CANXL_PAYLOAD_LENGTH_OFFSET]);
		if (packet_size < (CANXL_ACCEPTANCE_FIELD_OFFSET + CANXL_ACCEPTANCE_FIELD_LEN)) {
			/* Not enough data to have the full payload length field */
			return;
		}
		PBSWAP32(&pd[CANXL_ACCEPTANCE_FIELD_OFFSET]);
		break;

	default:
		/* Not a CAN packet; nothing to fix */
		return;
	}
}

static void
pcap_byteswap_linux_sll_pseudoheader(wtap_rec *rec)
{
	uint8_t *pd;
	unsigned packet_size;
	uint16_t protocol;

	pd = ws_buffer_start_ptr(&rec->data);

	/*
	 * Minimum of captured and actual length (just in case the
	 * actual length < the captured length, which Should Never
	 * Happen).
	 */
	packet_size = rec->rec_header.packet_header.caplen;
	if (packet_size > rec->rec_header.packet_header.len)
		packet_size = rec->rec_header.packet_header.len;

	if (packet_size < LINUX_SLL_LEN) {
		/* Not enough data to have the protocol */
		return;
	}

	/*
	 * Byte-swap the SocketCAN pseudoheader, if we have one.
	 */
	protocol = pntoh16(&pd[LINUX_SLL_PROTOCOL_OFFSET]);
	pcap_byteswap_can_socketcan_pseudoheader(packet_size - LINUX_SLL_LEN,
	    protocol, pd + LINUX_SLL_LEN);
}

static void
pcap_byteswap_linux_sll2_pseudoheader(wtap_rec *rec)
{
	uint8_t *pd;
	unsigned packet_size;
	uint16_t protocol;

	pd = ws_buffer_start_ptr(&rec->data);

	/*
	 * Minimum of captured and actual length (just in case the
	 * actual length < the captured length, which Should Never
	 * Happen).
	 */
	packet_size = rec->rec_header.packet_header.caplen;
	if (packet_size > rec->rec_header.packet_header.len)
		packet_size = rec->rec_header.packet_header.len;

	if (packet_size < LINUX_SLL2_LEN) {
		/* Not enough data to have the protocol */
		return;
	}

	/*
	 * Byte-swap the SocketCAN pseudoheader, if we have one.
	 */
	protocol = pntoh16(&pd[LINUX_SLL2_PROTOCOL_OFFSET]);
	pcap_byteswap_can_socketcan_pseudoheader(packet_size - LINUX_SLL2_LEN,
	    protocol, pd + LINUX_SLL2_LEN);
}

static void
pcap_byteswap_linux_usb_pseudoheader(wtap_rec *rec, bool header_len_64_bytes)
{
	uint8_t *pd;
	unsigned packet_size;
	struct linux_usb_phdr *usb_phdr;
	struct linux_usb_isodesc *pisodesc;
	int32_t iso_numdesc, i;

	pd = ws_buffer_start_ptr(&rec->data);

	/*
	 * Minimum of captured and actual length (just in case the
	 * actual length < the captured length, which Should Never
	 * Happen).
	 */
	packet_size = rec->rec_header.packet_header.caplen;
	if (packet_size > rec->rec_header.packet_header.len)
		packet_size = rec->rec_header.packet_header.len;

	/*
	 * Greasy hack, but we never directly dereference any of
	 * the fields in *usb_phdr, we just get offsets of and
	 * addresses of its members and byte-swap it with a
	 * byte-at-a-time macro, so it's alignment-safe.
	 */
	usb_phdr = (struct linux_usb_phdr *)(void *)pd;

	CHECK_AND_SWAP64(&usb_phdr->id);
	CHECK_AND_SWAP16(&usb_phdr->bus_id);
	CHECK_AND_SWAP64(&usb_phdr->ts_sec);
	CHECK_AND_SWAP32(&usb_phdr->ts_usec);
	CHECK_AND_SWAP32(&usb_phdr->status);
	CHECK_AND_SWAP32(&usb_phdr->urb_len);
	CHECK_AND_SWAP32(&usb_phdr->data_len);

	if (usb_phdr->transfer_type == URB_ISOCHRONOUS) {
		CHECK_AND_SWAP32(&usb_phdr->s.iso.error_count);
		CHECK_AND_SWAP32(&usb_phdr->s.iso.numdesc);
	}

	if (header_len_64_bytes) {
		/*
		 * This is either the "version 1" header, with
		 * 16 bytes of additional fields at the end, or
		 * a "version 0" header from a memory-mapped
		 * capture, with 16 bytes of zeroed-out padding
		 * at the end.  Byte swap them as if this were
		 * a "version 1" header.
		 *
		 * Yes, the first argument to END_OFFSETOF() should
		 * be usb_phdr, not usb_phdr_ext; we want the offset of
		 * the additional fields from the beginning of
		 * the packet.
		 */
		CHECK_AND_SWAP32(&usb_phdr->interval);
		CHECK_AND_SWAP32(&usb_phdr->start_frame);
		CHECK_AND_SWAP32(&usb_phdr->xfer_flags);
		CHECK_AND_SWAP32(&usb_phdr->ndesc);
	}

	if (usb_phdr->transfer_type == URB_ISOCHRONOUS) {
		/* swap the values in struct linux_usb_isodesc */

		/*
		 * See previous "Greasy hack" comment.
		 */
		if (header_len_64_bytes) {
			pisodesc = (struct linux_usb_isodesc*)(void *)(pd + 64);
		} else {
			pisodesc = (struct linux_usb_isodesc*)(void *)(pd + 48);
		}
		iso_numdesc = usb_phdr->s.iso.numdesc;
		for (i = 0; i < iso_numdesc; i++) {
			CHECK_AND_SWAP32(&pisodesc->iso_status);
			CHECK_AND_SWAP32(&pisodesc->iso_off);
			CHECK_AND_SWAP32(&pisodesc->iso_len);
			CHECK_AND_SWAP32(&pisodesc->_pad);

			pisodesc++;
		}
	}
}

struct nflog_hdr {
	uint8_t nflog_family;		/* address family */
	uint8_t nflog_version;		/* version */
	uint16_t nflog_rid;		/* resource ID */
};

struct nflog_tlv {
	uint16_t tlv_length;		/* tlv length */
	uint16_t tlv_type;		/* tlv type */
	/* value follows this */
};

static void
pcap_byteswap_nflog_pseudoheader(wtap_rec *rec)
{
	uint8_t *pd;
	unsigned packet_size;
	uint8_t *p;
	struct nflog_hdr *nfhdr;
	struct nflog_tlv *tlv;
	unsigned size;

	pd = ws_buffer_start_ptr(&rec->data);

	/*
	 * Minimum of captured and actual length (just in case the
	 * actual length < the captured length, which Should Never
	 * Happen).
	 */
	packet_size = rec->rec_header.packet_header.caplen;
	if (packet_size > rec->rec_header.packet_header.len)
		packet_size = rec->rec_header.packet_header.len;

	if (packet_size < sizeof(struct nflog_hdr)) {
		/* Not enough data to have any TLVs. */
		return;
	}

	p = pd;
	nfhdr = (struct nflog_hdr *)pd;
	if (nfhdr->nflog_version != 0) {
		/* Unknown NFLOG version */
		return;
	}

	packet_size -= (unsigned)sizeof(struct nflog_hdr);
	p += sizeof(struct nflog_hdr);

	while (packet_size >= sizeof(struct nflog_tlv)) {
		tlv = (struct nflog_tlv *) p;

		/* Swap the type and length. */
		PBSWAP16((uint8_t *)&tlv->tlv_type);
		PBSWAP16((uint8_t *)&tlv->tlv_length);

		/* Get the length of the TLV. */
		size = tlv->tlv_length;
		size = WS_ROUNDUP_4(size);

		/* Is the TLV's length less than the minimum? */
		if (size < sizeof(struct nflog_tlv)) {
			/* Yes. Give up now. */
			return;
		}

		/* Do we have enough data for the full TLV? */
		if (packet_size < size) {
			/* No. */
			return;
		}

		/* Skip over the TLV. */
		packet_size -= size;
		p += size;
	}
}

/*
 * pflog headers, at least as they exist now.
 */
#define PFLOG_IFNAMSIZ		16
#define PFLOG_RULESET_NAME_SIZE	16

struct pfloghdr {
	uint8_t		length;
	uint8_t		af;
	uint8_t		action;
	uint8_t		reason;
	char		ifname[PFLOG_IFNAMSIZ];
	char		ruleset[PFLOG_RULESET_NAME_SIZE];
	uint32_t		rulenr;
	uint32_t		subrulenr;
	uint32_t		uid;
	int32_t		pid;
	uint32_t		rule_uid;
	int32_t		rule_pid;
	uint8_t		dir;
	/* More follows, depending on the header length */
};

static void
pcap_byteswap_pflog_pseudoheader(wtap_rec *rec)
{
	uint8_t *pd;
	unsigned packet_size;
	struct pfloghdr *pflhdr;

	pd = ws_buffer_start_ptr(&rec->data);

	/*
	 * Minimum of captured and actual length (just in case the
	 * actual length < the captured length, which Should Never
	 * Happen).
	 */
	packet_size = rec->rec_header.packet_header.caplen;
	if (packet_size > rec->rec_header.packet_header.len)
		packet_size = rec->rec_header.packet_header.len;

	if (packet_size < sizeof(struct pfloghdr)) {
		/* Not enough data to have the UID and PID fields */
		return;
	}

	pflhdr = (struct pfloghdr *)pd;
	if (pflhdr->length < (unsigned) (offsetof(struct pfloghdr, rule_pid) + sizeof pflhdr->rule_pid)) {
		/* Header doesn't include the UID and PID fields */
		return;
	}
	PBSWAP32((uint8_t *)&pflhdr->uid);
	PBSWAP32((uint8_t *)&pflhdr->pid);
	PBSWAP32((uint8_t *)&pflhdr->rule_uid);
	PBSWAP32((uint8_t *)&pflhdr->rule_pid);
}

int
pcap_process_pseudo_header(FILE_T fh, bool is_nokia, int wtap_encap,
    unsigned packet_size, wtap_rec *rec, int *err, char **err_info)
{
	int phdr_len = 0;

	switch (wtap_encap) {

	case WTAP_ENCAP_ATM_PDUS:
		if (is_nokia) {
			/*
			 * Nokia IPSO ATM.
			 */
			phdr_len = pcap_read_nokiaatm_pseudoheader(fh,
			    &rec->rec_header.packet_header.pseudo_header,
			    packet_size, err, err_info);
			if (phdr_len == -1)
				return -1;	/* Read error */
		} else {
			/*
			 * SunATM.
			 */
			phdr_len = pcap_read_sunatm_pseudoheader(fh,
			    &rec->rec_header.packet_header.pseudo_header,
			    packet_size, err, err_info);
			if (phdr_len == -1)
				return -1;	/* Read error */
		}
		break;

	case WTAP_ENCAP_ETHERNET:
		if (is_nokia) {
			/*
			 * Nokia IPSO.  Pseudo header has already been read, but it's not considered
			 * part of the packet size, so reread it to store the data for later (when saving)
			 */
			if (!pcap_read_nokia_pseudoheader(fh, &rec->rec_header.packet_header.pseudo_header, err, err_info))
				return -1;	/* Read error */
		}

		/*
		 * We don't know whether there's an FCS in this frame or not.
		 */
		rec->rec_header.packet_header.pseudo_header.eth.fcs_len = -1;
		break;

	case WTAP_ENCAP_IEEE_802_11:
	case WTAP_ENCAP_IEEE_802_11_PRISM:
	case WTAP_ENCAP_IEEE_802_11_RADIOTAP:
	case WTAP_ENCAP_IEEE_802_11_AVS:
		/*
		 * We don't know whether there's an FCS in this frame or not,
		 * at least in pcap files.  For radiotap, that's indicated in
		 * the radiotap header.
		 *
		 * XXX - in pcapng, there *could* be a packet option
		 * indicating the FCS length.
		 */
		memset(&rec->rec_header.packet_header.pseudo_header.ieee_802_11, 0, sizeof(rec->rec_header.packet_header.pseudo_header.ieee_802_11));
		rec->rec_header.packet_header.pseudo_header.ieee_802_11.fcs_len = -1;
		rec->rec_header.packet_header.pseudo_header.ieee_802_11.decrypted = false;
		rec->rec_header.packet_header.pseudo_header.ieee_802_11.datapad = false;
		break;

	case WTAP_ENCAP_IRDA:
		phdr_len = pcap_read_irda_pseudoheader(fh,
		    &rec->rec_header.packet_header.pseudo_header,
		    packet_size, err, err_info);
		if (phdr_len == -1)
			return -1;	/* Read error */
		break;

	case WTAP_ENCAP_MTP2_WITH_PHDR:
		phdr_len = pcap_read_mtp2_pseudoheader(fh,
		    &rec->rec_header.packet_header.pseudo_header,
		    packet_size, err, err_info);
		if (phdr_len == -1)
			return -1;	/* Read error */
		break;

	case WTAP_ENCAP_LINUX_LAPD:
		phdr_len = pcap_read_lapd_pseudoheader(fh,
		    &rec->rec_header.packet_header.pseudo_header,
		    packet_size, err, err_info);
		if (phdr_len == -1)
			return -1;	/* Read error */
		break;

	case WTAP_ENCAP_SITA:
		phdr_len = pcap_read_sita_pseudoheader(fh,
		    &rec->rec_header.packet_header.pseudo_header,
		    packet_size, err, err_info);
		if (phdr_len == -1)
			return -1;	/* Read error */
		break;

	case WTAP_ENCAP_BLUETOOTH_H4:
		/* We don't have pseudoheader, so just pretend we received everything. */
		rec->rec_header.packet_header.pseudo_header.p2p.sent = false;
		break;

	case WTAP_ENCAP_BLUETOOTH_H4_WITH_PHDR:
		phdr_len = pcap_read_bt_pseudoheader(fh,
		    &rec->rec_header.packet_header.pseudo_header,
		    packet_size, err, err_info);
		if (phdr_len == -1)
			return -1;	/* Read error */
		break;

	case WTAP_ENCAP_BLUETOOTH_LINUX_MONITOR:
		phdr_len = pcap_read_bt_monitor_pseudoheader(fh,
		    &rec->rec_header.packet_header.pseudo_header,
		    packet_size, err, err_info);
		if (phdr_len == -1)
			return -1;	/* Read error */
		break;

	case WTAP_ENCAP_NFC_LLCP:
		phdr_len = pcap_read_llcp_pseudoheader(fh,
		    &rec->rec_header.packet_header.pseudo_header,
		    packet_size, err, err_info);
		if (phdr_len == -1)
			return -1;	/* Read error */
		break;

	case WTAP_ENCAP_PPP_WITH_PHDR:
		phdr_len = pcap_read_ppp_pseudoheader(fh,
		    &rec->rec_header.packet_header.pseudo_header,
		    packet_size, err, err_info);
		if (phdr_len == -1)
			return -1;	/* Read error */
		break;

	case WTAP_ENCAP_ERF:
		phdr_len = pcap_read_erf_pseudoheader(fh, rec,
		    &rec->rec_header.packet_header.pseudo_header,
		    packet_size, err, err_info);
		if (phdr_len == -1)
			return -1;	/* Read error */
		break;

	case WTAP_ENCAP_I2C_LINUX:
		phdr_len = pcap_read_i2c_linux_pseudoheader(fh,
		    &rec->rec_header.packet_header.pseudo_header,
		    packet_size, err, err_info);
		if (phdr_len == -1)
			return -1;	/* Read error */
		break;
	}

	return phdr_len;
}

/*
 * Compute, from the data provided by the Linux USB memory-mapped capture
 * mechanism, the amount of packet data that would have been provided
 * had the capture mechanism not chopped off any data at the end, if, in
 * fact, it did so.
 *
 * Set the "unsliced length" field of the packet header to that value.
 */
static void
fix_linux_usb_mmapped_length(wtap_rec *rec, const u_char *bp)
{
	const struct linux_usb_phdr *hdr;
	u_int bytes_left;

	/*
	 * All callers of this routine must ensure that pkth->caplen is
	 * >= sizeof (struct linux_usb_phdr).
	 */
	bytes_left = rec->rec_header.packet_header.caplen;
	bytes_left -= sizeof (struct linux_usb_phdr);

	hdr = (const struct linux_usb_phdr *) bp;
	if (!hdr->data_flag && hdr->transfer_type == URB_ISOCHRONOUS &&
	    hdr->event_type == URB_COMPLETE &&
	    (hdr->endpoint_number & URB_TRANSFER_IN) &&
	    rec->rec_header.packet_header.len == sizeof(struct linux_usb_phdr) +
	                 (hdr->ndesc * sizeof (struct linux_usb_isodesc)) + hdr->urb_len) {
		struct linux_usb_isodesc *descs;
		u_int pre_truncation_data_len, pre_truncation_len;

		descs = (struct linux_usb_isodesc *) (bp + sizeof(struct linux_usb_phdr));

		/*
		 * We have data (yes, data_flag is 0 if we *do* have data),
		 * and this is a "this is complete" incoming isochronous
		 * transfer event, and the length was calculated based
		 * on the URB length.
		 *
		 * That's not correct, because the data isn't contiguous,
		 * and the isochronous descriptos show how it's scattered.
		 *
		 * Find the end of the last chunk of data in the buffer
		 * referred to by the isochronous descriptors; that indicates
		 * how far into the buffer the data would have gone.
		 *
		 * Make sure we don't run past the end of the captured data
		 * while processing the isochronous descriptors.
		 */
		pre_truncation_data_len = 0;
		for (uint32_t desc = 0;
		    desc < hdr->ndesc && bytes_left >= sizeof (struct linux_usb_isodesc);
		    desc++, bytes_left -= sizeof (struct linux_usb_isodesc)) {
			u_int desc_end;

			if (descs[desc].iso_len != 0) {
				desc_end = descs[desc].iso_off + descs[desc].iso_len;
				if (desc_end > pre_truncation_data_len)
					pre_truncation_data_len = desc_end;
			}
		}

		/*
		 * Now calculate the total length based on that data
		 * length.
		 */
		pre_truncation_len = sizeof(struct linux_usb_phdr) +
		    (hdr->ndesc * sizeof (struct linux_usb_isodesc)) +
		    pre_truncation_data_len;

		/*
		 * If that's greater than or equal to the captured length,
		 * use that as the length.
		 */
		if (pre_truncation_len >= rec->rec_header.packet_header.caplen)
			rec->rec_header.packet_header.len = pre_truncation_len;

		/*
		 * If the captured length is greater than the length,
		 * use the captured length.
		 *
		 * For completion events for incoming isochronous transfers,
		 * it's based on data_len, which is calculated the same way
		 * we calculated pre_truncation_data_len above, except that
		 * it has access to all the isochronous descriptors, not
		 * just the ones that the kernel were able to provide us or,
		 * for a capture file, that weren't sliced off by a snapshot
		 * length.
		 *
		 * However, it might have been reduced by the USB capture
		 * mechanism arbitrarily limiting the amount of data it
		 * provides to userland, or by the libpcap capture code
		 * limiting it to being no more than the snapshot, so
		 * we don't want to just use it all the time; we only
		 * do so to try to get a better estimate of the actual
		 * length - and to make sure the on-the-network length
		 * is always >= the captured length.
		 */
		if (rec->rec_header.packet_header.caplen > rec->rec_header.packet_header.len)
			rec->rec_header.packet_header.len = rec->rec_header.packet_header.caplen;
	}
}

static void
pcap_fixup_len(wtap_rec *rec)
{
	const uint8_t *pd;
	struct linux_usb_phdr *usb_phdr;

	pd = ws_buffer_start_ptr(&rec->data);

	/*
	 * Greasy hack, but we never directly dereference any of
	 * the fields in *usb_phdr, we just get offsets of and
	 * addresses of its members and byte-swap it with a
	 * byte-at-a-time macro, so it's alignment-safe.
	 */
	usb_phdr = (struct linux_usb_phdr *)(void *)pd;

	if (rec->rec_header.packet_header.caplen >=
	    sizeof (struct linux_usb_phdr)) {
		/*
		 * In older versions of libpcap, in memory-mapped captures,
		 * the "on-the-bus length" for completion events for
		 * incoming isochronous transfers was miscalculated; it
		 * needed to be calculated based on the* offsets and lengths
		 * in the descriptors, not on the raw URB length, but it
		 * wasn't.
		 *
		 * If this packet contains transferred data (yes, data_flag
		 * is 0 if we *do* have data), and the total on-the-network
		 * length is equal to the value calculated from the raw URB
		 * length, then it might be one of those transfers.
		 *
		 * We only do this if we have the full USB pseudo-header.
		 */
		if (!usb_phdr->data_flag &&
		    rec->rec_header.packet_header.len == sizeof (struct linux_usb_phdr) +
		      (usb_phdr->ndesc * sizeof (struct linux_usb_isodesc)) + usb_phdr->urb_len) {
			/*
			 * It might need fixing; fix it if it's a completion
			 * event for an incoming isochronous transfer.
			 */
			fix_linux_usb_mmapped_length(rec, pd);
		}
	}
}

void
pcap_read_post_process(bool is_nokia, int wtap_encap,
    wtap_rec *rec, bool bytes_swapped, int fcs_len)
{
	switch (wtap_encap) {

	case WTAP_ENCAP_ATM_PDUS:
		if (is_nokia) {
			/*
			 * Nokia IPSO ATM.
			 *
			 * Guess the traffic type based on the packet
			 * contents.
			 */
			atm_guess_traffic_type(rec);
		} else {
			/*
			 * SunATM.
			 *
			 * If this is ATM LANE traffic, try to guess what
			 * type of LANE traffic it is based on the packet
			 * contents.
			 */
			if (rec->rec_header.packet_header.pseudo_header.atm.type == TRAF_LANE)
				atm_guess_lane_type(rec);
		}
		break;

	case WTAP_ENCAP_ETHERNET:
		/*
		 * The FCS length is supposed to be in bits.
		 * If it's < 8, assume it's in bytes; otherwise,
		 * convert it to bytes.
		 */
		if (fcs_len < 8)
			rec->rec_header.packet_header.pseudo_header.eth.fcs_len = fcs_len;
		else
			rec->rec_header.packet_header.pseudo_header.eth.fcs_len = fcs_len/8;
		break;

	case WTAP_ENCAP_SLL:
		if (bytes_swapped)
			pcap_byteswap_linux_sll_pseudoheader(rec);
		break;

	case WTAP_ENCAP_SLL2:
		if (bytes_swapped)
			pcap_byteswap_linux_sll2_pseudoheader(rec);
		break;

	case WTAP_ENCAP_USB_LINUX:
		if (bytes_swapped)
			pcap_byteswap_linux_usb_pseudoheader(rec, false);
		break;

	case WTAP_ENCAP_USB_LINUX_MMAPPED:
		if (bytes_swapped)
			pcap_byteswap_linux_usb_pseudoheader(rec, true);

		/*
		 * Fix up the on-the-network length if necessary.
		 */
		pcap_fixup_len(rec);
		break;

	case WTAP_ENCAP_NETANALYZER:
		/*
		 * Not strictly necessary, as the netANALYZER
		 * dissector calls the "Ethernet with FCS"
		 * dissector, but we might as well set it.
		 */
		rec->rec_header.packet_header.pseudo_header.eth.fcs_len = 4;
		break;

	case WTAP_ENCAP_NFLOG:
		if (bytes_swapped)
			pcap_byteswap_nflog_pseudoheader(rec);
		break;

	case WTAP_ENCAP_ERF:
		/*
		 * Update packet size to account for ERF padding and snapping.
		 * Captured length is minimum of wlen and previously calculated
		 * caplen (which would have included padding but not phdr).
		 */
		rec->rec_header.packet_header.len = rec->rec_header.packet_header.pseudo_header.erf.phdr.wlen;
		rec->rec_header.packet_header.caplen = MIN(rec->rec_header.packet_header.len, rec->rec_header.packet_header.caplen);
		break;

	case WTAP_ENCAP_PFLOG:
		if (bytes_swapped)
			pcap_byteswap_pflog_pseudoheader(rec);
		break;

	default:
		break;
	}
}

bool
wtap_encap_requires_phdr(int wtap_encap)
{
	switch (wtap_encap) {

	case WTAP_ENCAP_ATM_PDUS:
	case WTAP_ENCAP_IRDA:
	case WTAP_ENCAP_MTP2_WITH_PHDR:
	case WTAP_ENCAP_LINUX_LAPD:
	case WTAP_ENCAP_SITA:
	case WTAP_ENCAP_BLUETOOTH_H4_WITH_PHDR:
	case WTAP_ENCAP_BLUETOOTH_LINUX_MONITOR:
	case WTAP_ENCAP_NFC_LLCP:
	case WTAP_ENCAP_PPP_WITH_PHDR:
	case WTAP_ENCAP_ERF:
	case WTAP_ENCAP_I2C_LINUX:
		return true;
	}
	return false;
}

int
pcap_get_phdr_size(int encap, const union wtap_pseudo_header *pseudo_header)
{
	int hdrsize;

	switch (encap) {

	case WTAP_ENCAP_ATM_PDUS:
		hdrsize = SUNATM_LEN;
		break;

	case WTAP_ENCAP_IRDA:
		hdrsize = IRDA_SLL_LEN;
		break;

	case WTAP_ENCAP_MTP2_WITH_PHDR:
		hdrsize = MTP2_HDR_LEN;
		break;

	case WTAP_ENCAP_LINUX_LAPD:
		hdrsize = LAPD_SLL_LEN;
		break;

	case WTAP_ENCAP_SITA:
		hdrsize = SITA_HDR_LEN;
		break;

	case WTAP_ENCAP_BLUETOOTH_H4_WITH_PHDR:
		hdrsize = (int)sizeof (struct pcap_bt_phdr);
		break;

	case WTAP_ENCAP_BLUETOOTH_LINUX_MONITOR:
		hdrsize = (int)sizeof (struct pcap_bt_monitor_phdr);
		break;

	case WTAP_ENCAP_NFC_LLCP:
		hdrsize = LLCP_HEADER_LEN;
		break;

	case WTAP_ENCAP_PPP_WITH_PHDR:
		hdrsize = (int)sizeof (struct pcap_ppp_phdr);
		break;

	case WTAP_ENCAP_ERF:
		hdrsize = (int)sizeof (struct erf_phdr);

		/*
		 * If the type of record given in the pseudo header
		 * indicates the presence of an extension header, then
		 * add in the lengths of the extension headers.
		 */
		if (pseudo_header->erf.phdr.type & 0x80) {
			int i = 0, max = array_length(pseudo_header->erf.ehdr_list);
			uint8_t erf_exhdr[8];
			uint8_t type;

			do {
				phtonll(erf_exhdr, pseudo_header->erf.ehdr_list[i].ehdr);
				type = erf_exhdr[0];
				hdrsize += 8;
				i++;
			} while (type & 0x80 && i < max);
		}

		/*
		 * Now add in the length of the subheader, if any.
		 */
		switch (pseudo_header->erf.phdr.type & 0x7F) {

		case ERF_TYPE_MC_HDLC:
		case ERF_TYPE_MC_RAW:
		case ERF_TYPE_MC_ATM:
		case ERF_TYPE_MC_RAW_CHANNEL:
		case ERF_TYPE_MC_AAL5:
		case ERF_TYPE_MC_AAL2:
		case ERF_TYPE_COLOR_MC_HDLC_POS:
			hdrsize += (int)sizeof(struct erf_mc_hdr);
			break;
		case ERF_TYPE_AAL2:
			hdrsize += (int)sizeof(struct erf_aal2_hdr);
			break;

		case ERF_TYPE_ETH:
		case ERF_TYPE_COLOR_ETH:
		case ERF_TYPE_DSM_COLOR_ETH:
		case ERF_TYPE_COLOR_HASH_ETH:
			hdrsize += (int)sizeof(struct erf_eth_hdr);
			break;

		default:
			break;
		}
		break;

	case WTAP_ENCAP_I2C_LINUX:
		hdrsize = (int)sizeof (struct i2c_linux_file_hdr);
		break;

	default:
		hdrsize = 0;
		break;
	}

	return hdrsize;
}

bool
pcap_write_phdr(wtap_dumper *wdh, int encap, const union wtap_pseudo_header *pseudo_header,
    int *err)
{
	switch (encap) {

	case WTAP_ENCAP_ATM_PDUS:
		if (!pcap_write_sunatm_pseudoheader(wdh, pseudo_header, err))
			return false;
		break;

	case WTAP_ENCAP_IRDA:
		if (!pcap_write_irda_pseudoheader(wdh, pseudo_header, err))
			return false;
		break;

	case WTAP_ENCAP_MTP2_WITH_PHDR:
		if (!pcap_write_mtp2_pseudoheader(wdh, pseudo_header, err))
			return false;
		break;

	case WTAP_ENCAP_LINUX_LAPD:
		if (!pcap_write_lapd_pseudoheader(wdh, pseudo_header, err))
			return false;
		break;

	case WTAP_ENCAP_SITA:
		if (!pcap_write_sita_pseudoheader(wdh, pseudo_header, err))
			return false;
		break;

	case WTAP_ENCAP_BLUETOOTH_H4_WITH_PHDR:
		if (!pcap_write_bt_pseudoheader(wdh, pseudo_header, err))
			return false;
		break;

	case WTAP_ENCAP_BLUETOOTH_LINUX_MONITOR:
		if (!pcap_write_bt_monitor_pseudoheader(wdh, pseudo_header, err))
			return false;
		break;

	case WTAP_ENCAP_NFC_LLCP:
		if (!pcap_write_llcp_pseudoheader(wdh, pseudo_header, err))
			return false;
		break;

	case WTAP_ENCAP_PPP_WITH_PHDR:
		if (!pcap_write_ppp_pseudoheader(wdh, pseudo_header, err))
			return false;
		break;

	case WTAP_ENCAP_ERF:
		if (!pcap_write_erf_pseudoheader(wdh, pseudo_header, err))
			return false;
		break;

	case WTAP_ENCAP_I2C_LINUX:
		if (!pcap_write_i2c_linux_pseudoheader(wdh, pseudo_header, err))
			return false;
		break;
	}
	return true;
}

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: t
 * End:
 *
 * vi: set shiftwidth=8 tabstop=8 noexpandtab:
 * :indentSize=8:tabSize=8:noTabs=false:
 */

/* packet-gadu-gadu.c
 * Routines for Gadu-Gadu dissection
 * Copyright 2011,2012, Jakub Zawadzki <darkjames-ws@darkjames.pl>
 *
 * Protocol documentation available at http://toxygen.net/libgadu/protocol/
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "config.h"

#include <epan/packet.h>
#include <epan/prefs.h>
#include "packet-tcp.h"

void proto_register_gadu_gadu(void);
void proto_reg_handoff_gadu_gadu(void);

#define TCP_PORT_GADU_GADU 8074	/* assigned by IANA */

/* desegmentation of Gadu-Gadu over TCP */
static bool gadu_gadu_desegment = true;

static dissector_handle_t gadu_gadu_handle;

static int proto_gadu_gadu;

static int hf_dcc_filename;
static int hf_dcc_id;
static int hf_dcc_type;
static int hf_dcc_uin_from;
static int hf_dcc_uin_to;
static int hf_gadu_gadu_contact_type;
static int hf_gadu_gadu_contact_uin;
static int hf_gadu_gadu_contact_uin_str;
static int hf_gadu_gadu_data;
static int hf_gadu_gadu_header_length;
static int hf_gadu_gadu_header_type_recv;
static int hf_gadu_gadu_header_type_send;
static int hf_gadu_gadu_login80_lang;
static int hf_gadu_gadu_login_hash;
static int hf_gadu_gadu_login_hash_type;
static int hf_gadu_gadu_login_local_ip;
static int hf_gadu_gadu_login_local_port;
static int hf_gadu_gadu_login_protocol;
static int hf_gadu_gadu_login_status;
static int hf_gadu_gadu_login_uin;
static int hf_gadu_gadu_login_version;
static int hf_gadu_gadu_msg80_offset_attr;
static int hf_gadu_gadu_msg80_offset_plain;
static int hf_gadu_gadu_msg_ack_recipient;
static int hf_gadu_gadu_msg_ack_seq;
static int hf_gadu_gadu_msg_ack_status;
static int hf_gadu_gadu_msg_class;
static int hf_gadu_gadu_msg_recipient;
static int hf_gadu_gadu_msg_sender;
static int hf_gadu_gadu_msg_seq;
static int hf_gadu_gadu_msg_text;
static int hf_gadu_gadu_msg_time;
static int hf_gadu_gadu_msg_uin;
static int hf_gadu_gadu_new_status_desc;
static int hf_gadu_gadu_new_status_status;
static int hf_gadu_gadu_pubdir_reply_seq;
static int hf_gadu_gadu_pubdir_reply_str;
static int hf_gadu_gadu_pubdir_reply_type;
static int hf_gadu_gadu_pubdir_request_seq;
static int hf_gadu_gadu_pubdir_request_str;
static int hf_gadu_gadu_pubdir_request_type;
static int hf_gadu_gadu_status_descr;
static int hf_gadu_gadu_status_img_size;
static int hf_gadu_gadu_status_ip;
static int hf_gadu_gadu_status_port;
static int hf_gadu_gadu_status_status;
static int hf_gadu_gadu_status_uin;
static int hf_gadu_gadu_status_version;
static int hf_gadu_gadu_typing_notify_type;
static int hf_gadu_gadu_typing_notify_uin;
static int hf_gadu_gadu_userdata_attr_name;
static int hf_gadu_gadu_userdata_attr_type;
static int hf_gadu_gadu_userdata_attr_value;
static int hf_gadu_gadu_userdata_uin;
static int hf_gadu_gadu_userlist;
static int hf_gadu_gadu_userlist_format;
static int hf_gadu_gadu_userlist_reply_type;
static int hf_gadu_gadu_userlist_request_type;
static int hf_gadu_gadu_userlist_version;
static int hf_gadu_gadu_welcome_seed;

static int ett_gadu_gadu;
static int ett_gadu_gadu_contact;

#define GG_ERA_OMNIX_MASK 0x04000000
#define GG_HAS_AUDIO_MASK 0x40000000

#define GG_WELCOME                  0x01
#define GG_STATUS                   0x02
#define GG_LOGIN_OK                 0x03
#define GG_SEND_MSG_ACK             0x05
#define GG_PONG                     0x07
#define GG_PING                     0x08
#define GG_LOGIN_FAILED             0x09
#define GG_RECV_MSG                 0x0a
#define GG_DISCONNECTING            0x0b
#define GG_NOTIFY_REPLY             0x0c
#define GG_DISCONNECT_ACK           0x0d
#define GG_PUBDIR50_REPLY           0x0e
#define GG_STATUS60                 0x0f
#define GG_USERLIST_REPLY           0x10
#define GG_NOTIFY_REPLY60           0x11
#define GG_NEED_EMAIL               0x14
#define GG_LOGIN_HASH_TYPE_INVALID  0x16
#define GG_STATUS77                 0x17
#define GG_NOTIFY_REPLY77           0x18
#define GG_DCC7_INFO                0x1f
#define GG_DCC7_NEW                 0x20
#define GG_DCC7_ACCEPT              0x21
#define GG_DCC7_REJECT              0x22
#define GG_DCC7_ID_REPLY            0x23
#define GG_DCC7_ID_ABORTED          0x25
#define GG_XML_EVENT                0x27
#define GG_STATUS80BETA             0x2a
#define GG_NOTIFY_REPLY80BETA       0x2b
#define GG_XML_ACTION               0x2c
#define GG_RECV_MSG80               0x2e
#define GG_USERLIST_REPLY80         0x30
#define GG_LOGIN_OK80               0x35
#define GG_STATUS80                 0x36
#define GG_NOTIFY_REPLY80           0x37
#define GG_USERLIST100_REPLY        0x41
#define GG_LOGIN80_FAILED           0x43
#define GG_USER_DATA                0x44
#define GG_TYPING_NOTIFY            0x59
#define GG_OWN_MESSAGE              0x5A
#define GG_OWN_RESOURCE_INFO        0x5B
#define GG_USERLIST100_VERSION      0x5C


#define GG_TYPE_VS(x) { x, #x }

/* original (GG_*) names likes in documentation (http://toxygen.net/libgadu/protocol/#ch1.16) */
static const value_string gadu_gadu_packets_type_recv[] = {
	GG_TYPE_VS(GG_WELCOME),
	GG_TYPE_VS(GG_STATUS),
	GG_TYPE_VS(GG_LOGIN_OK),
	GG_TYPE_VS(GG_SEND_MSG_ACK),
	GG_TYPE_VS(GG_PONG),
	GG_TYPE_VS(GG_PING),
	GG_TYPE_VS(GG_LOGIN_FAILED),
	GG_TYPE_VS(GG_RECV_MSG),
	GG_TYPE_VS(GG_DISCONNECTING),
	GG_TYPE_VS(GG_NOTIFY_REPLY),
	GG_TYPE_VS(GG_DISCONNECT_ACK),
	GG_TYPE_VS(GG_PUBDIR50_REPLY),
	GG_TYPE_VS(GG_STATUS60),
	GG_TYPE_VS(GG_USERLIST_REPLY),
	GG_TYPE_VS(GG_NOTIFY_REPLY60),
	GG_TYPE_VS(GG_NEED_EMAIL),
	GG_TYPE_VS(GG_LOGIN_HASH_TYPE_INVALID),
	GG_TYPE_VS(GG_STATUS77),
	GG_TYPE_VS(GG_NOTIFY_REPLY77),
	GG_TYPE_VS(GG_DCC7_INFO),
	GG_TYPE_VS(GG_DCC7_NEW),
	GG_TYPE_VS(GG_DCC7_ACCEPT),
	GG_TYPE_VS(GG_DCC7_REJECT),
	GG_TYPE_VS(GG_DCC7_ID_REPLY),
	GG_TYPE_VS(GG_DCC7_ID_ABORTED),
	GG_TYPE_VS(GG_XML_EVENT),
	GG_TYPE_VS(GG_STATUS80BETA),
	GG_TYPE_VS(GG_NOTIFY_REPLY80BETA),
	GG_TYPE_VS(GG_XML_ACTION),
	GG_TYPE_VS(GG_RECV_MSG80),
	GG_TYPE_VS(GG_USERLIST_REPLY80),
	GG_TYPE_VS(GG_LOGIN_OK80),
	GG_TYPE_VS(GG_STATUS80),
	GG_TYPE_VS(GG_NOTIFY_REPLY80),
	GG_TYPE_VS(GG_USERLIST100_REPLY),
	GG_TYPE_VS(GG_LOGIN80_FAILED),
	GG_TYPE_VS(GG_USER_DATA),
	GG_TYPE_VS(GG_TYPING_NOTIFY),
	GG_TYPE_VS(GG_OWN_MESSAGE),
	GG_TYPE_VS(GG_OWN_RESOURCE_INFO),
	GG_TYPE_VS(GG_USERLIST100_VERSION),
	{ 0, NULL }
};

#define GG_NEW_STATUS           0x02
#define GG_PONG                 0x07
#define GG_PING                 0x08
#define GG_SEND_MSG             0x0b
#define GG_LOGIN                0x0c
#define GG_ADD_NOTIFY           0x0d
#define GG_REMOVE_NOTIFY        0x0e
#define GG_NOTIFY_FIRST         0x0f
#define GG_NOTIFY_LAST          0x10
#define GG_LIST_EMPTY           0x12
#define GG_LOGIN_EXT            0x13
#define GG_PUBDIR50_REQUEST     0x14
#define GG_LOGIN60              0x15
#define GG_USERLIST_REQUEST     0x16
#define GG_LOGIN70              0x19
#define GG_DCC7_INFO            0x1f
#define GG_DCC7_NEW             0x20
#define GG_DCC7_ACCEPT          0x21
#define GG_DCC7_REJECT          0x22
#define GG_DCC7_ID_REQUEST      0x23
#define GG_DCC7_ID_DUNNO1       0x24
#define GG_DCC7_ID_ABORT        0x25
#define GG_NEW_STATUS80BETA     0x28
#define GG_LOGIN80BETA          0x29
#define GG_SEND_MSG80           0x2d
#define GG_USERLIST_REQUEST80   0x2f
#define GG_LOGIN80              0x31
#define GG_NEW_STATUS80         0x38
#define GG_USERLIST100_REQUEST  0x40
#define GG_RECV_MSG_ACK         0x46
#define GG_TYPING_NOTIFY        0x59
#define GG_OWN_DISCONNECT       0x62
#define GG_NEW_STATUS105        0x63
#define GG_NOTIFY105            0x78
#define GG_ADD_NOTIFY105        0x7b
#define GG_REMOVE_NOTIFY105     0x7c
#define GG_LOGIN105             0x83

static const value_string gadu_gadu_packets_type_send[] = {
	GG_TYPE_VS(GG_NEW_STATUS),
	GG_TYPE_VS(GG_PONG),
	GG_TYPE_VS(GG_PING),
	GG_TYPE_VS(GG_SEND_MSG),
	GG_TYPE_VS(GG_LOGIN),
	GG_TYPE_VS(GG_ADD_NOTIFY),
	GG_TYPE_VS(GG_REMOVE_NOTIFY),
	GG_TYPE_VS(GG_NOTIFY_FIRST),
	GG_TYPE_VS(GG_NOTIFY_LAST),
	GG_TYPE_VS(GG_LIST_EMPTY),
	GG_TYPE_VS(GG_LOGIN_EXT),
	GG_TYPE_VS(GG_PUBDIR50_REQUEST),
	GG_TYPE_VS(GG_LOGIN60),
	GG_TYPE_VS(GG_USERLIST_REQUEST),
	GG_TYPE_VS(GG_LOGIN70),
	GG_TYPE_VS(GG_DCC7_INFO),
	GG_TYPE_VS(GG_DCC7_NEW),
	GG_TYPE_VS(GG_DCC7_ACCEPT),
	GG_TYPE_VS(GG_DCC7_REJECT),
	GG_TYPE_VS(GG_DCC7_ID_REQUEST),
	GG_TYPE_VS(GG_DCC7_ID_DUNNO1),
	GG_TYPE_VS(GG_DCC7_ID_ABORT),
	GG_TYPE_VS(GG_NEW_STATUS80BETA),
	GG_TYPE_VS(GG_LOGIN80BETA),
	GG_TYPE_VS(GG_SEND_MSG80),
	GG_TYPE_VS(GG_USERLIST_REQUEST80),
	GG_TYPE_VS(GG_LOGIN80),
	GG_TYPE_VS(GG_NEW_STATUS80),
	GG_TYPE_VS(GG_USERLIST100_REQUEST),
	GG_TYPE_VS(GG_RECV_MSG_ACK),
	GG_TYPE_VS(GG_TYPING_NOTIFY),
	GG_TYPE_VS(GG_OWN_DISCONNECT),
	GG_TYPE_VS(GG_NEW_STATUS105),
	GG_TYPE_VS(GG_NOTIFY105),
	GG_TYPE_VS(GG_ADD_NOTIFY105),
	GG_TYPE_VS(GG_REMOVE_NOTIFY105),
	GG_TYPE_VS(GG_LOGIN105),
	{ 0, NULL }
};

static const value_string gadu_gadu_msg_ack_status_vals[] = {
	{ 0x01, "Message blocked" },
	{ 0x02, "Message delivered" },
	{ 0x03, "Message queued" },
	{ 0x04, "Message not delivered (queue full)" },
	{ 0x06, "CTCP Message not delivered" },
	{ 0, NULL }
};

#define GG_STATUS_NOT_AVAIL         0x01
#define GG_STATUS_NOT_AVAIL_DESCR   0x15
#define GG_STATUS_FFC               0x17
#define GG_STATUS_FFC_DESCR         0x18
#define GG_STATUS_AVAIL             0x02
#define GG_STATUS_AVAIL_DESCR       0x04
#define GG_STATUS_BUSY              0x03
#define GG_STATUS_BUSY_DESCR        0x05
#define GG_STATUS_DND               0x21
#define GG_STATUS_DND_DESCR         0x22
#define GG_STATUS_INVISIBLE         0x14
#define GG_STATUS_INVISIBLE_DESCR   0x16
#define GG_STATUS_BLOCKED           0x06

#define GG_LOGIN_HASH_GG32 0x01
#define GG_LOGIN_HASH_SHA1 0x02

static const value_string gadu_gadu_hash_type_vals[] = {
	{ GG_LOGIN_HASH_GG32, "GG32 hash" },
	{ GG_LOGIN_HASH_SHA1, "SHA1 hash" },
	{ 0, NULL }
};

#define GG_USERLIST_PUT 0x00
#define GG_USERLIST_PUT_MORE 0x01
#define GG_USERLIST_GET 0x02

static const value_string gadu_gadu_userlist_request_type_vals[] = {
	{ GG_USERLIST_PUT, "Userlist put" },
	{ GG_USERLIST_PUT_MORE, "Userlist put (more)" },
	{ GG_USERLIST_GET, "Userlist get" },
	{ 0, NULL }
};

#define GG_USERLIST_PUT_REPLY 0x00
#define GG_USERLIST_PUT_MORE_REPLY 0x02
#define GG_USERLIST_GET_REPLY 0x06
#define GG_USERLIST_GET_MORE_REPLY 0x04

static const value_string gadu_gadu_userlist_reply_type_vals[] = {
	{ GG_USERLIST_PUT_REPLY, "Userlist put" },
	{ GG_USERLIST_PUT_MORE_REPLY, "Userlist put (more)" },
	{ GG_USERLIST_GET_REPLY, "Userlist get" },
	{ GG_USERLIST_GET_MORE_REPLY, "Userlist get (more)" },
	{ 0, NULL }
};

#define GG_USERLIST100_FORMAT_TYPE_NONE 0x00
#define GG_USERLIST100_FORMAT_TYPE_GG70 0x01
#define GG_USERLIST100_FORMAT_TYPE_GG100 0x02

static const value_string gadu_gadu_userlist_request_format_vals[] = {
	{ GG_USERLIST100_FORMAT_TYPE_NONE, "None" },
	{ GG_USERLIST100_FORMAT_TYPE_GG70, "Classic (7.0)" },
	{ GG_USERLIST100_FORMAT_TYPE_GG100, "XML (10.0)" },
	{ 0, NULL }
};

/* XXX, add compatible libgadu versions? */
static const value_string gadu_gadu_version_vals[] = {
	{ 0x2e, "Gadu-Gadu 8.0 (build 8283)" },
	{ 0x2d, "Gadu-Gadu 8.0 (build 4881)" },
	{ 0x2a,	"Gadu-Gadu 7.7 (build 3315)" },
	{ 0x29,	"Gadu-Gadu 7.6 (build 1688)" },
	{ 0x28,	"Gadu-Gadu 7.5.0 (build 2201)" },
	{ 0x27,	"Gadu-Gadu 7.0 (build 22)" },
	{ 0x26,	"Gadu-Gadu 7.0 (build 20)" },
	{ 0x25,	"Gadu-Gadu 7.0 (build 1)" },
	{ 0x24,	"Gadu-Gadu 6.1 (build 155) or 7.6 (build 1359)" },
	{ 0x22,	"Gadu-Gadu 6.0 (build 140)" },
	{ 0x21,	"Gadu-Gadu 6.0 (build 133)" },
	{ 0x20,	"Gadu-Gadu 6.0" },
	{ 0x1e,	"Gadu-Gadu 5.7 beta (build 121)" },
	{ 0x1c,	"Gadu_Gadu 5.7 beta" },
	{ 0x1b,	"Gadu-Gadu 5.0.5" },
	{ 0x19,	"Gadu-Gadu 5.0.3" },
	{ 0x18,	"Gadu-Gadu 5.0.1, 5.0.0, 4.9.3" },
	{ 0x17,	"Gadu-Gadu 4.9.2" },
	{ 0x16,	"Gadu-Gadu 4.9.1" },
	{ 0x15,	"Gadu-Gadu 4.8.9" },
	{ 0x14,	"Gadu-Gadu 4.8.3, 4.8.1" },
	{ 0x11,	"Gadu-Gadu 4.6.10, 4.6.1" },
	{ 0x10,	"Gadu-Gadu 4.5.22, 4.5.21, 4.5.19, 4.5.17, 4.5.15" },
	{ 0x0f,	"Gadu-Gadu 4.5.12" },
	{ 0x0b,	"Gadu-Gadu 4.0.30, 4.0.29, 4.0.28, 4.0.25" },
	{ 0, NULL }
};

static const value_string gadu_gadu_dcc_type_vals[] = {
	{ 1, "Voice transmission" },
	{ 4, "File transmission" },
	{ 0, NULL }
};

static const value_string gadu_gadu_typing_notify_type_vals[] = {
	{ 1, "Typing started" },
	{ 0, "Typing stopped" },
	{ 0, NULL }
};

static const value_string gadu_gadu_pubdir_type_vals[] = {
	{ 1, "Public directory write" },
	{ 2, "Public directory read" },
	{ 3, "Public directory search" },
	{ 0, NULL }
};

static dissector_handle_t xml_handle;

struct gadu_gadu_conv_data {
	uint32_t uin;	/* uin from login packet */
};

static struct gadu_gadu_conv_data *
gadu_gadu_create_conversation(packet_info *pinfo, uint32_t uin)
{
	conversation_t *conv;
	struct gadu_gadu_conv_data *gg_conv;

	conv = find_or_create_conversation(pinfo);
	gg_conv = (struct gadu_gadu_conv_data *)conversation_get_proto_data(conv, proto_gadu_gadu);
	if (!gg_conv) {
		gg_conv = wmem_new(wmem_file_scope(), struct gadu_gadu_conv_data);
		gg_conv->uin = uin;

		conversation_add_proto_data(conv, proto_gadu_gadu, gg_conv);
	}
	/* assert(gg_conv->uin == uin); */
	return gg_conv;
}

static struct gadu_gadu_conv_data *
gadu_gadu_get_conversation_data(packet_info *pinfo)
{
	conversation_t *conv;

	conv = find_conversation_pinfo(pinfo, 0);
	if (conv)
		return (struct gadu_gadu_conv_data *)conversation_get_proto_data(conv, proto_gadu_gadu);
	return NULL;
}

static bool
gadu_gadu_status_has_descr(int status)
{
	return
		(status == GG_STATUS_NOT_AVAIL_DESCR) ||
		(status == GG_STATUS_FFC_DESCR) ||
		(status == GG_STATUS_AVAIL_DESCR) ||
		(status == GG_STATUS_BUSY_DESCR) ||
		(status == GG_STATUS_DND_DESCR) ||
		(status == GG_STATUS_INVISIBLE_DESCR);
}

/* like tvb_strsize() but return maximum length instead of throwing exception */
static int
gadu_gadu_strsize(tvbuff_t *tvb, const int abs_offset)
{
	int nul_offset;

	nul_offset = tvb_find_uint8(tvb, abs_offset, -1, 0);
	if (nul_offset == -1)
		nul_offset = tvb_captured_length(tvb) - 1;

	return (nul_offset - abs_offset) + 1;
}

static int
dissect_gadu_gadu_stringz_cp1250(tvbuff_t *tvb, int hfindex, proto_tree *tree, const int offset)
{
	int len = gadu_gadu_strsize(tvb, offset);

	proto_tree_add_item(tree, hfindex, tvb, offset, len, ENC_WINDOWS_1250 | ENC_NA);

	return offset + len;
}

static int
dissect_gadu_gadu_uint32_string_utf8(tvbuff_t *tvb, int hfindex, proto_tree *tree, int offset)
{
	const int org_offset = offset;

	uint32_t len;

	len = tvb_get_letohl(tvb, offset);
	offset += 4;

	offset += len;

	proto_tree_add_item(tree, hfindex, tvb, org_offset, offset - org_offset, ENC_UTF_8 | ENC_NA);

	return offset;
}


static int
dissect_gadu_gadu_disconnecting(tvbuff_t *tvb _U_, packet_info *pinfo, proto_tree *tree _U_, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Disconnecting");

	/* empty packet */

	return offset;
}


static int
dissect_gadu_gadu_disconnect_ack(tvbuff_t *tvb _U_, packet_info *pinfo, proto_tree *tree _U_, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Disconnect acknowledge (< 10.0)");

	/* empty packet */

	return offset;
}

static void *
_tvb_memcpy_reverse(tvbuff_t *tvb, void *target, int offset, size_t length)
{
	uint8_t *t = (uint8_t *) target;

	while (length > 0) {
		length--;
		t[length] = tvb_get_uint8(tvb, offset);
		offset++;
	}
	return target;
}

static int
dissect_gadu_gadu_login_protocol(tvbuff_t *tvb, proto_tree *tree, int offset)
{
	proto_item *ti;

	uint32_t protocol;

	protocol = tvb_get_letohl(tvb, offset) & 0xff;
	proto_tree_add_item(tree, hf_gadu_gadu_login_protocol, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	ti = proto_tree_add_string(tree, hf_gadu_gadu_login_version, tvb, offset, 4, val_to_str(protocol, gadu_gadu_version_vals, "Unknown (0x%x)"));
	proto_item_set_generated(ti);
	offset += 4;

	return offset;
}

static int
dissect_gadu_gadu_login(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	proto_item *ti;

	uint32_t uin;
	uint8_t hash[4];

	col_set_str(pinfo->cinfo, COL_INFO, "Login request (< 6.0)");

	uin = tvb_get_letohl(tvb, offset);
	gadu_gadu_create_conversation(pinfo, uin);

	proto_tree_add_uint(tree, hf_gadu_gadu_login_uin, tvb, offset, 4, uin);
	offset += 4;

	ti = proto_tree_add_uint(tree, hf_gadu_gadu_login_hash_type, tvb, 0, 0, GG_LOGIN_HASH_GG32);
	proto_item_set_generated(ti);

	/* hash is 32-bit number written in LE */
	_tvb_memcpy_reverse(tvb, hash, offset, 4);
	proto_tree_add_bytes_format_value(tree, hf_gadu_gadu_login_hash, tvb, offset, 4, hash, "0x%.8x", tvb_get_letohl(tvb, offset));
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_login_status, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	offset = dissect_gadu_gadu_login_protocol(tvb, tree, offset);

	proto_tree_add_item(tree, hf_gadu_gadu_login_local_ip, tvb, offset, 4, ENC_BIG_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_login_local_port, tvb, offset, 2, ENC_LITTLE_ENDIAN);
	offset += 2;

	return offset;
}

static int
dissect_gadu_gadu_login_hash(tvbuff_t *tvb, proto_tree *tree, int offset)
{
	uint8_t hash_type;

	uint8_t hash[4];
	int i;

	hash_type = tvb_get_uint8(tvb, offset);
	proto_tree_add_item(tree, hf_gadu_gadu_login_hash_type, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	switch (hash_type) {
		case GG_LOGIN_HASH_GG32:
			/* hash is 32-bit number written in LE */
			_tvb_memcpy_reverse(tvb, hash, offset, 4);
			proto_tree_add_bytes_format_value(tree, hf_gadu_gadu_login_hash, tvb, offset, 4, hash, "0x%.8x", tvb_get_letohl(tvb, offset));
			for (i = 4; i < 64; i++) {
				if (tvb_get_uint8(tvb, offset+i)) {
					proto_tree_add_item(tree, hf_gadu_gadu_data, tvb, offset + 4, 64-4, ENC_NA);
					break;
				}
			}
			break;

		case GG_LOGIN_HASH_SHA1:
			proto_tree_add_item(tree, hf_gadu_gadu_login_hash, tvb, offset, 20, ENC_NA);
			for (i = 20; i < 64; i++) {
				if (tvb_get_uint8(tvb, offset+i)) {
					proto_tree_add_item(tree, hf_gadu_gadu_data, tvb, offset + 20, 64-20, ENC_NA);
					break;
				}
			}
			break;

		default:
			proto_tree_add_item(tree, hf_gadu_gadu_data, tvb, offset, 64, ENC_NA);
			break;
	}
	offset += 64;

	return offset;
}

static int
dissect_gadu_gadu_login70(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	uint32_t uin;

	col_set_str(pinfo->cinfo, COL_INFO, "Login request (7.0)");

	uin = tvb_get_letohl(tvb, offset) & ~(GG_ERA_OMNIX_MASK | GG_HAS_AUDIO_MASK);
	gadu_gadu_create_conversation(pinfo, uin);

	proto_tree_add_uint(tree, hf_gadu_gadu_login_uin, tvb, offset, 4, uin);
	offset += 4;

	offset = dissect_gadu_gadu_login_hash(tvb, tree, offset);

	proto_tree_add_item(tree, hf_gadu_gadu_login_status, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	offset = dissect_gadu_gadu_login_protocol(tvb, tree, offset);

	proto_tree_add_item(tree, hf_gadu_gadu_data, tvb, offset, 1, ENC_NA);	/* 00 */
	offset += 1;

	proto_tree_add_item(tree, hf_gadu_gadu_login_local_ip, tvb, offset, 4, ENC_BIG_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_login_local_port, tvb, offset, 2, ENC_LITTLE_ENDIAN);
	offset += 2;

	/* XXX packet not fully dissected */

	return offset;
}

static int
dissect_gadu_gadu_login80(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	uint32_t uin;

	col_set_str(pinfo->cinfo, COL_INFO, "Login request (8.0)");

	uin = tvb_get_letohl(tvb, offset);
	gadu_gadu_create_conversation(pinfo, uin);

	proto_tree_add_item(tree, hf_gadu_gadu_login_uin, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_login80_lang, tvb, offset, 2, ENC_ASCII);
	offset += 2;

	offset = dissect_gadu_gadu_login_hash(tvb, tree, offset);

	proto_tree_add_item(tree, hf_gadu_gadu_login_status, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	/* XXX packet not fully dissected */

	return offset;
}

static int
dissect_gadu_gadu_login_ok(tvbuff_t *tvb _U_, packet_info *pinfo, proto_tree *tree _U_, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Login success (< 8.0)");

	/* not empty packet, but content unknown */

	return offset;
}

static int
dissect_gadu_gadu_login_failed(tvbuff_t *tvb _U_, packet_info *pinfo, proto_tree *tree _U_, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Login fail (< 8.0)");

	/* empty packet */

	return offset;
}

static int
dissect_gadu_gadu_login_ok80(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Login success (8.0)");

	proto_tree_add_item(tree, hf_gadu_gadu_data, tvb, offset, 4, ENC_NA);	/* 01 00 00 00 */
	offset += 4;

	return offset;
}

static int
dissect_gadu_gadu_login80_failed(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Login fail (8.0)");

	proto_tree_add_item(tree, hf_gadu_gadu_data, tvb, offset, 4, ENC_NA);	/* 01 00 00 00 */
	offset += 4;

	return offset;
}

static int
dissect_gadu_gadu_user_data(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	uint32_t users_num;

	col_set_str(pinfo->cinfo, COL_INFO, "Contact details");

	/* XXX, add subtrees */

	offset += 4;

	users_num = tvb_get_letohl(tvb, offset);
	offset += 4;

	while (users_num--) {
		uint32_t attr_num;

		proto_tree_add_item(tree, hf_gadu_gadu_userdata_uin, tvb, offset, 4, ENC_LITTLE_ENDIAN);
		offset += 4;

		attr_num = tvb_get_letohl(tvb, offset);
		offset += 4;

		while (attr_num--) {
			uint32_t name_size, val_size;
			char *name, *val;
	/* name */
			name_size = tvb_get_letohl(tvb, offset);
			offset += 4;

			name = tvb_get_string_enc(pinfo->pool, tvb, offset, name_size, ENC_ASCII | ENC_NA);
			proto_tree_add_string(tree, hf_gadu_gadu_userdata_attr_name, tvb, offset - 4, 4 + name_size, name);
			offset += name_size;
	/* type */
			proto_tree_add_item(tree, hf_gadu_gadu_userdata_attr_type, tvb, offset, 4, ENC_LITTLE_ENDIAN);
			offset += 4;
	/* value */
			val_size = tvb_get_letohl(tvb, offset);
			offset += 4;

			val = tvb_get_string_enc(pinfo->pool, tvb, offset, val_size, ENC_ASCII | ENC_NA);
			proto_tree_add_string(tree, hf_gadu_gadu_userdata_attr_value, tvb, offset - 4, 4 + val_size, val);
			offset += val_size;
		}
	}

	return offset;
}

static int
dissect_gadu_gadu_typing_notify(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Typing notify");

	/* XXX, when type > 1, it's length not type ! */
	proto_tree_add_item(tree, hf_gadu_gadu_typing_notify_type, tvb, offset, 2, ENC_LITTLE_ENDIAN);
	offset += 2;

	proto_tree_add_item(tree, hf_gadu_gadu_typing_notify_uin, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	return offset;
}

static int
dissect_gadu_gadu_msg_attr(tvbuff_t *tvb _U_, proto_tree *tree _U_, int offset)
{
	/* XXX, stub */

	return offset;
}

static int
dissect_gadu_gadu_recv_msg(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	struct gadu_gadu_conv_data *conv;
	proto_item *ti;

	col_set_str(pinfo->cinfo, COL_INFO, "Receive message (< 8.0)");

	if ((conv = gadu_gadu_get_conversation_data(pinfo))) {
		ti = proto_tree_add_uint(tree, hf_gadu_gadu_msg_recipient, tvb, 0, 0, conv->uin);
		proto_item_set_generated(ti);

		ti = proto_tree_add_uint(tree, hf_gadu_gadu_msg_uin, tvb, 0, 0, conv->uin);
		proto_item_set_generated(ti);
		proto_item_set_hidden(ti);
	}

	ti = proto_tree_add_item(tree, hf_gadu_gadu_msg_uin, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	proto_item_set_hidden(ti);
	proto_tree_add_item(tree, hf_gadu_gadu_msg_sender, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_msg_seq, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_msg_time, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_msg_class, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	offset = dissect_gadu_gadu_stringz_cp1250(tvb, hf_gadu_gadu_msg_text, tree, offset);

	offset = dissect_gadu_gadu_msg_attr(tvb, tree, offset);

	return offset;
}

static int
dissect_gadu_gadu_send_msg(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	struct gadu_gadu_conv_data *conv;
	proto_item *ti;

	col_set_str(pinfo->cinfo, COL_INFO, "Send message (< 8.0)");

	ti = proto_tree_add_item(tree, hf_gadu_gadu_msg_uin, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	proto_item_set_hidden(ti);
	proto_tree_add_item(tree, hf_gadu_gadu_msg_recipient, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	if ((conv = gadu_gadu_get_conversation_data(pinfo))) {
		ti = proto_tree_add_uint(tree, hf_gadu_gadu_msg_sender, tvb, 0, 0, conv->uin);
		proto_item_set_generated(ti);

		ti = proto_tree_add_uint(tree, hf_gadu_gadu_msg_uin, tvb, 0, 0, conv->uin);
		proto_item_set_generated(ti);
		proto_item_set_hidden(ti);
	}

	proto_tree_add_item(tree, hf_gadu_gadu_msg_seq, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	ti = proto_tree_add_time(tree, hf_gadu_gadu_msg_time, tvb, 0, 0, &(pinfo->abs_ts));
	proto_item_set_generated(ti);

	proto_tree_add_item(tree, hf_gadu_gadu_msg_class, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	offset = dissect_gadu_gadu_stringz_cp1250(tvb, hf_gadu_gadu_msg_text, tree, offset);

	offset = dissect_gadu_gadu_msg_attr(tvb, tree, offset);

	return offset;
}

static int
dissect_gadu_gadu_recv_msg80(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	struct gadu_gadu_conv_data *conv;
	proto_item *ti;

	col_set_str(pinfo->cinfo, COL_INFO, "Receive message (8.0)");

	if ((conv = gadu_gadu_get_conversation_data(pinfo))) {
		ti = proto_tree_add_uint(tree, hf_gadu_gadu_msg_recipient, tvb, 0, 0, conv->uin);
		proto_item_set_generated(ti);

		ti = proto_tree_add_uint(tree, hf_gadu_gadu_msg_uin, tvb, 0, 0, conv->uin);
		proto_item_set_generated(ti);
		proto_item_set_hidden(ti);
	}

	ti = proto_tree_add_item(tree, hf_gadu_gadu_msg_uin, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	proto_item_set_hidden(ti);
	proto_tree_add_item(tree, hf_gadu_gadu_msg_sender, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_msg_seq, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_msg_time, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_msg_class, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_msg80_offset_plain, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_msg80_offset_attr, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	/* XXX packet not fully dissected */

	return offset;
}

static int
dissect_gadu_gadu_send_msg80(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	struct gadu_gadu_conv_data *conv;
	proto_item *ti;

	col_set_str(pinfo->cinfo, COL_INFO, "Send message (8.0)");

	ti = proto_tree_add_item(tree, hf_gadu_gadu_msg_uin, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	proto_item_set_hidden(ti);
	proto_tree_add_item(tree, hf_gadu_gadu_msg_recipient, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	if ((conv = gadu_gadu_get_conversation_data(pinfo))) {
		ti = proto_tree_add_uint(tree, hf_gadu_gadu_msg_sender, tvb, 0, 0, conv->uin);
		proto_item_set_generated(ti);

		ti = proto_tree_add_uint(tree, hf_gadu_gadu_msg_uin, tvb, 0, 0, conv->uin);
		proto_item_set_generated(ti);
		proto_item_set_hidden(ti);
	}

	proto_tree_add_item(tree, hf_gadu_gadu_msg_seq, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	ti = proto_tree_add_time(tree, hf_gadu_gadu_msg_time, tvb, 0, 0, &(pinfo->abs_ts));
	proto_item_set_generated(ti);

	proto_tree_add_item(tree, hf_gadu_gadu_msg_class, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_msg80_offset_plain, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_msg80_offset_attr, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	/* XXX packet not fully dissected */

	return offset;
}

static int
dissect_gadu_gadu_send_msg_ack(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Message acknowledge (server)");

	proto_tree_add_item(tree, hf_gadu_gadu_msg_ack_status, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_msg_ack_recipient, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_msg_ack_seq, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	return offset;
}

static int
dissect_gadu_gadu_recv_msg_ack(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Message acknowledge (client)");

	proto_tree_add_item(tree, hf_gadu_gadu_msg_ack_seq, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	return offset;
}

static int
dissect_gadu_gadu_status60(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	uint32_t uin;
	uint8_t status;

	col_set_str(pinfo->cinfo, COL_INFO, "Receive status (6.0)");

	uin = tvb_get_letohl(tvb, offset) & ~(GG_ERA_OMNIX_MASK | GG_HAS_AUDIO_MASK);
	proto_tree_add_uint(tree, hf_gadu_gadu_status_uin, tvb, offset, 4, uin);
	offset += 4;

	status = tvb_get_uint8(tvb, offset);
	proto_tree_add_item(tree, hf_gadu_gadu_status_status, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	proto_tree_add_item(tree, hf_gadu_gadu_status_ip, tvb, offset, 4, ENC_BIG_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_status_port, tvb, offset, 2, ENC_LITTLE_ENDIAN);
	offset += 2;

	proto_tree_add_item(tree, hf_gadu_gadu_status_version, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	proto_tree_add_item(tree, hf_gadu_gadu_status_img_size, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	proto_tree_add_item(tree, hf_gadu_gadu_data, tvb, offset, 1, ENC_NA);	/* 00 */
	offset += 1;

	if (gadu_gadu_status_has_descr(status))
		offset = dissect_gadu_gadu_stringz_cp1250(tvb, hf_gadu_gadu_status_descr, tree, offset);

	return offset;
}

static int
dissect_gadu_gadu_status77(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	uint32_t uin;
	uint8_t status;

	col_set_str(pinfo->cinfo, COL_INFO, "Receive status (7.7)");

	uin = tvb_get_letohl(tvb, offset) & ~(GG_ERA_OMNIX_MASK | GG_HAS_AUDIO_MASK);
	proto_tree_add_uint(tree, hf_gadu_gadu_status_uin, tvb, offset, 4, uin);
	offset += 4;

	status = tvb_get_uint8(tvb, offset);
	proto_tree_add_item(tree, hf_gadu_gadu_status_status, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	proto_tree_add_item(tree, hf_gadu_gadu_status_ip, tvb, offset, 4, ENC_BIG_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_status_port, tvb, offset, 2, ENC_LITTLE_ENDIAN);
	offset += 2;

	proto_tree_add_item(tree, hf_gadu_gadu_status_version, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	proto_tree_add_item(tree, hf_gadu_gadu_status_img_size, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	proto_tree_add_item(tree, hf_gadu_gadu_data, tvb, offset, 1, ENC_NA);	/* 00 */
	offset += 1;

	proto_tree_add_item(tree, hf_gadu_gadu_data, tvb, offset, 4, ENC_NA);
	offset += 4;

	if (gadu_gadu_status_has_descr(status))
		offset = dissect_gadu_gadu_stringz_cp1250(tvb, hf_gadu_gadu_status_descr, tree, offset);

	return offset;
}

static int
dissect_gadu_gadu_status80(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Receive status (8.0)");

	proto_tree_add_item(tree, hf_gadu_gadu_status_uin, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_status_status, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_data, tvb, offset, 4, ENC_NA);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_status_ip, tvb, offset, 4, ENC_BIG_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_status_port, tvb, offset, 2, ENC_LITTLE_ENDIAN);
	offset += 2;

	proto_tree_add_item(tree, hf_gadu_gadu_status_img_size, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	proto_tree_add_item(tree, hf_gadu_gadu_data, tvb, offset, 1, ENC_NA);
	offset += 1;

	proto_tree_add_item(tree, hf_gadu_gadu_data, tvb, offset, 4, ENC_NA);
	offset += 4;

	offset = dissect_gadu_gadu_uint32_string_utf8(tvb, hf_gadu_gadu_status_descr, tree, offset);

	return offset;
}

static int
dissect_gadu_gadu_notify_reply80(tvbuff_t *tvb _U_, packet_info *pinfo, proto_tree *tree _U_, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Receive status list (8.0)");

	/* XXX packet not fully dissected */

	return offset;
}

static int
dissect_gadu_gadu_new_status(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	uint32_t status;

	col_set_str(pinfo->cinfo, COL_INFO, "New status (< 8.0)");

	status = tvb_get_letohl(tvb, offset);
	proto_tree_add_item(tree, hf_gadu_gadu_new_status_status, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	if (gadu_gadu_status_has_descr(status & 0xff))
		offset = dissect_gadu_gadu_stringz_cp1250(tvb, hf_gadu_gadu_status_descr, tree, offset);

	return offset;
}

static int
dissect_gadu_gadu_new_status80(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "New status (8.0)");

	proto_tree_add_item(tree, hf_gadu_gadu_new_status_status, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_data, tvb, offset, 4, ENC_NA);
	offset += 4;

	offset = dissect_gadu_gadu_uint32_string_utf8(tvb, hf_gadu_gadu_new_status_desc, tree, offset);

	return offset;
}

static int
dissect_gadu_gadu_list_empty(tvbuff_t *tvb _U_, packet_info *pinfo, proto_tree *tree _U_, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Notify list (empty)");

	/* empty packet */

	return offset;
}

static int
dissect_gadu_gadu_add_notify(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Notify list add");

	proto_tree_add_item(tree, hf_gadu_gadu_contact_uin, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_contact_type, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	return offset;
}

static int
dissect_gadu_gadu_notify105_common(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset, char **puin)
{
	uint16_t uin_len;
	char *uin;

	proto_tree_add_item(tree, hf_gadu_gadu_data, tvb, offset, 1, ENC_NA); /* unknown 00 */
	offset += 1;

	uin_len = tvb_get_uint8(tvb, offset);
	offset += 1;
	uin = tvb_get_string_enc(pinfo->pool, tvb, offset, uin_len, ENC_ASCII | ENC_NA);
	proto_tree_add_string(tree, hf_gadu_gadu_contact_uin_str, tvb, offset - 1, 1 + uin_len, uin);
	offset += uin_len;
	if (puin)
		*puin = uin;

	proto_tree_add_item(tree, hf_gadu_gadu_contact_type, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	return offset;
}

static int
dissect_gadu_gadu_add_notify105(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Notify list add (10.5)");

	return dissect_gadu_gadu_notify105_common(tvb, pinfo, tree, offset, NULL);
}

static int
dissect_gadu_gadu_remove_notify(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Notify list remove");

	proto_tree_add_item(tree, hf_gadu_gadu_contact_uin, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_gadu_gadu_contact_type, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	return offset;
}

static int
dissect_gadu_gadu_remove_notify105(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Notify list remove (10.5)");

	return dissect_gadu_gadu_notify105_common(tvb, pinfo, tree, offset, NULL);
}

static int
dissect_gadu_gadu_notify_common(tvbuff_t *tvb, packet_info *pinfo _U_, proto_tree *tree, int offset)
{
	proto_tree *contact_tree;

	while (tvb_reported_length_remaining(tvb, offset) >= 4+1) {
		uint32_t uin = tvb_get_letohl(tvb, offset);

		contact_tree = proto_tree_add_subtree_format(tree, tvb, offset, 5,
								ett_gadu_gadu_contact, NULL, "Contact: %u", uin);

		proto_tree_add_item(contact_tree, hf_gadu_gadu_contact_uin, tvb, offset, 4, ENC_LITTLE_ENDIAN);
		offset += 4;

		proto_tree_add_item(contact_tree, hf_gadu_gadu_contact_type, tvb, offset, 1, ENC_LITTLE_ENDIAN);
		offset += 1;
	}

	return offset;
}

static int
dissect_gadu_gadu_notify_first(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Notify list");

	return dissect_gadu_gadu_notify_common(tvb, pinfo, tree, offset);
}

static int
dissect_gadu_gadu_notify_last(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Notify list (last)");

	return dissect_gadu_gadu_notify_common(tvb, pinfo, tree, offset);
}

static int
dissect_gadu_gadu_notify105(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Notify list (10.5)");

	while (tvb_reported_length_remaining(tvb, offset) >= 2) {
		const int org_offset = offset;

		proto_tree *contact_tree;
		proto_item *ti;

		char *uin;

		contact_tree = proto_tree_add_subtree(tree, tvb, offset, 0, ett_gadu_gadu_contact, &ti, "Contact: ");

		offset = dissect_gadu_gadu_notify105_common(tvb, pinfo, contact_tree, offset, &uin);
		proto_item_append_text(ti, "%s", uin);

		proto_item_set_len(ti, offset - org_offset);
	}

	return offset;
}

static int
dissect_gadu_gadu_ping(tvbuff_t *tvb _U_, packet_info *pinfo, proto_tree *tree _U_, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Ping");

	/* empty packet */

	return offset;
}

static int
dissect_gadu_gadu_welcome(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Welcome");

	proto_tree_add_item(tree, hf_gadu_gadu_welcome_seed, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	return offset;
}

static int
dissect_gadu_gadu_userlist_xml_compressed(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	int remain = tvb_reported_length_remaining(tvb, offset);
	tvbuff_t *uncomp_tvb;

	if (remain <= 0)
		return offset;

	if ((uncomp_tvb = tvb_child_uncompress_zlib(tvb, tvb, offset, remain))) {
		proto_tree_add_bytes_format_value(tree, hf_gadu_gadu_userlist, tvb, offset, remain, NULL, "%s", "[Decompression succeeded]");

		add_new_data_source(pinfo, uncomp_tvb, "Uncompressed userlist");

		/* XXX add DTD (pinfo->match_string) */
		call_dissector_only(xml_handle, uncomp_tvb, pinfo, tree, NULL);
	} else
		proto_tree_add_bytes_format_value(tree, hf_gadu_gadu_userlist, tvb, offset, remain, NULL, "%s", "[Error: Decompression failed] (or no zlib)");

	offset += remain;

	return offset;
}

static int
dissect_gadu_gadu_userlist_request80(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	uint8_t type;
	proto_item *ti;

	col_set_str(pinfo->cinfo, COL_INFO, "Userlist request (8.0)");

	type = tvb_get_uint8(tvb, offset);
	proto_tree_add_item(tree, hf_gadu_gadu_userlist_request_type, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	ti = proto_tree_add_uint(tree, hf_gadu_gadu_userlist_format, tvb, 0, 0, GG_USERLIST100_FORMAT_TYPE_GG100);
	proto_item_set_generated(ti);

	switch (type) {
		case GG_USERLIST_PUT:
			offset = dissect_gadu_gadu_userlist_xml_compressed(tvb, pinfo, tree, offset);
			break;
	}

	return offset;
}

static int
dissect_gadu_gadu_userlist_request100(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	uint8_t type, format;

	col_set_str(pinfo->cinfo, COL_INFO, "Userlist request (10.0)");

	type = tvb_get_uint8(tvb, offset);
	proto_tree_add_item(tree, hf_gadu_gadu_userlist_request_type, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	proto_tree_add_item(tree, hf_gadu_gadu_userlist_version, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	format = tvb_get_uint8(tvb, offset);
	proto_tree_add_item(tree, hf_gadu_gadu_userlist_format, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	proto_tree_add_item(tree, hf_gadu_gadu_data, tvb, offset, 1, ENC_NA);	/* 01 */
	offset += 1;

	switch (type) {
		case GG_USERLIST_PUT:
			if (format == GG_USERLIST100_FORMAT_TYPE_GG100)
				offset = dissect_gadu_gadu_userlist_xml_compressed(tvb, pinfo, tree, offset);
			break;
	}

	return offset;
}

static int
dissect_gadu_gadu_userlist_reply80(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	uint8_t type;

	col_set_str(pinfo->cinfo, COL_INFO, "Userlist reply (8.0)");

	type = tvb_get_uint8(tvb, offset);
	proto_tree_add_item(tree, hf_gadu_gadu_userlist_reply_type, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	switch (type) {
		case GG_USERLIST_GET_REPLY:
			offset = dissect_gadu_gadu_userlist_xml_compressed(tvb, pinfo, tree, offset);
			break;
	}

	return offset;
}

static int
dissect_gadu_gadu_userlist_reply100(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	uint8_t type, format;

	col_set_str(pinfo->cinfo, COL_INFO, "Userlist reply (10.0)");

	type = tvb_get_uint8(tvb, offset);
	proto_tree_add_item(tree, hf_gadu_gadu_userlist_reply_type, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	proto_tree_add_item(tree, hf_gadu_gadu_userlist_version, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	format = tvb_get_uint8(tvb, offset);
	proto_tree_add_item(tree, hf_gadu_gadu_userlist_format, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	proto_tree_add_item(tree, hf_gadu_gadu_data, tvb, offset, 1, ENC_NA);	/* 01 */
	offset += 1;

	switch (type) {
		case GG_USERLIST_GET_REPLY:
			if (format == GG_USERLIST100_FORMAT_TYPE_GG100)
				offset = dissect_gadu_gadu_userlist_xml_compressed(tvb, pinfo, tree, offset);
			break;
	}

	return offset;
}

static int
dissect_gadu_gadu_userlist_version100(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Userlist version (10.0)");

	proto_tree_add_item(tree, hf_gadu_gadu_userlist_version, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	return offset;
}

static int
dissect_gadu_gadu_dcc7_id_request(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Direct connection id request");

	proto_tree_add_item(tree, hf_dcc_type, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	return offset;
}

static int
dissect_gadu_gadu_dcc7_id_reply(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Direct connection id reply");

	proto_tree_add_item(tree, hf_dcc_type, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_dcc_id, tvb, offset, 8, ENC_NA);
	offset += 8;

	return offset;
}

static int
dissect_gadu_gadu_dcc7_new(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Direct connection new");

	proto_tree_add_item(tree, hf_dcc_id, tvb, offset, 8, ENC_NA);
	offset += 8;

	proto_tree_add_item(tree, hf_dcc_uin_from, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_dcc_uin_to, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_dcc_type, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_dcc_filename, tvb, offset, 255, ENC_ASCII);
	offset += 255;

	return offset;
}

static int
dissect_gadu_gadu_dcc7_id_abort(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	col_set_str(pinfo->cinfo, COL_INFO, "Direct connection abort");

	proto_tree_add_item(tree, hf_dcc_id, tvb, offset, 8, ENC_NA);
	offset += 8;

	proto_tree_add_item(tree, hf_dcc_uin_from, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	proto_tree_add_item(tree, hf_dcc_uin_to, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	return offset;
}

static int
dissect_gadu_gadu_pubdir50_request(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	int pos;

	col_set_str(pinfo->cinfo, COL_INFO, "Public directory request");

	proto_tree_add_item(tree, hf_gadu_gadu_pubdir_request_type, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	proto_tree_add_item(tree, hf_gadu_gadu_pubdir_request_seq, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;
	/* XXX, link request sequence with reply sequence */

	while ((pos = tvb_find_uint8(tvb, offset, -1, '\0')) > 0) {
		/* XXX, display it better, field=value */
		proto_tree_add_item(tree, hf_gadu_gadu_pubdir_request_str, tvb, offset, (pos - offset) + 1, ENC_WINDOWS_1250);
		offset = pos + 1;
	}

	return offset;
}

static int
dissect_gadu_gadu_pubdir50_reply(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	int pos;

	col_set_str(pinfo->cinfo, COL_INFO, "Public directory reply");

	proto_tree_add_item(tree, hf_gadu_gadu_pubdir_reply_type, tvb, offset, 1, ENC_LITTLE_ENDIAN);
	offset += 1;

	proto_tree_add_item(tree, hf_gadu_gadu_pubdir_reply_seq, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;
	/* XXX, link reply sequence with request sequence */

	while ((pos = tvb_find_uint8(tvb, offset, -1, '\0')) > 0) {
		/* XXX, display it better, field=value */
		proto_tree_add_item(tree, hf_gadu_gadu_pubdir_reply_str, tvb, offset, (pos - offset) + 1, ENC_WINDOWS_1250);
		offset = pos + 1;
	}

	return offset;
}

static int
dissect_gadu_gadu_xml_action(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, int offset)
{
	tvbuff_t *xml_tvb;
	int ret;

	col_set_str(pinfo->cinfo, COL_INFO, "XML action message");

	xml_tvb = tvb_new_subset_remaining(tvb, offset);

	/* XXX add DTD (pinfo->match_string) */
	ret = call_dissector_only(xml_handle, xml_tvb, pinfo, tree, NULL);

	return offset + ret;
}

static int
dissect_gadu_gadu_pdu(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void* data _U_)
{
	proto_tree *gadu_gadu_tree = NULL;

	int offset = 0;
	uint32_t pkt_type;

	col_clear(pinfo->cinfo, COL_INFO); /* XXX, remove, add separator when multiple PDU */

	if (tree) {
		proto_item *ti = proto_tree_add_item(tree, proto_gadu_gadu, tvb, 0, -1, ENC_NA);
		gadu_gadu_tree = proto_item_add_subtree(ti, ett_gadu_gadu);
	}

	pkt_type = tvb_get_letohl(tvb, offset);
	proto_tree_add_item(gadu_gadu_tree, (pinfo->p2p_dir == P2P_DIR_RECV) ? hf_gadu_gadu_header_type_recv : hf_gadu_gadu_header_type_send, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;
	proto_tree_add_item(gadu_gadu_tree, hf_gadu_gadu_header_length, tvb, offset, 4, ENC_LITTLE_ENDIAN);
	offset += 4;

	if (pinfo->p2p_dir == P2P_DIR_RECV) {
		switch (pkt_type) {
			case GG_DISCONNECTING:
				offset = dissect_gadu_gadu_disconnecting(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_DISCONNECT_ACK:
				offset = dissect_gadu_gadu_disconnect_ack(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_LOGIN_OK:
				offset = dissect_gadu_gadu_login_ok(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_LOGIN_OK80:
				offset = dissect_gadu_gadu_login_ok80(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_LOGIN_FAILED:
				offset = dissect_gadu_gadu_login_failed(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_LOGIN80_FAILED:
				offset = dissect_gadu_gadu_login80_failed(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_USER_DATA:
				offset = dissect_gadu_gadu_user_data(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_TYPING_NOTIFY:
				offset = dissect_gadu_gadu_typing_notify(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_RECV_MSG:
				offset = dissect_gadu_gadu_recv_msg(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_RECV_MSG80:
				offset = dissect_gadu_gadu_recv_msg80(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_SEND_MSG_ACK:
				/* GG_SEND_MSG_ACK is received by client */
				offset = dissect_gadu_gadu_send_msg_ack(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_STATUS60:
				offset = dissect_gadu_gadu_status60(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_STATUS77:
				offset = dissect_gadu_gadu_status77(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_STATUS80:
				offset = dissect_gadu_gadu_status80(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_NOTIFY_REPLY80:
				offset = dissect_gadu_gadu_notify_reply80(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_DCC7_ID_REPLY:
				offset = dissect_gadu_gadu_dcc7_id_reply(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_WELCOME:
				offset = dissect_gadu_gadu_welcome(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_USERLIST_REPLY80:
				offset = dissect_gadu_gadu_userlist_reply80(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_USERLIST100_REPLY:
				offset = dissect_gadu_gadu_userlist_reply100(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_USERLIST100_VERSION:
				offset = dissect_gadu_gadu_userlist_version100(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_PUBDIR50_REPLY:
				offset = dissect_gadu_gadu_pubdir50_reply(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_XML_ACTION:
				offset = dissect_gadu_gadu_xml_action(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_STATUS:
			case GG_PONG:
			case GG_PING:
			case GG_NOTIFY_REPLY:
			case GG_USERLIST_REPLY:
			case GG_NOTIFY_REPLY60:
			case GG_NEED_EMAIL:
			case GG_LOGIN_HASH_TYPE_INVALID:
			case GG_NOTIFY_REPLY77:
			case GG_DCC7_INFO:
			case GG_DCC7_NEW:
			case GG_DCC7_ACCEPT:
			case GG_DCC7_REJECT:
			case GG_DCC7_ID_ABORTED:
			case GG_XML_EVENT:
			case GG_STATUS80BETA:
			case GG_NOTIFY_REPLY80BETA:
			case GG_OWN_MESSAGE:
			case GG_OWN_RESOURCE_INFO:
			default:
			{
				const char *pkt_name = try_val_to_str(pkt_type, gadu_gadu_packets_type_recv);

				if (pkt_name)
					col_set_str(pinfo->cinfo, COL_INFO, pkt_name);
				else
					col_add_fstr(pinfo->cinfo, COL_INFO, "Unknown recv packet: %.2x", pkt_type);
				break;
			}
		}

	} else {
		switch (pkt_type) {
			case GG_LOGIN:
				offset = dissect_gadu_gadu_login(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_LOGIN70:
				offset = dissect_gadu_gadu_login70(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_LOGIN80:
				offset = dissect_gadu_gadu_login80(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_LIST_EMPTY:
				offset = dissect_gadu_gadu_list_empty(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_NOTIFY_FIRST:
				offset = dissect_gadu_gadu_notify_first(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_NOTIFY_LAST:
				offset = dissect_gadu_gadu_notify_last(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_NOTIFY105:
				offset = dissect_gadu_gadu_notify105(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_ADD_NOTIFY:
				offset = dissect_gadu_gadu_add_notify(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_ADD_NOTIFY105:
				offset = dissect_gadu_gadu_add_notify105(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_REMOVE_NOTIFY:
				offset = dissect_gadu_gadu_remove_notify(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_REMOVE_NOTIFY105:
				offset = dissect_gadu_gadu_remove_notify105(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_PING:
				offset = dissect_gadu_gadu_ping(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_TYPING_NOTIFY:
				offset = dissect_gadu_gadu_typing_notify(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_SEND_MSG:
				offset = dissect_gadu_gadu_send_msg(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_SEND_MSG80:
				offset = dissect_gadu_gadu_send_msg80(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_RECV_MSG_ACK:
				/* GG_RECV_MSG_ACK is send by client */
				offset = dissect_gadu_gadu_recv_msg_ack(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_NEW_STATUS:
				offset = dissect_gadu_gadu_new_status(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_NEW_STATUS80:
				offset = dissect_gadu_gadu_new_status80(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_DCC7_ID_REQUEST:
				offset = dissect_gadu_gadu_dcc7_id_request(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_DCC7_NEW:
				offset = dissect_gadu_gadu_dcc7_new(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_DCC7_ID_ABORT:
				offset = dissect_gadu_gadu_dcc7_id_abort(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_USERLIST_REQUEST80:
				offset = dissect_gadu_gadu_userlist_request80(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_USERLIST100_REQUEST:
				offset = dissect_gadu_gadu_userlist_request100(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_PUBDIR50_REQUEST:
				offset = dissect_gadu_gadu_pubdir50_request(tvb, pinfo, gadu_gadu_tree, offset);
				break;

			case GG_PONG:
			case GG_LOGIN_EXT:
			case GG_LOGIN60:
			case GG_USERLIST_REQUEST:
			case GG_DCC7_INFO:
			case GG_DCC7_ACCEPT:
			case GG_DCC7_REJECT:
			case GG_DCC7_ID_DUNNO1:
			case GG_NEW_STATUS80BETA:
			case GG_LOGIN80BETA:
			case GG_OWN_DISCONNECT:
			case GG_NEW_STATUS105:
			case GG_LOGIN105:
			default:
			{
				const char *pkt_name = try_val_to_str(pkt_type, gadu_gadu_packets_type_send);

				if (pkt_name)
					col_set_str(pinfo->cinfo, COL_INFO, pkt_name);
				else
					col_add_fstr(pinfo->cinfo, COL_INFO, "Unknown send packet: %.2x", pkt_type);
				break;
			}
		}
	}

	/* for now display rest of data as FT_BYTES. */
	if (tvb_reported_length_remaining(tvb, offset) > 0) {
		proto_tree_add_item(gadu_gadu_tree, hf_gadu_gadu_data, tvb, offset, -1, ENC_NA);
	}

	return tvb_captured_length(tvb);
}

static unsigned
get_gadu_gadu_pdu_len(packet_info *pinfo _U_, tvbuff_t *tvb,
                      int offset, void *data _U_)
{
	uint32_t len = tvb_get_letohl(tvb, offset + 4);

	return len + 8;
}

static int
dissect_gadu_gadu(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data)
{
	if (pinfo->srcport == pinfo->match_uint && pinfo->destport != pinfo->match_uint)
		pinfo->p2p_dir = P2P_DIR_RECV;
	else if (pinfo->srcport != pinfo->match_uint && pinfo->destport == pinfo->match_uint)
		pinfo->p2p_dir = P2P_DIR_SENT;
	else
		return 0;

	col_set_str(pinfo->cinfo, COL_PROTOCOL, "Gadu-Gadu");
	col_clear(pinfo->cinfo, COL_INFO);

	tcp_dissect_pdus(tvb, pinfo, tree, gadu_gadu_desegment, 8, get_gadu_gadu_pdu_len, dissect_gadu_gadu_pdu, data);
	return tvb_captured_length(tvb);
}

void
proto_register_gadu_gadu(void)
{
	static hf_register_info hf[] = {
		{ &hf_gadu_gadu_header_type_recv,
			{ "Packet Type", "gadu-gadu.recv",
			  FT_UINT32, BASE_HEX, VALS(gadu_gadu_packets_type_recv), 0x0,
			  "Packet Type (recv)", HFILL }
		},
		{ &hf_gadu_gadu_header_type_send,
			{ "Packet Type", "gadu-gadu.send",
			  FT_UINT32, BASE_HEX, VALS(gadu_gadu_packets_type_send), 0x0,
			  "Packet Type (send)", HFILL }
		},
		{ &hf_gadu_gadu_header_length,
			{ "Packet Length", "gadu-gadu.len",
			  FT_UINT32, BASE_DEC, NULL, 0x0,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_login_uin,
			{ "Client UIN", "gadu-gadu.login.uin",
			  FT_UINT32, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_login_hash_type,
			{ "Login hash type", "gadu-gadu.login.hash_type",
			  FT_UINT8, BASE_HEX, VALS(gadu_gadu_hash_type_vals), 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_login_hash,
			{ "Login hash", "gadu-gadu.login.hash",
			  FT_BYTES, BASE_NONE, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_login_status,
			{ "Client status", "gadu-gadu.login.status",
			  FT_UINT32, BASE_HEX, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_login_protocol,
			{ "Client protocol", "gadu-gadu.login.protocol",
			  FT_UINT32, BASE_HEX, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_login_version,
			{ "Client version", "gadu-gadu.login.version",
			  FT_STRING, BASE_NONE, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_login_local_ip,
			{ "Client local IP", "gadu-gadu.login.local_ip",
			  FT_IPv4, BASE_NONE, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_login_local_port,
			{ "Client local port", "gadu-gadu.login.local_port",
			  FT_UINT16, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_login80_lang,
			{ "Client language", "gadu-gadu.login80.lang",
			  FT_STRING, BASE_NONE, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_userdata_uin,
			{ "UIN", "gadu-gadu.user_data.uin",
			  FT_UINT32, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_userdata_attr_name,
			{ "Attribute name", "gadu-gadu.user_data.attr_name",
			  FT_STRING, BASE_NONE, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_userdata_attr_type,
			{ "Attribute type", "gadu-gadu.user_data.attr_type",
			  FT_UINT32, BASE_HEX, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_userdata_attr_value,
			{ "Attribute value", "gadu-gadu.user_data.attr_val",
			  FT_STRING, BASE_NONE, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_typing_notify_type,
			{ "Typing notify type", "gadu-gadu.typing_notify.type",
			  FT_UINT16, BASE_HEX, VALS(gadu_gadu_typing_notify_type_vals), 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_typing_notify_uin,
			{ "Typing notify recipient", "gadu-gadu.typing_notify.uin",
			  FT_UINT32, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_msg_uin,
			{ "Message sender or recipient", "gadu-gadu.msg.uin",
			  FT_UINT32, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_msg_sender,
			{ "Message sender", "gadu-gadu.msg.sender",
			  FT_UINT32, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_msg_recipient,
			{ "Message recipient", "gadu-gadu.msg.recipient",
			  FT_UINT32, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_msg_seq,
			{ "Message sequence number", "gadu-gadu.msg.seq",
			  FT_UINT32, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_msg_time,
			{ "Message time", "gadu-gadu.msg.time",
			  FT_ABSOLUTE_TIME, ABSOLUTE_TIME_LOCAL, NULL, 0x0,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_msg_class,
			{ "Message class", "gadu-gadu.msg.class",
			  FT_UINT32, BASE_HEX, NULL, 0x0,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_msg_text,
			{ "Message text", "gadu-gadu.msg.text",
			  FT_STRINGZ, BASE_NONE, NULL, 0x0,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_msg80_offset_plain,
			{ "Message plaintext offset", "gadu-gadu.msg80.offset_plain",
			  FT_UINT32, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_msg80_offset_attr,
			{ "Message attribute offset", "gadu-gadu.msg80.offset_attributes",
			  FT_UINT32, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_msg_ack_status,
			{ "Message status", "gadu-gadu.msg_ack.status",
			  FT_UINT32, BASE_HEX, VALS(gadu_gadu_msg_ack_status_vals), 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_msg_ack_recipient,
			{ "Message recipient", "gadu-gadu.msg_ack.recipient",
			  FT_UINT32, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_msg_ack_seq,
			{ "Message sequence number", "gadu-gadu.msg_ack.seq",
			  FT_UINT32, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_status_uin,
			{ "UIN", "gadu-gadu.status.uin",
			  FT_UINT32, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_status_status,
			{ "Status", "gadu-gadu.status.status",
			  FT_UINT32, BASE_HEX, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_status_ip,
			{ "IP", "gadu-gadu.status.remote_ip",
			  FT_IPv4, BASE_NONE, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_status_port,
			{ "Port", "gadu-gadu.status.remote_port",
			  FT_UINT16, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_status_version,
			{ "Version", "gadu-gadu.status.version",
			  FT_UINT8, BASE_HEX, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_status_img_size,
			{ "Image size", "gadu-gadu.status.image_size",
			  FT_UINT8, BASE_DEC, NULL, 0x00,
			  "Maximum image size in KB", HFILL }
		},
		{ &hf_gadu_gadu_status_descr,
			{ "Description", "gadu-gadu.status.description",
			  FT_STRINGZ, BASE_NONE, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_dcc_type,
			{ "Direct connection type", "gadu-gadu.dcc.type",
			  FT_UINT32, BASE_HEX, VALS(gadu_gadu_dcc_type_vals), 0x00,
			  NULL, HFILL }
		},
		{ &hf_dcc_id,
			{ "Direct connection id", "gadu-gadu.dcc.id",
			  FT_BYTES, BASE_NONE, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_dcc_uin_to,
			{ "Direct connection UIN target", "gadu-gadu.dcc.uin_to",
			  FT_UINT32, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_dcc_uin_from,
			{ "Direct connection UIN initiator", "gadu-gadu.dcc.uin_from",
			  FT_UINT32, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_dcc_filename,
			{ "Direct connection filename", "gadu-gadu.dcc.filename",
			  FT_STRING, BASE_NONE, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_new_status_status,
			{ "Status", "gadu-gadu.new_status.status",
			  FT_UINT32, BASE_HEX, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_new_status_desc,
			{ "Description", "gadu-gadu.new_status.description",
			  FT_STRINGZ, BASE_NONE, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_userlist_request_type,
			{ "Request type", "gadu-gadu.userlist.request_type",
			  FT_UINT32, BASE_HEX, VALS(gadu_gadu_userlist_request_type_vals), 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_userlist_version,
			{ "Userlist version", "gadu-gadu.userlist.version",
			  FT_UINT32, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_userlist_format,
			{ "Userlist format", "gadu-gadu.userlist.format",
			  FT_UINT8, BASE_HEX, VALS(gadu_gadu_userlist_request_format_vals), 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_userlist_reply_type,
			{ "Reply type", "gadu-gadu.userlist.reply_type",
			  FT_UINT32, BASE_HEX, VALS(gadu_gadu_userlist_reply_type_vals), 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_userlist,
			{ "Userlist XML data", "gadu-gadu.userlist",
			  FT_BYTES, BASE_NONE, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_pubdir_request_type,
			{ "Request type", "gadu-gadu.pubdir.request_type",
			  FT_UINT8, BASE_HEX, VALS(gadu_gadu_pubdir_type_vals), 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_pubdir_request_seq,
			{ "Request sequence", "gadu-gadu.pubdir.request_seq",
			  FT_UINT32, BASE_HEX, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_pubdir_request_str,
			{ "Request string", "gadu-gadu.pubdir.request_str",
			  FT_STRINGZ, BASE_NONE, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_pubdir_reply_type,
			{ "Reply type", "gadu-gadu.pubdir.reply_type",
			  FT_UINT8, BASE_HEX, VALS(gadu_gadu_pubdir_type_vals), 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_pubdir_reply_seq,
			{ "Reply sequence", "gadu-gadu.pubdir.reply_seq",
			  FT_UINT32, BASE_HEX, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_pubdir_reply_str,
			{ "Reply string", "gadu-gadu.pubdir.reply_str",
			  FT_STRINGZ, BASE_NONE, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_contact_uin,
			{ "UIN", "gadu-gadu.contact.uin",
			  FT_UINT32, BASE_DEC, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_contact_uin_str,
			{ "UIN", "gadu-gadu.contact.uin_str",
			  FT_STRING, BASE_NONE, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_contact_type,
			{ "Type", "gadu-gadu.contact.type",
			  FT_UINT8, BASE_HEX, NULL, 0x00,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_welcome_seed,
			{ "Seed", "gadu-gadu.welcome.seed",
			  FT_UINT32, BASE_HEX, NULL, 0x0,
			  NULL, HFILL }
		},
		{ &hf_gadu_gadu_data,
			{ "Packet Data", "gadu-gadu.data",
			  FT_BYTES, BASE_NONE, NULL, 0x0,
			  NULL, HFILL }
		},
	};

	static int *ett[] = {
		&ett_gadu_gadu,
		&ett_gadu_gadu_contact
	};

	module_t *gadu_gadu_module;

	proto_gadu_gadu = proto_register_protocol("Gadu-Gadu Protocol", "Gadu-Gadu", "gadu-gadu");

	gadu_gadu_module = prefs_register_protocol(proto_gadu_gadu, NULL);
	prefs_register_bool_preference(gadu_gadu_module, "desegment",
			"Reassemble Gadu-Gadu messages spanning multiple TCP segments",
			"Whether the Gadu-Gadu dissector should reassemble messages spanning multiple TCP segments."
			"To use this option, you must also enable \"Allow subdissectors to reassemble TCP streams\" in the TCP protocol settings.",
			&gadu_gadu_desegment);

	proto_register_field_array(proto_gadu_gadu, hf, array_length(hf));
	proto_register_subtree_array(ett, array_length(ett));

	gadu_gadu_handle = register_dissector("gadugadu", dissect_gadu_gadu, proto_gadu_gadu);
}

void
proto_reg_handoff_gadu_gadu(void)
{
	dissector_add_uint_with_preference("tcp.port", TCP_PORT_GADU_GADU, gadu_gadu_handle);

	xml_handle = find_dissector_add_dependency("xml", proto_gadu_gadu);
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

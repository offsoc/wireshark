/* Do not modify this file. Changes will be overwritten.                      */
/* Generated automatically by the ASN.1 to Wireshark dissector compiler       */
/* packet-camel.c                                                             */
/* asn2wrs.py -b -q -L -p camel -c ./camel.cnf -s ./packet-camel-template -D . -O ../.. CAP-object-identifiers.asn CAP-classes.asn CAP-datatypes.asn CAP-errorcodes.asn CAP-errortypes.asn CAP-operationcodes.asn CAP-GPRS-ReferenceNumber.asn CAP-gsmSCF-gsmSRF-ops-args.asn CAP-gsmSSF-gsmSCF-ops-args.asn CAP-gprsSSF-gsmSCF-ops-args.asn CAP-SMS-ops-args.asn CAP-U-ABORT-Data.asn CamelV2diff.asn ../ros/Remote-Operations-Information-Objects.asn ../ros/Remote-Operations-Generic-ROS-PDUs.asn */

/* packet-camel-template.c
 * Routines for Camel
 * Copyright 2004, Tim Endean <endeant@hotmail.com>
 * Copyright 2005, Olivier Jacques <olivier.jacques@hp.com>
 * Copyright 2005, Javier Acuna <javier.acuna@sixbell.com>
 * Updated to ETSI TS 129 078 V6.4.0 (2004-3GPP TS 29.078 version 6.4.0 Release 6 1 12)
 * Copyright 2005-2010, Anders Broman <anders.broman@ericsson.com>
 * Updated to 3GPP TS 29.078 version 7.3.0 Release 7 (2006-06)
 * Built from the gsm-map dissector Copyright 2004, Anders Broman <anders.broman@ericsson.com>
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 * References: ETSI 300 374
 */
/*
 * Indentation logic: this file is indented with 2 spaces indentation.
 *                    there are no tabs.
 */
#include "config.h"

#include <stdlib.h>

#include <epan/packet.h>
#include <epan/prefs.h>
#include <epan/oids.h>
#include <epan/tap.h>
#include <epan/srt_table.h>
#include <epan/stat_tap_ui.h>
#include <epan/asn1.h>
#include <epan/expert.h>
#include <epan/tfs.h>
#include <wsutil/strtoi.h>
#include <wsutil/array.h>

#include "packet-ber.h"
#include "packet-camel.h"
#include "packet-q931.h"
#include "packet-e164.h"
#include "packet-isup.h"
#include "packet-gsm_map.h"
#include "packet-gsm_a_common.h"
#include "packet-inap.h"
#include "packet-tcap.h"

#define PNAME  "Camel"
#define PSNAME "CAMEL"
#define PFNAME "camel"

/* Initialize the protocol and registered fields */
static int proto_camel;
static int date_format = 1; /*assume european date format */
static int camel_tap;
/* Global variables */
static uint32_t opcode=0;
static uint32_t errorCode=0;
static uint32_t camel_ver;

/* When several Camel components are received in a single TCAP message,
   we have to use several buffers for the stored parameters
   because else this data are erased during TAP dissector call */
#define MAX_CAMEL_INSTANCE 10
static int camelsrt_global_current=0;
static struct camelsrt_info_t camelsrt_global_info[MAX_CAMEL_INSTANCE];

/* ROSE context */
static rose_ctx_t camel_rose_ctx;

static int hf_digit;
static int hf_camel_extension_code_local;
static int hf_camel_error_code_local;
static int hf_camel_cause_indicator;
static int hf_camel_PDPTypeNumber_etsi;
static int hf_camel_PDPTypeNumber_ietf;
static int hf_camel_PDPAddress_IPv4;
static int hf_camel_PDPAddress_IPv6;
static int hf_camel_cellGlobalIdOrServiceAreaIdFixedLength;
static int hf_camel_RP_Cause;
static int hf_camel_CAMEL_AChBillingChargingCharacteristics;
static int hf_camel_CAMEL_FCIBillingChargingCharacteristics;
static int hf_camel_CAMEL_FCIGPRSBillingChargingCharacteristics;
static int hf_camel_CAMEL_FCISMSBillingChargingCharacteristics;
static int hf_camel_CAMEL_SCIBillingChargingCharacteristics;
static int hf_camel_CAMEL_SCIGPRSBillingChargingCharacteristics;
static int hf_camel_CAMEL_CallResult;

/* Used by persistent data */
static int hf_camelsrt_SessionId;
//static int hf_camelsrt_RequestNumber;
static int hf_camelsrt_Duplicate;
static int hf_camelsrt_RequestFrame;
static int hf_camelsrt_ResponseFrame;
//static int hf_camelsrt_DeltaTime;
//static int hf_camelsrt_SessionTime;
static int hf_camelsrt_DeltaTime31;
static int hf_camelsrt_DeltaTime75;
static int hf_camelsrt_DeltaTime65;
static int hf_camelsrt_DeltaTime22;
static int hf_camelsrt_DeltaTime35;
static int hf_camelsrt_DeltaTime80;
static int hf_camel_timeandtimezone_time;
static int hf_camel_timeandtimezone_tz;

static int hf_camel_PAR_cancelFailed_PDU;         /* PAR_cancelFailed */
static int hf_camel_PAR_requestedInfoError_PDU;   /* PAR_requestedInfoError */
static int hf_camel_UnavailableNetworkResource_PDU;  /* UnavailableNetworkResource */
static int hf_camel_PAR_taskRefused_PDU;          /* PAR_taskRefused */
static int hf_camel_CAP_GPRS_ReferenceNumber_PDU;  /* CAP_GPRS_ReferenceNumber */
static int hf_camel_PlayAnnouncementArg_PDU;      /* PlayAnnouncementArg */
static int hf_camel_PromptAndCollectUserInformationArg_PDU;  /* PromptAndCollectUserInformationArg */
static int hf_camel_ReceivedInformationArg_PDU;   /* ReceivedInformationArg */
static int hf_camel_SpecializedResourceReportArg_PDU;  /* SpecializedResourceReportArg */
static int hf_camel_ApplyChargingArg_PDU;         /* ApplyChargingArg */
static int hf_camel_ApplyChargingReportArg_PDU;   /* ApplyChargingReportArg */
static int hf_camel_AssistRequestInstructionsArg_PDU;  /* AssistRequestInstructionsArg */
static int hf_camel_CallGapArg_PDU;               /* CallGapArg */
static int hf_camel_CallInformationReportArg_PDU;  /* CallInformationReportArg */
static int hf_camel_CallInformationRequestArg_PDU;  /* CallInformationRequestArg */
static int hf_camel_CancelArg_PDU;                /* CancelArg */
static int hf_camel_CollectInformationArg_PDU;    /* CollectInformationArg */
static int hf_camel_ConnectArg_PDU;               /* ConnectArg */
static int hf_camel_ConnectToResourceArg_PDU;     /* ConnectToResourceArg */
static int hf_camel_ContinueWithArgumentArg_PDU;  /* ContinueWithArgumentArg */
static int hf_camel_DisconnectForwardConnectionWithArgumentArg_PDU;  /* DisconnectForwardConnectionWithArgumentArg */
static int hf_camel_DisconnectLegArg_PDU;         /* DisconnectLegArg */
static int hf_camel_EntityReleasedArg_PDU;        /* EntityReleasedArg */
static int hf_camel_EstablishTemporaryConnectionArg_PDU;  /* EstablishTemporaryConnectionArg */
static int hf_camel_EventReportBCSMArg_PDU;       /* EventReportBCSMArg */
static int hf_camel_FurnishChargingInformationArg_PDU;  /* FurnishChargingInformationArg */
static int hf_camel_InitialDPArg_PDU;             /* InitialDPArg */
static int hf_camel_InitiateCallAttemptArg_PDU;   /* InitiateCallAttemptArg */
static int hf_camel_InitiateCallAttemptRes_PDU;   /* InitiateCallAttemptRes */
static int hf_camel_MoveLegArg_PDU;               /* MoveLegArg */
static int hf_camel_PlayToneArg_PDU;              /* PlayToneArg */
static int hf_camel_ReleaseCallArg_PDU;           /* ReleaseCallArg */
static int hf_camel_RequestReportBCSMEventArg_PDU;  /* RequestReportBCSMEventArg */
static int hf_camel_ResetTimerArg_PDU;            /* ResetTimerArg */
static int hf_camel_SendChargingInformationArg_PDU;  /* SendChargingInformationArg */
static int hf_camel_SplitLegArg_PDU;              /* SplitLegArg */
static int hf_camel_ApplyChargingGPRSArg_PDU;     /* ApplyChargingGPRSArg */
static int hf_camel_ApplyChargingReportGPRSArg_PDU;  /* ApplyChargingReportGPRSArg */
static int hf_camel_CancelGPRSArg_PDU;            /* CancelGPRSArg */
static int hf_camel_ConnectGPRSArg_PDU;           /* ConnectGPRSArg */
static int hf_camel_ContinueGPRSArg_PDU;          /* ContinueGPRSArg */
static int hf_camel_EntityReleasedGPRSArg_PDU;    /* EntityReleasedGPRSArg */
static int hf_camel_EventReportGPRSArg_PDU;       /* EventReportGPRSArg */
static int hf_camel_FurnishChargingInformationGPRSArg_PDU;  /* FurnishChargingInformationGPRSArg */
static int hf_camel_InitialDPGPRSArg_PDU;         /* InitialDPGPRSArg */
static int hf_camel_ReleaseGPRSArg_PDU;           /* ReleaseGPRSArg */
static int hf_camel_RequestReportGPRSEventArg_PDU;  /* RequestReportGPRSEventArg */
static int hf_camel_ResetTimerGPRSArg_PDU;        /* ResetTimerGPRSArg */
static int hf_camel_SendChargingInformationGPRSArg_PDU;  /* SendChargingInformationGPRSArg */
static int hf_camel_ConnectSMSArg_PDU;            /* ConnectSMSArg */
static int hf_camel_EventReportSMSArg_PDU;        /* EventReportSMSArg */
static int hf_camel_FurnishChargingInformationSMSArg_PDU;  /* FurnishChargingInformationSMSArg */
static int hf_camel_InitialDPSMSArg_PDU;          /* InitialDPSMSArg */
static int hf_camel_ReleaseSMSArg_PDU;            /* ReleaseSMSArg */
static int hf_camel_RequestReportSMSEventArg_PDU;  /* RequestReportSMSEventArg */
static int hf_camel_ResetTimerSMSArg_PDU;         /* ResetTimerSMSArg */
static int hf_camel_CAP_U_ABORT_REASON_PDU;       /* CAP_U_ABORT_REASON */
static int hf_camel_legID;                        /* LegID */
static int hf_camel_srfConnection;                /* CallSegmentID */
static int hf_camel_aOCInitial;                   /* CAI_GSM0224 */
static int hf_camel_aOCSubsequent;                /* AOCSubsequent */
static int hf_camel_cAI_GSM0224;                  /* CAI_GSM0224 */
static int hf_camel_aocSubsequent_tariffSwitchInterval;  /* INTEGER_1_86400 */
static int hf_camel_audibleIndicatorTone;         /* BOOLEAN */
static int hf_camel_burstList;                    /* BurstList */
static int hf_camel_conferenceTreatmentIndicator;  /* OCTET_STRING_SIZE_1 */
static int hf_camel_callCompletionTreatmentIndicator;  /* OCTET_STRING_SIZE_1 */
static int hf_camel_calledAddressValue;           /* Digits */
static int hf_camel_gapOnService;                 /* GapOnService */
static int hf_camel_calledAddressAndService;      /* T_calledAddressAndService */
static int hf_camel_serviceKey;                   /* ServiceKey */
static int hf_camel_callingAddressAndService;     /* T_callingAddressAndService */
static int hf_camel_callingAddressValue;          /* Digits */
static int hf_camel_eventTypeBCSM;                /* EventTypeBCSM */
static int hf_camel_monitorMode;                  /* MonitorMode */
static int hf_camel_dpSpecificCriteria;           /* DpSpecificCriteria */
static int hf_camel_automaticRearm;               /* NULL */
static int hf_camel_cause;                        /* Cause */
static int hf_camel_bearerCap;                    /* T_bearerCap */
static int hf_camel_numberOfBursts;               /* INTEGER_1_3 */
static int hf_camel_burstInterval;                /* INTEGER_1_1200 */
static int hf_camel_numberOfTonesInBurst;         /* INTEGER_1_3 */
static int hf_camel_burstToneDuration;            /* INTEGER_1_20 */
static int hf_camel_toneInterval;                 /* INTEGER_1_20 */
static int hf_camel_warningPeriod;                /* INTEGER_1_1200 */
static int hf_camel_bursts;                       /* Burst */
static int hf_camel_e1;                           /* INTEGER_0_8191 */
static int hf_camel_e2;                           /* INTEGER_0_8191 */
static int hf_camel_e3;                           /* INTEGER_0_8191 */
static int hf_camel_e4;                           /* INTEGER_0_8191 */
static int hf_camel_e5;                           /* INTEGER_0_8191 */
static int hf_camel_e6;                           /* INTEGER_0_8191 */
static int hf_camel_e7;                           /* INTEGER_0_8191 */
static int hf_camel_callSegmentID;                /* CallSegmentID */
static int hf_camel_invokeID;                     /* InvokeID */
static int hf_camel_timeDurationCharging;         /* T_timeDurationCharging */
static int hf_camel_maxCallPeriodDuration;        /* INTEGER_1_864000 */
static int hf_camel_releaseIfdurationExceeded;    /* BOOLEAN */
static int hf_camel_timeDurationCharging_tariffSwitchInterval;  /* INTEGER_1_86400 */
static int hf_camel_audibleIndicator;             /* T_audibleIndicator */
static int hf_camel_extensions;                   /* Extensions */
static int hf_camel_timeDurationChargingResult;   /* T_timeDurationChargingResult */
static int hf_camel_timeDurationChargingResultpartyToCharge;  /* ReceivingSideID */
static int hf_camel_timeInformation;              /* TimeInformation */
static int hf_camel_legActive;                    /* BOOLEAN */
static int hf_camel_callLegReleasedAtTcpExpiry;   /* NULL */
static int hf_camel_aChChargingAddress;           /* AChChargingAddress */
static int hf_camel_fci_fCIBCCCAMELsequence1;     /* T_fci_fCIBCCCAMELsequence1 */
static int hf_camel_freeFormatData;               /* OCTET_STRING_SIZE_bound__minFCIBillingChargingDataLength_bound__maxFCIBillingChargingDataLength */
static int hf_camel_fCIBCCCAMELsequence1partyToCharge;  /* SendingSideID */
static int hf_camel_appendFreeFormatData;         /* AppendFreeFormatData */
static int hf_camel_fciGPRS_fCIBCCCAMELsequence1;  /* T_fciGPRS_fCIBCCCAMELsequence1 */
static int hf_camel_pDPID;                        /* PDPID */
static int hf_camel_fciSMS_fCIBCCCAMELsequence1;  /* T_fciSMS_fCIBCCCAMELsequence1 */
static int hf_camel_aOCBeforeAnswer;              /* AOCBeforeAnswer */
static int hf_camel_aOCAfterAnswer;               /* AOCSubsequent */
static int hf_camel_aOC_extension;                /* CAMEL_SCIBillingChargingCharacteristicsAlt */
static int hf_camel_aOCGPRS;                      /* AOCGPRS */
static int hf_camel_ChangeOfPositionControlInfo_item;  /* ChangeOfLocation */
static int hf_camel_cellGlobalId;                 /* CellGlobalIdOrServiceAreaIdFixedLength */
static int hf_camel_serviceAreaId;                /* CellGlobalIdOrServiceAreaIdFixedLength */
static int hf_camel_locationAreaId;               /* LAIFixedLength */
static int hf_camel_inter_SystemHandOver;         /* NULL */
static int hf_camel_inter_PLMNHandOver;           /* NULL */
static int hf_camel_inter_MSCHandOver;            /* NULL */
static int hf_camel_changeOfLocationAlt;          /* ChangeOfLocationAlt */
static int hf_camel_maxTransferredVolume;         /* INTEGER_1_4294967295 */
static int hf_camel_maxElapsedTime;               /* INTEGER_1_86400 */
static int hf_camel_transferredVolume;            /* TransferredVolume */
static int hf_camel_elapsedTime;                  /* ElapsedTime */
static int hf_camel_transferredVolumeRollOver;    /* TransferredVolumeRollOver */
static int hf_camel_elapsedTimeRollOver;          /* ElapsedTimeRollOver */
static int hf_camel_minimumNbOfDigits;            /* INTEGER_1_30 */
static int hf_camel_maximumNbOfDigits;            /* INTEGER_1_30 */
static int hf_camel_endOfReplyDigit;              /* OCTET_STRING_SIZE_1_2 */
static int hf_camel_cancelDigit;                  /* OCTET_STRING_SIZE_1_2 */
static int hf_camel_startDigit;                   /* OCTET_STRING_SIZE_1_2 */
static int hf_camel_firstDigitTimeOut;            /* INTEGER_1_127 */
static int hf_camel_interDigitTimeOut;            /* INTEGER_1_127 */
static int hf_camel_errorTreatment;               /* ErrorTreatment */
static int hf_camel_interruptableAnnInd;          /* BOOLEAN */
static int hf_camel_voiceInformation;             /* BOOLEAN */
static int hf_camel_voiceBack;                    /* BOOLEAN */
static int hf_camel_collectedDigits;              /* CollectedDigits */
static int hf_camel_basicGapCriteria;             /* BasicGapCriteria */
static int hf_camel_scfID;                        /* ScfID */
static int hf_camel_DestinationRoutingAddress_item;  /* CalledPartyNumber */
static int hf_camel_applicationTimer;             /* ApplicationTimer */
static int hf_camel_midCallControlInfo;           /* MidCallControlInfo */
static int hf_camel_dpSpecificCriteriaAlt;        /* DpSpecificCriteriaAlt */
static int hf_camel_changeOfPositionControlInfo;  /* ChangeOfPositionControlInfo */
static int hf_camel_numberOfDigits;               /* NumberOfDigits */
static int hf_camel_interDigitTimeout;            /* INTEGER_1_127 */
static int hf_camel_oServiceChangeSpecificInfo;   /* T_oServiceChangeSpecificInfo */
static int hf_camel_ext_basicServiceCode;         /* Ext_BasicServiceCode */
static int hf_camel_initiatorOfServiceChange;     /* InitiatorOfServiceChange */
static int hf_camel_natureOfServiceChange;        /* NatureOfServiceChange */
static int hf_camel_tServiceChangeSpecificInfo;   /* T_tServiceChangeSpecificInfo */
static int hf_camel_collectedInfoSpecificInfo;    /* T_collectedInfoSpecificInfo */
static int hf_camel_calledPartyNumber;            /* CalledPartyNumber */
static int hf_camel_timeGPRSIfNoTariffSwitch;     /* INTEGER_0_86400 */
static int hf_camel_timeGPRSIfTariffSwitch;       /* T_timeGPRSIfTariffSwitch */
static int hf_camel_timeGPRSSinceLastTariffSwitch;  /* INTEGER_0_86400 */
static int hf_camel_timeGPRSTariffSwitchInterval;  /* INTEGER_0_86400 */
static int hf_camel_rO_TimeGPRSIfNoTariffSwitch;  /* INTEGER_0_255 */
static int hf_camel_rO_TimeGPRSIfTariffSwitch;    /* T_rO_TimeGPRSIfTariffSwitch */
static int hf_camel_rO_TimeGPRSSinceLastTariffSwitch;  /* INTEGER_0_255 */
static int hf_camel_rO_TimeGPRSTariffSwitchInterval;  /* INTEGER_0_255 */
static int hf_camel_pDPTypeOrganization;          /* T_pDPTypeOrganization */
static int hf_camel_pDPTypeNumber;                /* T_pDPTypeNumber */
static int hf_camel_pDPAddress;                   /* T_pDPAddress */
static int hf_camel_routeSelectFailureSpecificInfo;  /* T_routeSelectFailureSpecificInfo */
static int hf_camel_routeSelectfailureCause;      /* Cause */
static int hf_camel_oCalledPartyBusySpecificInfo;  /* T_oCalledPartyBusySpecificInfo */
static int hf_camel_busyCause;                    /* Cause */
static int hf_camel_oNoAnswerSpecificInfo;        /* T_oNoAnswerSpecificInfo */
static int hf_camel_oAnswerSpecificInfo;          /* T_oAnswerSpecificInfo */
static int hf_camel_destinationAddress;           /* CalledPartyNumber */
static int hf_camel_or_Call;                      /* NULL */
static int hf_camel_forwardedCall;                /* NULL */
static int hf_camel_chargeIndicator;              /* ChargeIndicator */
static int hf_camel_ext_basicServiceCode2;        /* Ext_BasicServiceCode */
static int hf_camel_oMidCallSpecificInfo;         /* T_oMidCallSpecificInfo */
static int hf_camel_omidCallEvents;               /* T_omidCallEvents */
static int hf_camel_dTMFDigitsCompleted;          /* Digits */
static int hf_camel_dTMFDigitsTimeOut;            /* Digits */
static int hf_camel_oDisconnectSpecificInfo;      /* T_oDisconnectSpecificInfo */
static int hf_camel_releaseCause;                 /* Cause */
static int hf_camel_tBusySpecificInfo;            /* T_tBusySpecificInfo */
static int hf_camel_callForwarded;                /* NULL */
static int hf_camel_routeNotPermitted;            /* NULL */
static int hf_camel_forwardingDestinationNumber;  /* CalledPartyNumber */
static int hf_camel_tNoAnswerSpecificInfo;        /* T_tNoAnswerSpecificInfo */
static int hf_camel_tAnswerSpecificInfo;          /* T_tAnswerSpecificInfo */
static int hf_camel_tMidCallSpecificInfo;         /* T_tMidCallSpecificInfo */
static int hf_camel_tmidCallEvents;               /* T_tmidCallEvents */
static int hf_camel_tDisconnectSpecificInfo;      /* T_tDisconnectSpecificInfo */
static int hf_camel_oTermSeizedSpecificInfo;      /* T_oTermSeizedSpecificInfo */
static int hf_camel_locationInformation;          /* LocationInformation */
static int hf_camel_callAcceptedSpecificInfo;     /* T_callAcceptedSpecificInfo */
static int hf_camel_oAbandonSpecificInfo;         /* T_oAbandonSpecificInfo */
static int hf_camel_oChangeOfPositionSpecificInfo;  /* T_oChangeOfPositionSpecificInfo */
static int hf_camel_metDPCriteriaList;            /* MetDPCriteriaList */
static int hf_camel_tChangeOfPositionSpecificInfo;  /* T_tChangeOfPositionSpecificInfo */
static int hf_camel_dpSpecificInfoAlt;            /* DpSpecificInfoAlt */
static int hf_camel_o_smsFailureSpecificInfo;     /* T_o_smsFailureSpecificInfo */
static int hf_camel_mo_smsfailureCause;           /* MO_SMSCause */
static int hf_camel_o_smsSubmissionSpecificInfo;  /* T_o_smsSubmissionSpecificInfo */
static int hf_camel_t_smsFailureSpecificInfo;     /* T_t_smsFailureSpecificInfo */
static int hf_camel_t_smsfailureCause;            /* MT_SMSCause */
static int hf_camel_t_smsDeliverySpecificInfo;    /* T_t_smsDeliverySpecificInfo */
static int hf_camel_Extensions_item;              /* ExtensionField */
static int hf_camel_type;                         /* Code */
static int hf_camel_criticality;                  /* CriticalityType */
static int hf_camel_value;                        /* T_value */
static int hf_camel_callDiversionTreatmentIndicator;  /* OCTET_STRING_SIZE_1 */
static int hf_camel_callingPartyRestrictionIndicator;  /* OCTET_STRING_SIZE_1 */
static int hf_camel_compoundGapCriteria;          /* CompoundCriteria */
static int hf_camel_gapIndicatorsDuration;        /* Duration */
static int hf_camel_gapInterval;                  /* Interval */
static int hf_camel_informationToSend;            /* InformationToSend */
static int hf_camel_GenericNumbers_item;          /* GenericNumber */
static int hf_camel_short_QoS_format;             /* QoS_Subscribed */
static int hf_camel_long_QoS_format;              /* Ext_QoS_Subscribed */
static int hf_camel_supplement_to_long_QoS_format;  /* Ext2_QoS_Subscribed */
static int hf_camel_additionalSupplement;         /* Ext3_QoS_Subscribed */
static int hf_camel_gPRSEventType;                /* GPRSEventType */
static int hf_camel_attachChangeOfPositionSpecificInformation;  /* T_attachChangeOfPositionSpecificInformation */
static int hf_camel_locationInformationGPRS;      /* LocationInformationGPRS */
static int hf_camel_pdp_ContextchangeOfPositionSpecificInformation;  /* T_pdp_ContextchangeOfPositionSpecificInformation */
static int hf_camel_accessPointName;              /* AccessPointName */
static int hf_camel_chargingID;                   /* GPRSChargingID */
static int hf_camel_endUserAddress;               /* EndUserAddress */
static int hf_camel_qualityOfService;             /* QualityOfService */
static int hf_camel_timeAndTimeZone;              /* TimeAndTimezone */
static int hf_camel_gGSNAddress;                  /* GSN_Address */
static int hf_camel_detachSpecificInformation;    /* T_detachSpecificInformation */
static int hf_camel_initiatingEntity;             /* InitiatingEntity */
static int hf_camel_routeingAreaUpdate;           /* NULL */
static int hf_camel_disconnectSpecificInformation;  /* T_disconnectSpecificInformation */
static int hf_camel_pDPContextEstablishmentSpecificInformation;  /* T_pDPContextEstablishmentSpecificInformation */
static int hf_camel_pDPInitiationType;            /* PDPInitiationType */
static int hf_camel_secondaryPDP_context;         /* NULL */
static int hf_camel_pDPContextEstablishmentAcknowledgementSpecificInformation;  /* T_pDPContextEstablishmentAcknowledgementSpecificInformation */
static int hf_camel_messageID;                    /* MessageID */
static int hf_camel_numberOfRepetitions;          /* INTEGER_1_127 */
static int hf_camel_inbandInfoDuration;           /* INTEGER_0_32767 */
static int hf_camel_interval;                     /* INTEGER_0_32767 */
static int hf_camel_inbandInfo;                   /* InbandInfo */
static int hf_camel_tone;                         /* Tone */
static int hf_camel_cellGlobalIdOrServiceAreaIdOrLAI;  /* T_cellGlobalIdOrServiceAreaIdOrLAI */
static int hf_camel_routeingAreaIdentity;         /* RAIdentity */
static int hf_camel_geographicalInformation;      /* GeographicalInformation */
static int hf_camel_sgsn_Number;                  /* ISDN_AddressString */
static int hf_camel_selectedLSAIdentity;          /* LSAIdentity */
static int hf_camel_extensionContainer;           /* ExtensionContainer */
static int hf_camel_sai_Present;                  /* NULL */
static int hf_camel_userCSGInformation;           /* UserCSGInformation */
static int hf_camel_elementaryMessageID;          /* Integer4 */
static int hf_camel_text;                         /* T_text */
static int hf_camel_messageContent;               /* IA5String_SIZE_bound__minMessageContentLength_bound__maxMessageContentLength */
static int hf_camel_attributes;                   /* OCTET_STRING_SIZE_bound__minAttributesLength_bound__maxAttributesLength */
static int hf_camel_elementaryMessageIDs;         /* SEQUENCE_SIZE_1_bound__numOfMessageIDs_OF_Integer4 */
static int hf_camel_elementaryMessageIDs_item;    /* Integer4 */
static int hf_camel_variableMessage;              /* T_variableMessage */
static int hf_camel_variableParts;                /* SEQUENCE_SIZE_1_5_OF_VariablePart */
static int hf_camel_variableParts_item;           /* VariablePart */
static int hf_camel_MetDPCriteriaList_item;       /* MetDPCriterion */
static int hf_camel_enteringCellGlobalId;         /* CellGlobalIdOrServiceAreaIdFixedLength */
static int hf_camel_leavingCellGlobalId;          /* CellGlobalIdOrServiceAreaIdFixedLength */
static int hf_camel_enteringServiceAreaId;        /* CellGlobalIdOrServiceAreaIdFixedLength */
static int hf_camel_leavingServiceAreaId;         /* CellGlobalIdOrServiceAreaIdFixedLength */
static int hf_camel_enteringLocationAreaId;       /* LAIFixedLength */
static int hf_camel_leavingLocationAreaId;        /* LAIFixedLength */
static int hf_camel_inter_SystemHandOverToUMTS;   /* NULL */
static int hf_camel_inter_SystemHandOverToGSM;    /* NULL */
static int hf_camel_metDPCriterionAlt;            /* MetDPCriterionAlt */
static int hf_camel_minimumNumberOfDigits;        /* INTEGER_1_30 */
static int hf_camel_maximumNumberOfDigits;        /* INTEGER_1_30 */
static int hf_camel_requested_QoS;                /* GPRS_QoS */
static int hf_camel_subscribed_QoS;               /* GPRS_QoS */
static int hf_camel_negotiated_QoS;               /* GPRS_QoS */
static int hf_camel_requested_QoS_Extension;      /* GPRS_QoS_Extension */
static int hf_camel_subscribed_QoS_Extension;     /* GPRS_QoS_Extension */
static int hf_camel_negotiated_QoS_Extension;     /* GPRS_QoS_Extension */
static int hf_camel_receivingSideID;              /* LegType */
static int hf_camel_RequestedInformationList_item;  /* RequestedInformation */
static int hf_camel_RequestedInformationTypeList_item;  /* RequestedInformationType */
static int hf_camel_requestedInformationType;     /* RequestedInformationType */
static int hf_camel_requestedInformationValue;    /* RequestedInformationValue */
static int hf_camel_callAttemptElapsedTimeValue;  /* INTEGER_0_255 */
static int hf_camel_callStopTimeValue;            /* DateAndTime */
static int hf_camel_callConnectedElapsedTimeValue;  /* Integer4 */
static int hf_camel_releaseCauseValue;            /* Cause */
static int hf_camel_sendingSideID;                /* LegType */
static int hf_camel_forwardServiceInteractionInd;  /* ForwardServiceInteractionInd */
static int hf_camel_backwardServiceInteractionInd;  /* BackwardServiceInteractionInd */
static int hf_camel_bothwayThroughConnectionInd;  /* BothwayThroughConnectionInd */
static int hf_camel_connectedNumberTreatmentInd;  /* ConnectedNumberTreatmentInd */
static int hf_camel_nonCUGCall;                   /* NULL */
static int hf_camel_holdTreatmentIndicator;       /* OCTET_STRING_SIZE_1 */
static int hf_camel_cwTreatmentIndicator;         /* OCTET_STRING_SIZE_1 */
static int hf_camel_ectTreatmentIndicator;        /* OCTET_STRING_SIZE_1 */
static int hf_camel_eventTypeSMS;                 /* EventTypeSMS */
static int hf_camel_timeSinceTariffSwitch;        /* INTEGER_0_864000 */
static int hf_camel_timeIfTariffSwitch_tariffSwitchInterval;  /* INTEGER_1_864000 */
static int hf_camel_timeIfNoTariffSwitch;         /* TimeIfNoTariffSwitch */
static int hf_camel_timeIfTariffSwitch;           /* TimeIfTariffSwitch */
static int hf_camel_toneID;                       /* Integer4 */
static int hf_camel_toneDuration;                 /* Integer4 */
static int hf_camel_volumeIfNoTariffSwitch;       /* INTEGER_0_4294967295 */
static int hf_camel_volumeIfTariffSwitch;         /* T_volumeIfTariffSwitch */
static int hf_camel_volumeSinceLastTariffSwitch;  /* INTEGER_0_4294967295 */
static int hf_camel_volumeTariffSwitchInterval;   /* INTEGER_0_4294967295 */
static int hf_camel_rO_VolumeIfNoTariffSwitch;    /* INTEGER_0_255 */
static int hf_camel_rO_VolumeIfTariffSwitch;      /* T_rO_VolumeIfTariffSwitch */
static int hf_camel_rO_VolumeSinceLastTariffSwitch;  /* INTEGER_0_255 */
static int hf_camel_rO_VolumeTariffSwitchInterval;  /* INTEGER_0_255 */
static int hf_camel_integer;                      /* Integer4 */
static int hf_camel_number;                       /* Digits */
static int hf_camel_time;                         /* OCTET_STRING_SIZE_2 */
static int hf_camel_date;                         /* OCTET_STRING_SIZE_4 */
static int hf_camel_price;                        /* OCTET_STRING_SIZE_4 */
static int hf_camel_par_cancelFailedProblem;      /* T_par_cancelFailedProblem */
static int hf_camel_operation;                    /* InvokeID */
static int hf_camel_destinationReference;         /* Integer4 */
static int hf_camel_originationReference;         /* Integer4 */
static int hf_camel_disconnectFromIPForbidden;    /* BOOLEAN */
static int hf_camel_requestAnnouncementCompleteNotification;  /* BOOLEAN */
static int hf_camel_requestAnnouncementStartedNotification;  /* BOOLEAN */
static int hf_camel_collectedInfo;                /* CollectedInfo */
static int hf_camel_digitsResponse;               /* Digits */
static int hf_camel_allAnnouncementsComplete;     /* NULL */
static int hf_camel_firstAnnouncementStarted;     /* NULL */
static int hf_camel_aChBillingChargingCharacteristics;  /* AChBillingChargingCharacteristics */
static int hf_camel_partyToCharge;                /* SendingSideID */
static int hf_camel_iTXcharging;                  /* BOOLEAN */
static int hf_camel_correlationID;                /* CorrelationID */
static int hf_camel_iPSSPCapabilities;            /* IPSSPCapabilities */
static int hf_camel_gapCriteria;                  /* GapCriteria */
static int hf_camel_gapIndicators;                /* GapIndicators */
static int hf_camel_controlType;                  /* ControlType */
static int hf_camel_gapTreatment;                 /* GapTreatment */
static int hf_camel_requestedInformationList;     /* RequestedInformationList */
static int hf_camel_legID_01;                     /* ReceivingSideID */
static int hf_camel_requestedInformationTypeList;  /* RequestedInformationTypeList */
static int hf_camel_legID_02;                     /* SendingSideID */
static int hf_camel_allRequests;                  /* NULL */
static int hf_camel_callSegmentToCancel;          /* CallSegmentToCancel */
static int hf_camel_destinationRoutingAddress;    /* DestinationRoutingAddress */
static int hf_camel_alertingPattern;              /* AlertingPattern */
static int hf_camel_originalCalledPartyID;        /* OriginalCalledPartyID */
static int hf_camel_carrier;                      /* Carrier */
static int hf_camel_callingPartysCategory;        /* CallingPartysCategory */
static int hf_camel_redirectingPartyID;           /* RedirectingPartyID */
static int hf_camel_redirectionInformation;       /* RedirectionInformation */
static int hf_camel_genericNumbers;               /* GenericNumbers */
static int hf_camel_serviceInteractionIndicatorsTwo;  /* ServiceInteractionIndicatorsTwo */
static int hf_camel_chargeNumber;                 /* ChargeNumber */
static int hf_camel_legToBeConnected;             /* LegID */
static int hf_camel_cug_Interlock;                /* CUG_Interlock */
static int hf_camel_cug_OutgoingAccess;           /* NULL */
static int hf_camel_suppressionOfAnnouncement;    /* SuppressionOfAnnouncement */
static int hf_camel_oCSIApplicable;               /* OCSIApplicable */
static int hf_camel_naOliInfo;                    /* NAOliInfo */
static int hf_camel_bor_InterrogationRequested;   /* NULL */
static int hf_camel_suppress_N_CSI;               /* NULL */
static int hf_camel_resourceAddress;              /* T_resourceAddress */
static int hf_camel_ipRoutingAddress;             /* IPRoutingAddress */
static int hf_camel_none;                         /* NULL */
static int hf_camel_suppress_O_CSI;               /* NULL */
static int hf_camel_continueWithArgumentArgExtension;  /* ContinueWithArgumentArgExtension */
static int hf_camel_suppress_D_CSI;               /* NULL */
static int hf_camel_suppressOutgoingCallBarring;  /* NULL */
static int hf_camel_legOrCallSegment;             /* LegOrCallSegment */
static int hf_camel_legToBeReleased;              /* LegID */
static int hf_camel_callSegmentFailure;           /* CallSegmentFailure */
static int hf_camel_bCSM_Failure;                 /* BCSM_Failure */
static int hf_camel_assistingSSPIPRoutingAddress;  /* AssistingSSPIPRoutingAddress */
static int hf_camel_callingPartyNumber;           /* CallingPartyNumber */
static int hf_camel_eventSpecificInformationBCSM;  /* EventSpecificInformationBCSM */
static int hf_camel_miscCallInfo;                 /* MiscCallInfo */
static int hf_camel_cGEncountered;                /* CGEncountered */
static int hf_camel_locationNumber;               /* LocationNumber */
static int hf_camel_highLayerCompatibility;       /* HighLayerCompatibility */
static int hf_camel_additionalCallingPartyNumber;  /* AdditionalCallingPartyNumber */
static int hf_camel_bearerCapability;             /* BearerCapability */
static int hf_camel_cug_Index;                    /* CUG_Index */
static int hf_camel_iMSI;                         /* IMSI */
static int hf_camel_subscriberState;              /* SubscriberState */
static int hf_camel_callReferenceNumber;          /* CallReferenceNumber */
static int hf_camel_mscAddress;                   /* ISDN_AddressString */
static int hf_camel_calledPartyBCDNumber;         /* CalledPartyBCDNumber */
static int hf_camel_timeAndTimezone;              /* TimeAndTimezone */
static int hf_camel_callForwardingSS_Pending;     /* NULL */
static int hf_camel_initialDPArgExtension;        /* InitialDPArgExtension */
static int hf_camel_gmscAddress;                  /* ISDN_AddressString */
static int hf_camel_ms_Classmark2;                /* MS_Classmark2 */
static int hf_camel_iMEI;                         /* IMEI */
static int hf_camel_supportedCamelPhases;         /* SupportedCamelPhases */
static int hf_camel_offeredCamel4Functionalities;  /* OfferedCamel4Functionalities */
static int hf_camel_bearerCapability2;            /* BearerCapability */
static int hf_camel_highLayerCompatibility2;      /* HighLayerCompatibility */
static int hf_camel_lowLayerCompatibility;        /* LowLayerCompatibility */
static int hf_camel_lowLayerCompatibility2;       /* LowLayerCompatibility */
static int hf_camel_enhancedDialledServicesAllowed;  /* NULL */
static int hf_camel_uu_Data;                      /* UU_Data */
static int hf_camel_collectInformationAllowed;    /* NULL */
static int hf_camel_releaseCallArgExtensionAllowed;  /* NULL */
static int hf_camel_legToBeCreated;               /* LegID */
static int hf_camel_newCallSegment;               /* CallSegmentID */
static int hf_camel_gsmSCFAddress;                /* ISDN_AddressString */
static int hf_camel_suppress_T_CSI;               /* NULL */
static int hf_camel_legIDToMove;                  /* LegID */
static int hf_camel_allCallSegments;              /* AllCallSegments */
static int hf_camel_allCallSegmentsWithExtension;  /* AllCallSegmentsWithExtension */
static int hf_camel_bcsmEvents;                   /* SEQUENCE_SIZE_1_bound__numOfBCSMEvents_OF_BCSMEvent */
static int hf_camel_bcsmEvents_item;              /* BCSMEvent */
static int hf_camel_timerID;                      /* TimerID */
static int hf_camel_timervalue;                   /* TimerValue */
static int hf_camel_sCIBillingChargingCharacteristics;  /* SCIBillingChargingCharacteristics */
static int hf_camel_legToBeSplit;                 /* LegID */
static int hf_camel_chargingCharacteristics;      /* ChargingCharacteristics */
static int hf_camel_applyChargingGPRS_tariffSwitchInterval;  /* INTEGER_1_86400 */
static int hf_camel_chargingResult;               /* ChargingResult */
static int hf_camel_active;                       /* BOOLEAN */
static int hf_camel_chargingRollOver;             /* ChargingRollOver */
static int hf_camel_pdpID;                        /* PDPID */
static int hf_camel_gPRSCause;                    /* GPRSCause */
static int hf_camel_miscGPRSInfo;                 /* MiscCallInfo */
static int hf_camel_gPRSEventSpecificInformation;  /* GPRSEventSpecificInformation */
static int hf_camel_mSISDN;                       /* ISDN_AddressString */
static int hf_camel_gPRSMSClass;                  /* GPRSMSClass */
static int hf_camel_sGSNCapabilities;             /* SGSNCapabilities */
static int hf_camel_gprsCause;                    /* GPRSCause */
static int hf_camel_gPRSEvent;                    /* SEQUENCE_SIZE_1_bound__numOfGPRSEvents_OF_GPRSEvent */
static int hf_camel_gPRSEvent_item;               /* GPRSEvent */
static int hf_camel_sCIGPRSBillingChargingCharacteristics;  /* SCIGPRSBillingChargingCharacteristics */
static int hf_camel_callingPartysNumber;          /* SMS_AddressString */
static int hf_camel_destinationSubscriberNumber;  /* CalledPartyBCDNumber */
static int hf_camel_sMSCAddress;                  /* ISDN_AddressString */
static int hf_camel_eventSpecificInformationSMS;  /* EventSpecificInformationSMS */
static int hf_camel_callingPartyNumber_01;        /* SMS_AddressString */
static int hf_camel_locationInformationMSC;       /* LocationInformation */
static int hf_camel_tPShortMessageSpecificInfo;   /* TPShortMessageSpecificInfo */
static int hf_camel_tPProtocolIdentifier;         /* TPProtocolIdentifier */
static int hf_camel_tPDataCodingScheme;           /* TPDataCodingScheme */
static int hf_camel_tPValidityPeriod;             /* TPValidityPeriod */
static int hf_camel_smsReferenceNumber;           /* CallReferenceNumber */
static int hf_camel_calledPartyNumber_01;         /* ISDN_AddressString */
static int hf_camel_sMSEvents;                    /* SEQUENCE_SIZE_1_bound__numOfSMSEvents_OF_SMSEvent */
static int hf_camel_sMSEvents_item;               /* SMSEvent */
static int hf_camel_extensions_01;                /* SEQUENCE_SIZE_1_numOfExtensions_OF_ExtensionField */
static int hf_camel_extensions_item;              /* ExtensionField */
static int hf_camel_na_info;                      /* NA_Info */
static int hf_camel_naCarrierInformation;         /* NACarrierInformation */
static int hf_camel_naCarrierId;                  /* NAEA_CIC */
static int hf_camel_naCICSelectionType;           /* NACarrierSelectionInfo */
static int hf_camel_naChargeNumber;               /* NAChargeNumber */
static int hf_camel_timeDurationCharging_01;      /* T_timeDurationCharging_01 */
static int hf_camel_releaseIfdurationExceeded_01;  /* ReleaseIfDurationExceeded */
static int hf_camel_tariffSwitchInterval;         /* INTEGER_1_86400 */
static int hf_camel_tone_01;                      /* BOOLEAN */
static int hf_camel_local;                        /* T_local */
static int hf_camel_global;                       /* T_global */
static int hf_camel_invoke;                       /* Invoke */
static int hf_camel_returnResult;                 /* ReturnResult */
static int hf_camel_returnError;                  /* ReturnError */
static int hf_camel_reject;                       /* Reject */
static int hf_camel_invokeId;                     /* InvokeId */
static int hf_camel_linkedId;                     /* T_linkedId */
static int hf_camel_linkedIdPresent;              /* T_linkedIdPresent */
static int hf_camel_absent;                       /* NULL */
static int hf_camel_opcode;                       /* Code */
static int hf_camel_argument;                     /* T_argument */
static int hf_camel_result;                       /* T_result */
static int hf_camel_resultArgument;               /* ResultArgument */
static int hf_camel_errcode;                      /* Code */
static int hf_camel_parameter;                    /* T_parameter */
static int hf_camel_problem;                      /* T_problem */
static int hf_camel_general;                      /* GeneralProblem */
static int hf_camel_invokeProblem;                /* InvokeProblem */
static int hf_camel_problemReturnResult;          /* ReturnResultProblem */
static int hf_camel_returnErrorProblem;           /* ReturnErrorProblem */
static int hf_camel_present;                      /* INTEGER */
static int hf_camel_InvokeId_present;             /* InvokeId_present */

static struct camelsrt_info_t * gp_camelsrt_info;

/* Forward declarations */
static int dissect_invokeData(proto_tree *tree, tvbuff_t *tvb, int offset,asn1_ctx_t *actx);
static int dissect_returnResultData(proto_tree *tree, tvbuff_t *tvb, int offset,asn1_ctx_t *actx);
static int dissect_returnErrorData(proto_tree *tree, tvbuff_t *tvb, int offset,asn1_ctx_t *actx);
static int dissect_camel_CAMEL_AChBillingChargingCharacteristics(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_);
static int dissect_camel_CAMEL_AChBillingChargingCharacteristicsV2(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_);
static int dissect_camel_CAMEL_CallResult(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_);
static int dissect_camel_EstablishTemporaryConnectionArgV2(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_);
static int dissect_camel_SpecializedResourceReportArgV23(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_);

/* XXX - can we get rid of these and always do the SRT work? */
static bool gcamel_PersistentSRT=false;
static bool gcamel_DisplaySRT=false;
bool gcamel_StatSRT=false;

/* Initialize the subtree pointers */
static int ett_camel;
static int ett_camelisup_parameter;
static int ett_camel_AccessPointName;
static int ett_camel_pdptypenumber;
static int ett_camel_cause;
static int ett_camel_RPcause;
static int ett_camel_stat;
static int ett_camel_calledpartybcdnumber;
static int ett_camel_callingpartynumber;
static int ett_camel_originalcalledpartyid;
static int ett_camel_redirectingpartyid;
static int ett_camel_locationnumber;
static int ett_camel_additionalcallingpartynumber;
static int ett_camel_calledAddressValue;
static int ett_camel_callingAddressValue;
static int ett_camel_assistingSSPIPRoutingAddress;
static int ett_camel_correlationID;
static int ett_camel_dTMFDigitsCompleted;
static int ett_camel_dTMFDigitsTimeOut;
static int ett_camel_number;
static int ett_camel_digitsResponse;
static int ett_camel_timeandtimezone;

static int ett_camel_AChChargingAddress;
static int ett_camel_AOCBeforeAnswer;
static int ett_camel_AOCGPRS;
static int ett_camel_AOCSubsequent;
static int ett_camel_AudibleIndicator;
static int ett_camel_BackwardServiceInteractionInd;
static int ett_camel_BasicGapCriteria;
static int ett_camel_T_calledAddressAndService;
static int ett_camel_T_callingAddressAndService;
static int ett_camel_BCSMEvent;
static int ett_camel_BCSM_Failure;
static int ett_camel_BearerCapability;
static int ett_camel_Burst;
static int ett_camel_BurstList;
static int ett_camel_CAI_GSM0224;
static int ett_camel_CallSegmentFailure;
static int ett_camel_CallSegmentToCancel;
static int ett_camel_CAMEL_AChBillingChargingCharacteristics;
static int ett_camel_T_timeDurationCharging;
static int ett_camel_CAMEL_CallResult;
static int ett_camel_T_timeDurationChargingResult;
static int ett_camel_CAMEL_FCIBillingChargingCharacteristics;
static int ett_camel_T_fci_fCIBCCCAMELsequence1;
static int ett_camel_CAMEL_FCIGPRSBillingChargingCharacteristics;
static int ett_camel_T_fciGPRS_fCIBCCCAMELsequence1;
static int ett_camel_CAMEL_FCISMSBillingChargingCharacteristics;
static int ett_camel_T_fciSMS_fCIBCCCAMELsequence1;
static int ett_camel_CAMEL_SCIBillingChargingCharacteristics;
static int ett_camel_CAMEL_SCIBillingChargingCharacteristicsAlt;
static int ett_camel_CAMEL_SCIGPRSBillingChargingCharacteristics;
static int ett_camel_ChangeOfPositionControlInfo;
static int ett_camel_ChangeOfLocation;
static int ett_camel_ChangeOfLocationAlt;
static int ett_camel_ChargingCharacteristics;
static int ett_camel_ChargingResult;
static int ett_camel_ChargingRollOver;
static int ett_camel_CollectedDigits;
static int ett_camel_CollectedInfo;
static int ett_camel_CompoundCriteria;
static int ett_camel_DestinationRoutingAddress;
static int ett_camel_DpSpecificCriteria;
static int ett_camel_DpSpecificCriteriaAlt;
static int ett_camel_DpSpecificInfoAlt;
static int ett_camel_T_oServiceChangeSpecificInfo;
static int ett_camel_T_tServiceChangeSpecificInfo;
static int ett_camel_T_collectedInfoSpecificInfo;
static int ett_camel_ElapsedTime;
static int ett_camel_T_timeGPRSIfTariffSwitch;
static int ett_camel_ElapsedTimeRollOver;
static int ett_camel_T_rO_TimeGPRSIfTariffSwitch;
static int ett_camel_EndUserAddress;
static int ett_camel_EventSpecificInformationBCSM;
static int ett_camel_T_routeSelectFailureSpecificInfo;
static int ett_camel_T_oCalledPartyBusySpecificInfo;
static int ett_camel_T_oNoAnswerSpecificInfo;
static int ett_camel_T_oAnswerSpecificInfo;
static int ett_camel_T_oMidCallSpecificInfo;
static int ett_camel_T_omidCallEvents;
static int ett_camel_T_oDisconnectSpecificInfo;
static int ett_camel_T_tBusySpecificInfo;
static int ett_camel_T_tNoAnswerSpecificInfo;
static int ett_camel_T_tAnswerSpecificInfo;
static int ett_camel_T_tMidCallSpecificInfo;
static int ett_camel_T_tmidCallEvents;
static int ett_camel_T_tDisconnectSpecificInfo;
static int ett_camel_T_oTermSeizedSpecificInfo;
static int ett_camel_T_callAcceptedSpecificInfo;
static int ett_camel_T_oAbandonSpecificInfo;
static int ett_camel_T_oChangeOfPositionSpecificInfo;
static int ett_camel_T_tChangeOfPositionSpecificInfo;
static int ett_camel_EventSpecificInformationSMS;
static int ett_camel_T_o_smsFailureSpecificInfo;
static int ett_camel_T_o_smsSubmissionSpecificInfo;
static int ett_camel_T_t_smsFailureSpecificInfo;
static int ett_camel_T_t_smsDeliverySpecificInfo;
static int ett_camel_Extensions;
static int ett_camel_ExtensionField;
static int ett_camel_ForwardServiceInteractionInd;
static int ett_camel_GapCriteria;
static int ett_camel_GapIndicators;
static int ett_camel_GapOnService;
static int ett_camel_GapTreatment;
static int ett_camel_GenericNumbers;
static int ett_camel_GPRS_QoS;
static int ett_camel_GPRS_QoS_Extension;
static int ett_camel_GPRSEvent;
static int ett_camel_GPRSEventSpecificInformation;
static int ett_camel_T_attachChangeOfPositionSpecificInformation;
static int ett_camel_T_pdp_ContextchangeOfPositionSpecificInformation;
static int ett_camel_T_detachSpecificInformation;
static int ett_camel_T_disconnectSpecificInformation;
static int ett_camel_T_pDPContextEstablishmentSpecificInformation;
static int ett_camel_T_pDPContextEstablishmentAcknowledgementSpecificInformation;
static int ett_camel_InbandInfo;
static int ett_camel_InformationToSend;
static int ett_camel_LegOrCallSegment;
static int ett_camel_LocationInformationGPRS;
static int ett_camel_MessageID;
static int ett_camel_T_text;
static int ett_camel_SEQUENCE_SIZE_1_bound__numOfMessageIDs_OF_Integer4;
static int ett_camel_T_variableMessage;
static int ett_camel_SEQUENCE_SIZE_1_5_OF_VariablePart;
static int ett_camel_MetDPCriteriaList;
static int ett_camel_MetDPCriterion;
static int ett_camel_MetDPCriterionAlt;
static int ett_camel_MidCallControlInfo;
static int ett_camel_QualityOfService;
static int ett_camel_ReceivingSideID;
static int ett_camel_RequestedInformationList;
static int ett_camel_RequestedInformationTypeList;
static int ett_camel_RequestedInformation;
static int ett_camel_RequestedInformationValue;
static int ett_camel_SendingSideID;
static int ett_camel_ServiceInteractionIndicatorsTwo;
static int ett_camel_SMSEvent;
static int ett_camel_TimeIfTariffSwitch;
static int ett_camel_TimeInformation;
static int ett_camel_Tone;
static int ett_camel_TransferredVolume;
static int ett_camel_T_volumeIfTariffSwitch;
static int ett_camel_TransferredVolumeRollOver;
static int ett_camel_T_rO_VolumeIfTariffSwitch;
static int ett_camel_VariablePart;
static int ett_camel_PAR_cancelFailed;
static int ett_camel_CAP_GPRS_ReferenceNumber;
static int ett_camel_PlayAnnouncementArg;
static int ett_camel_PromptAndCollectUserInformationArg;
static int ett_camel_ReceivedInformationArg;
static int ett_camel_SpecializedResourceReportArg;
static int ett_camel_ApplyChargingArg;
static int ett_camel_AssistRequestInstructionsArg;
static int ett_camel_CallGapArg;
static int ett_camel_CallInformationReportArg;
static int ett_camel_CallInformationRequestArg;
static int ett_camel_CancelArg;
static int ett_camel_CollectInformationArg;
static int ett_camel_ConnectArg;
static int ett_camel_ConnectToResourceArg;
static int ett_camel_T_resourceAddress;
static int ett_camel_ContinueWithArgumentArg;
static int ett_camel_ContinueWithArgumentArgExtension;
static int ett_camel_DisconnectForwardConnectionWithArgumentArg;
static int ett_camel_DisconnectLegArg;
static int ett_camel_EntityReleasedArg;
static int ett_camel_EstablishTemporaryConnectionArg;
static int ett_camel_EventReportBCSMArg;
static int ett_camel_InitialDPArg;
static int ett_camel_InitialDPArgExtension;
static int ett_camel_InitiateCallAttemptArg;
static int ett_camel_InitiateCallAttemptRes;
static int ett_camel_MoveLegArg;
static int ett_camel_PlayToneArg;
static int ett_camel_ReleaseCallArg;
static int ett_camel_AllCallSegmentsWithExtension;
static int ett_camel_RequestReportBCSMEventArg;
static int ett_camel_SEQUENCE_SIZE_1_bound__numOfBCSMEvents_OF_BCSMEvent;
static int ett_camel_ResetTimerArg;
static int ett_camel_SendChargingInformationArg;
static int ett_camel_SplitLegArg;
static int ett_camel_ApplyChargingGPRSArg;
static int ett_camel_ApplyChargingReportGPRSArg;
static int ett_camel_CancelGPRSArg;
static int ett_camel_ConnectGPRSArg;
static int ett_camel_ContinueGPRSArg;
static int ett_camel_EntityReleasedGPRSArg;
static int ett_camel_EventReportGPRSArg;
static int ett_camel_InitialDPGPRSArg;
static int ett_camel_ReleaseGPRSArg;
static int ett_camel_RequestReportGPRSEventArg;
static int ett_camel_SEQUENCE_SIZE_1_bound__numOfGPRSEvents_OF_GPRSEvent;
static int ett_camel_ResetTimerGPRSArg;
static int ett_camel_SendChargingInformationGPRSArg;
static int ett_camel_ConnectSMSArg;
static int ett_camel_EventReportSMSArg;
static int ett_camel_InitialDPSMSArg;
static int ett_camel_RequestReportSMSEventArg;
static int ett_camel_SEQUENCE_SIZE_1_bound__numOfSMSEvents_OF_SMSEvent;
static int ett_camel_ResetTimerSMSArg;
static int ett_camel_EstablishTemporaryConnectionArgV2;
static int ett_camel_SEQUENCE_SIZE_1_numOfExtensions_OF_ExtensionField;
static int ett_camel_InitialDPArgExtensionV2;
static int ett_camel_NACarrierInformation;
static int ett_camel_NA_Info;
static int ett_camel_CAMEL_AChBillingChargingCharacteristicsV2;
static int ett_camel_T_timeDurationCharging_01;
static int ett_camel_ReleaseIfDurationExceeded;
static int ett_camel_Code;
static int ett_camel_ROS;
static int ett_camel_Invoke;
static int ett_camel_T_linkedId;
static int ett_camel_ReturnResult;
static int ett_camel_T_result;
static int ett_camel_ReturnError;
static int ett_camel_Reject;
static int ett_camel_T_problem;
static int ett_camel_InvokeId;

static expert_field ei_camel_unknown_invokeData;
static expert_field ei_camel_unknown_returnResultData;
static expert_field ei_camel_unknown_returnErrorData;
static expert_field ei_camel_par_wrong_length;
static expert_field ei_camel_bcd_not_digit;

/* Preference settings default */
#define MAX_SSN 254
static range_t *global_ssn_range;
static dissector_handle_t  camel_handle;
static dissector_handle_t  camel_v1_handle;
static dissector_handle_t  camel_v2_handle;
static dissector_handle_t  camel_v3_handle;
static dissector_handle_t  camel_v4_handle;

/* Global variables */

static uint8_t PDPTypeOrganization;
static uint8_t PDPTypeNumber;
const char *camel_obj_id;
bool is_ExtensionField;

/* Global hash tables*/
static wmem_map_t *srt_calls;
static uint32_t camelsrt_global_SessionId=1;

static int camel_opcode_type;
#define CAMEL_OPCODE_INVOKE        1
#define CAMEL_OPCODE_RETURN_RESULT 2
#define CAMEL_OPCODE_RETURN_ERROR  3
#define CAMEL_OPCODE_REJECT        4

static const value_string camel_Component_vals[] = {
  {   1, "invoke" },
  {   2, "returnResultLast" },
  {   3, "returnError" },
  {   4, "reject" },
  { 0, NULL }
};

const value_string  camelSRTtype_naming[]= {
  { CAMELSRT_SESSION,         "TCAP_Session" },
  { CAMELSRT_VOICE_INITIALDP, "InitialDP/Continue" },
  { CAMELSRT_VOICE_ACR1,      "Slice1_ACR/ACH" },
  { CAMELSRT_VOICE_ACR2,      "Slice2_ACR/ACH" },
  { CAMELSRT_VOICE_ACR3,      "Slice3_ACR/ACH" },
  { CAMELSRT_VOICE_DISC,      "EvtRepBSCM/Release" },
  { CAMELSRT_SMS_INITIALDP,   "InitialDP/ContinueSMS" },
  { CAMELSRT_GPRS_INITIALDP,  "InitialDP/ContinueGPRS" },
  { CAMELSRT_GPRS_REPORT,     "EvtRepGPRS/ContinueGPRS" },
  { 0,NULL}
};

#define EUROPEAN_DATE 1
#define AMERICAN_DATE 2
#define CAMEL_DATE_AND_TIME_LEN 20 /* 2*5 + 4 + 5 + 1 (HH:MM:SS;mm/dd/yyyy) */

static const enum_val_t date_options[] = {
  { "european",         "DD/MM/YYYY",       EUROPEAN_DATE },
  { "american",        "MM/DD/YYYY",        AMERICAN_DATE },
  { NULL, NULL, 0 }
};

static const value_string digit_value[] = {
  { 0,  "0"},
  { 1,  "1"},
  { 2,  "2"},
  { 3,  "3"},
  { 4,  "4"},
  { 5,  "5"},
  { 6,  "6"},
  { 7,  "7"},
  { 8,  "8"},
  { 9,  "9"},
  { 10, "spare"},
  { 11, "spare"},
  { 12, "spare"},
  { 13, "spare"},
  { 0,  NULL}

};


#if 0
static const value_string camel_nature_of_addr_indicator_values[] = {
  {   0x00,  "unknown" },
  {   0x01,  "International Number" },
  {   0x02,  "National Significant Number" },
  {   0x03,  "Network Specific Number" },
  {   0x04,  "Subscriber Number" },
  {   0x05,  "Reserved" },
  {   0x06,  "Abbreviated Number" },
  {   0x07,  "Reserved for extension" },
  { 0, NULL }
};
static const value_string camel_number_plan_values[] = {
  {   0x00,  "unknown" },
  {   0x01,  "ISDN/Telephony Numbering (Rec ITU-T E.164)" },
  {   0x02,  "spare" },
  {   0x03,  "Data Numbering (ITU-T Rec. X.121)" },
  {   0x04,  "Telex Numbering (ITU-T Rec. F.69)" },
  {   0x05,  "spare" },
  {   0x06,  "Land Mobile Numbering (ITU-T Rec. E.212)" },
  {   0x07,  "spare" },
  {   0x08,  "National Numbering" },
  {   0x09,  "Private Numbering" },
  {   0x0f,  "Reserved for extension" },
  { 0, NULL }
};
#endif

/* End includes from old" packet-camel.c */

static const value_string camel_RP_Cause_values[] = {
  { 1, "Unassigned (unallocated) number" },
  { 8, "Operator determined barring" },
  { 10, "Call barred" },
  { 11, "Reserved" },
  { 21, "Short message transfer rejected" },
  { 27, "Destination out of order" },
  { 28, "Unidentified subscriber" },
  { 29, "Facility Rejected" },
  { 30, "Unknown subscriber" },
  { 38, "Network out of order" },
  { 41, "Temporary failure" },
  { 42, "Congestion" },
  { 47, "Resources unavailable, unspecified" },
  { 50, "Requested facility not subscribed" },
  { 69, "Requested facility not implemented" },
  { 81, "Invalid short message transfer reference value" },
  { 95, "Semantically incorrect message" },
  { 96, "Invalid mandatory information" },
  { 97, " Message Type non-existent or not implemented" },
  { 98, "Message not compatible with short message protocol state" },
  { 99, "Information element non existent or not implemented" },
  { 111, "Protocol error, unspecified" },
  { 127, "Interworking, unspecified" },
  { 22,"Memory capacity exceeded" },
  { 0, NULL }
};

static const value_string camel_holdTreatmentIndicator_values[] = {
  {   0x01,  "acceptHoldRequest" },
  {   0x02,  "rejectHoldRequest" },
  { 0, NULL }
};
static const value_string camel_cwTreatmentIndicator_values[] = {
  {   0x01,  "acceptCw" },
  {   0x02,  "rejectCw" },
  { 0, NULL }
};
static const value_string camel_ectTreatmentIndicator_values[] = {
  {   0x01,  "acceptEctRequest" },
  {   0x02,  "rejectEctRequest" },
  { 0, NULL }
};

#define tc_Messages                    "0.0.17.773.2.1.3"
#define tc_NotationExtensions          "0.0.17.775.2.4.1"
#define ros_InformationObjects         "2.4.5.0"
#define datatypes                      "0.4.0.0.1.3.52.7"
#define errortypes                     "0.4.0.0.1.3.51.7"
#define operationcodes                 "0.4.0.0.1.3.53.7"
#define errorcodes                     "0.4.0.0.1.3.57.7"
#define classes                        "0.4.0.0.1.3.54.7"
#define gsmSSF_gsmSCF_Operations       "0.4.0.0.1.3.101.7"
#define gsmSSF_gsmSCF_Protocol         "0.4.0.0.1.3.102.7"
#define gsmSCF_gsmSRF_Operations       "0.4.0.0.1.3.103.7"
#define gsmSCF_gsmSRF_Protocol         "0.4.0.0.1.3.104.7"
#define sms_Operations                 "0.4.0.0.1.3.105.7"
#define smsSSF_gsmSCF_Protocol         "0.4.0.0.1.3.106.7"
#define gprsSSF_gsmSCF_Operations      "0.4.0.0.1.3.107.7"
#define gprsSSF_gsmSCF_Protocol        "0.4.0.0.1.3.108.7"
#define id_CAP                         "0.4.0.0.1.22"
#define id_CAP3                        "0.4.0.0.1.20"
#define id_CAPOE                       "0.4.0.0.1.23"
#define id_CAP3OE                      "0.4.0.0.1.21"
#define id_ac                          id_CAP".3"
#define id_acE                         id_CAPOE".3"
#define id_ac3E                        id_CAP3OE".3"
#define id_as                          id_CAP".5"
#define id_asE                         id_CAPOE".5"
#define id_rosObject                   id_CAP".25"
#define id_contract                    id_CAP".26"
#define id_contract3                   id_CAP3".26"
#define id_contractE                   id_CAPOE".26"
#define id_package                     id_CAP".27"
#define id_packageE                    id_CAPOE".27"
#define id_rosObject_gsmSCF            id_rosObject".4"
#define id_rosObject_gsmSSF            id_rosObject".5"
#define id_rosObject_gsmSRF            id_rosObject".6"
#define id_rosObject_gprsSSF           id_rosObject".7"
#define id_rosObject_smsSSF_V3         id_rosObject".8"
#define id_rosObject_smsSSF_V4         id_rosObject".9"
#define id_ac_CAP_gsmSSF_scfGenericAC  id_acE".4"
#define id_ac_CAP_gsmSSF_scfAssistHandoffAC id_acE".6"
#define id_ac_CAP_scf_gsmSSFGenericAC  id_acE".8"
#define id_ac_gsmSRF_gsmSCF            id_ac".14"
#define id_ac_CAP_gprsSSF_gsmSCF_AC    id_ac3E".50"
#define id_ac_CAP_gsmSCF_gprsSSF_AC    id_ac3E".51"
#define id_ac_cap3_sms_AC              id_ac3E".61"
#define id_ac_cap4_sms_AC              id_acE".61"
#define id_CAPSsfToScfGeneric          id_contractE".3"
#define id_CAPAssistHandoffssfToScf    id_contractE".5"
#define id_CAPScfToSsfGeneric          id_contractE".6"
#define id_contract_gsmSRF_gsmSCF      id_contract".13"
#define id_capGprsSsfTogsmScf          id_contract".14"
#define id_capGsmScfToGprsSsf          id_contract".15"
#define id_cap3SmsSsfTogsmScf          id_contract3".16"
#define id_cap4SmsSsfTogsmScf          id_contract".16"
#define id_package_scfActivation       id_package".11"
#define id_package_gsmSRF_scfActivationOfAssist id_package".15"
#define id_package_assistConnectionEstablishment id_package".16"
#define id_package_genericDisconnectResource id_package".17"
#define id_package_nonAssistedConnectionEstablishment id_package".18"
#define id_package_connect             id_package".19"
#define id_package_callHandling        id_packageE".20"
#define id_package_bcsmEventHandling   id_package".21"
#define id_package_ssfCallProcessing   id_packageE".24"
#define id_package_scfCallInitiation   id_package".25"
#define id_package_timer               id_package".26"
#define id_package_billing             id_package".27"
#define id_package_charging            id_package".28"
#define id_package_trafficManagement   id_package".29"
#define id_package_callReport          id_package".32"
#define id_package_signallingControl   id_package".33"
#define id_package_activityTest        id_package".34"
#define id_package_cancel              id_packageE".36"
#define id_package_cphResponse         id_package".37"
#define id_package_exceptionInform     id_package".38"
#define id_package_playTone            id_package".39"
#define id_package_specializedResourceControl id_package".42"
#define id_package_gsmSRF_scfCancel    id_package".43"
#define id_package_gprsContinue        id_package".49"
#define id_package_gprsExceptionInformation id_package".50"
#define id_package_gprsScfActivation   id_package".51"
#define id_package_gprsConnect         id_package".52"
#define id_package_gprsRelease         id_package".53"
#define id_package_gprsEventHandling   id_package".54"
#define id_package_gprsTimer           id_package".55"
#define id_package_gprsBilling         id_package".56"
#define id_package_gprsCharging        id_package".57"
#define id_package_gprsActivityTest    id_package".58"
#define id_package_gprsCancel          id_package".59"
#define id_package_gprsChargeAdvice    id_package".60"
#define id_package_smsActivation       id_package".61"
#define id_package_smsConnect          id_package".62"
#define id_package_smsContinue         id_package".63"
#define id_package_smsRelease          id_package".64"
#define id_package_smsEventHandling    id_package".65"
#define id_package_smsBilling          id_package".66"
#define id_package_smsTimer            id_package".67"
#define id_as_gsmSSF_scfGenericAS      id_asE".4"
#define id_as_assistHandoff_gsmSSF_scfAS id_asE".6"
#define id_as_scf_gsmSSFGenericAS      id_asE".7"
#define id_as_basic_gsmSRF_gsmSCF      id_as".14"
#define id_as_gprsSSF_gsmSCF_AS        id_as".50"
#define id_as_gsmSCF_gprsSSF_AS        id_as".51"
#define id_as_smsSSF_gsmSCF_AS         id_as".61"
#define leg1                           0x01
#define leg2                           0x02
#define maxSMS_AddressStringLength     11
#define numOfInfoItems                 4
#define errcode_canceled               0
#define errcode_cancelFailed           1
#define errcode_eTCFailed              3
#define errcode_improperCallerResponse 4
#define errcode_missingCustomerRecord  6
#define errcode_missingParameter       7
#define errcode_parameterOutOfRange    8
#define errcode_requestedInfoError     10
#define errcode_systemFailure          11
#define errcode_taskRefused            12
#define errcode_unavailableResource    13
#define errcode_unexpectedComponentSequence 14
#define errcode_unexpectedDataValue    15
#define errcode_unexpectedParameter    16
#define errcode_unknownLegID           17
#define errcode_unknownPDPID           50
#define errcode_unknownCSID            51
#define opcode_initialDP               0
#define opcode_assistRequestInstructions 16
#define opcode_establishTemporaryConnection 17
#define opcode_disconnectForwardConnection 18
#define opcode_dFCWithArgument         86
#define opcode_connectToResource       19
#define opcode_connect                 20
#define opcode_releaseCall             22
#define opcode_requestReportBCSMEvent  23
#define opcode_eventReportBCSM         24
#define opcode_collectInformation      27
#define opcode_continue                31
#define opcode_initiateCallAttempt     32
#define opcode_resetTimer              33
#define opcode_furnishChargingInformation 34
#define opcode_applyCharging           35
#define opcode_applyChargingReport     36
#define opcode_callGap                 41
#define opcode_callInformationReport   44
#define opcode_callInformationRequest  45
#define opcode_sendChargingInformation 46
#define opcode_playAnnouncement        47
#define opcode_promptAndCollectUserInformation 48
#define opcode_specializedResourceReport 49
#define opcode_cancel                  53
#define opcode_activityTest            55
#define opcode_continueWithArgument    88
#define opcode_disconnectLeg           90
#define opcode_moveLeg                 93
#define opcode_splitLeg                95
#define opcode_entityReleased          96
#define opcode_playTone                97
#define opcode_initialDPSMS            60
#define opcode_furnishChargingInformationSMS 61
#define opcode_connectSMS              62
#define opcode_requestReportSMSEvent   63
#define opcode_eventReportSMS          64
#define opcode_continueSMS             65
#define opcode_releaseSMS              66
#define opcode_resetTimerSMS           67
#define opcode_activityTestGPRS        70
#define opcode_applyChargingGPRS       71
#define opcode_applyChargingReportGPRS 72
#define opcode_cancelGPRS              73
#define opcode_connectGPRS             74
#define opcode_continueGPRS            75
#define opcode_entityReleasedGPRS      76
#define opcode_furnishChargingInformationGPRS 77
#define opcode_initialDPGPRS           78
#define opcode_releaseGPRS             79
#define opcode_eventReportGPRS         80
#define opcode_requestReportGPRSEvent  81
#define opcode_resetTimerGPRS          82
#define opcode_sendChargingInformationGPRS 83
#define id_CAP_GPRS_ReferenceNumber    "0.4.0.0.1.1.5.2"
#define id_CAP_U_ABORT_Reason          "0.4.0.0.1.1.2.2"
#define noInvokeId                     NULL


/* CAMEL OPERATIONS */
const value_string camel_opr_code_strings[] = {
	{ opcode_playAnnouncement, "playAnnouncement" },
	{ opcode_promptAndCollectUserInformation, "promptAndCollectUserInformation" },
	{ opcode_specializedResourceReport, "specializedResourceReport" },
	{ opcode_activityTest, "activityTest" },
	{ opcode_applyCharging, "applyCharging" },
	{ opcode_applyChargingReport, "applyChargingReport" },
	{ opcode_assistRequestInstructions, "assistRequestInstructions" },
	{ opcode_callGap, "callGap" },
	{ opcode_callInformationReport, "callInformationReport" },
	{ opcode_callInformationRequest, "callInformationRequest" },
	{ opcode_cancel, "cancel" },
	{ opcode_collectInformation, "collectInformation" },
	{ opcode_connect, "connect" },
	{ opcode_connectToResource, "connectToResource" },
	{ opcode_continue, "continue" },
	{ opcode_continueWithArgument, "continueWithArgument" },
	{ opcode_disconnectForwardConnection, "disconnectForwardConnection" },
	{ opcode_dFCWithArgument, "disconnectForwardConnectionWithArgument" },
	{ opcode_disconnectLeg, "disconnectLeg" },
	{ opcode_entityReleased, "entityReleased" },
	{ opcode_establishTemporaryConnection, "establishTemporaryConnection" },
	{ opcode_eventReportBCSM, "eventReportBCSM" },
	{ opcode_furnishChargingInformation, "furnishChargingInformation" },
	{ opcode_initialDP, "initialDP" },
	{ opcode_initiateCallAttempt, "initiateCallAttempt" },
	{ opcode_moveLeg, "moveLeg" },
	{ opcode_playTone, "playTone" },
	{ opcode_releaseCall, "releaseCall" },
	{ opcode_requestReportBCSMEvent, "requestReportBCSMEvent" },
	{ opcode_resetTimer, "resetTimer" },
	{ opcode_sendChargingInformation, "sendChargingInformation" },
	{ opcode_splitLeg, "splitLeg" },
	{ opcode_activityTestGPRS, "activityTestGPRS" },
	{ opcode_applyChargingGPRS, "applyChargingGPRS" },
	{ opcode_applyChargingReportGPRS, "applyChargingReportGPRS" },
	{ opcode_cancelGPRS, "cancelGPRS" },
	{ opcode_connectGPRS, "connectGPRS" },
	{ opcode_continueGPRS, "continueGPRS" },
	{ opcode_entityReleasedGPRS, "entityReleasedGPRS" },
	{ opcode_eventReportGPRS, "eventReportGPRS" },
	{ opcode_furnishChargingInformationGPRS, "furnishChargingInformationGPRS" },
	{ opcode_initialDPGPRS, "initialDPGPRS" },
	{ opcode_releaseGPRS, "releaseGPRS" },
	{ opcode_requestReportGPRSEvent, "requestReportGPRSEvent" },
	{ opcode_resetTimerGPRS, "resetTimerGPRS" },
	{ opcode_sendChargingInformationGPRS, "sendChargingInformationGPRS" },
	{ opcode_connectSMS, "connectSMS" },
	{ opcode_continueSMS, "continueSMS" },
	{ opcode_eventReportSMS, "eventReportSMS" },
	{ opcode_furnishChargingInformationSMS, "furnishChargingInformationSMS" },
	{ opcode_initialDPSMS, "initialDPSMS" },
	{ opcode_releaseSMS, "releaseSMS" },
	{ opcode_requestReportSMSEvent, "requestReportSMSEvent" },
	{ opcode_resetTimerSMS, "resetTimerSMS" },
  { 0, NULL }
};


/* CAMEL ERRORS */
static const value_string camel_err_code_string_vals[] = {
	{ errcode_canceled, "canceled" },
	{ errcode_cancelFailed, "cancelFailed" },
	{ errcode_eTCFailed, "eTCFailed" },
	{ errcode_improperCallerResponse, "improperCallerResponse" },
	{ errcode_missingCustomerRecord, "missingCustomerRecord" },
	{ errcode_missingParameter, "missingParameter" },
	{ errcode_parameterOutOfRange, "parameterOutOfRange" },
	{ errcode_requestedInfoError, "requestedInfoError" },
	{ errcode_systemFailure, "systemFailure" },
	{ errcode_taskRefused, "taskRefused" },
	{ errcode_unavailableResource, "unavailableResource" },
	{ errcode_unexpectedComponentSequence, "unexpectedComponentSequence" },
	{ errcode_unexpectedDataValue, "unexpectedDataValue" },
	{ errcode_unexpectedParameter, "unexpectedParameter" },
	{ errcode_unknownLegID, "unknownLegID" },
	{ errcode_unknownCSID, "unknownCSID" },
	{ errcode_unknownPDPID, "unknownPDPID" },
  { 0, NULL }
};


/*
 * DEBUG fonctions
 */

#undef DEBUG_CAMELSRT
/* #define DEBUG_CAMELSRT */

#ifdef DEBUG_CAMELSRT
#include <stdio.h>
#include <stdarg.h>
static unsigned debug_level = 99;

static void dbg(unsigned level, char *fmt, ...) {
  va_list ap;

  if (level > debug_level) return;
  va_start(ap,fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}
#endif

static void
camelstat_init(struct register_srt* srt _U_, GArray* srt_array)
{
  srt_stat_table *camel_srt_table;
  char* tmp_str;
  uint32_t i;

  camel_srt_table = init_srt_table("CAMEL Commands", NULL, srt_array, NB_CAMELSRT_CATEGORY, NULL, NULL, NULL);
  for (i = 0; i < NB_CAMELSRT_CATEGORY; i++)
  {
    tmp_str = val_to_str_wmem(NULL,i,camelSRTtype_naming,"Unknown (%d)");
    init_srt_table_row(camel_srt_table, i, tmp_str);
    wmem_free(NULL, tmp_str);
  }
}

static tap_packet_status
camelstat_packet(void *pcamel, packet_info *pinfo, epan_dissect_t *edt _U_, const void *psi, tap_flags_t flags _U_)
{
  unsigned idx = 0;
  srt_stat_table *camel_srt_table;
  const struct camelsrt_info_t * pi=(const struct camelsrt_info_t *)psi;
  srt_data_t *data = (srt_data_t *)pcamel;
  int i;

  for (i=1; i<NB_CAMELSRT_CATEGORY; i++) {
    if ( pi->bool_msginfo[i] &&
         pi->msginfo[i].is_delta_time
         && pi->msginfo[i].request_available
         && !pi->msginfo[i].is_duplicate )
    {
      camel_srt_table = g_array_index(data->srt_array, srt_stat_table*, idx);
      add_srt_table_data(camel_srt_table, i, &pi->msginfo[i].req_time, pinfo);
    }
  } /* category */
  return TAP_PACKET_REDRAW;
}


static char camel_number_to_char(int number)
{
  if (number < 10)
    return (char) (number + 48 ); /* this is ASCII specific */
  else
    return (char) (number + 55 );
}

/*
 * 24.011 8.2.5.4
 */
static uint8_t
dissect_RP_cause_ie(tvbuff_t *tvb, uint32_t offset, _U_ unsigned len,
                    proto_tree *tree, int hf_cause_value, uint8_t *cause_value)
{
  uint8_t oct;
  uint32_t curr_offset;

  curr_offset = offset;
  oct = tvb_get_uint8(tvb, curr_offset);

  *cause_value = oct & 0x7f;

  proto_tree_add_uint(tree, hf_cause_value, tvb, curr_offset, 1, oct);
  curr_offset++;

  if ((oct & 0x80)) {
    oct = tvb_get_uint8(tvb, curr_offset);
    proto_tree_add_uint_format(tree, hf_cause_value,
                               tvb, curr_offset, 1, oct,
                               "Diagnostic : %u", oct);
    curr_offset++;
  }
  return curr_offset - offset;
}

static int dissect_camel_InitialDPArgExtensionV2(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_);



static int
dissect_camel_AccessPointName(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {

  tvbuff_t   *parameter_tvb;
  proto_tree *subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);


  if (!parameter_tvb)
    return offset;
  subtree = proto_item_add_subtree(actx->created_item, ett_camel_AccessPointName);
  de_sm_apn(parameter_tvb, subtree, actx->pinfo, 0, tvb_reported_length(parameter_tvb), NULL, 0);

  return offset;
}



static int
dissect_camel_AChBillingChargingCharacteristics(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
 tvbuff_t	*parameter_tvb;
 proto_tree *subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);

 if (!parameter_tvb)
	return offset;
 subtree = proto_item_add_subtree(actx->created_item, ett_camel_CAMEL_AChBillingChargingCharacteristics);
 if((camel_ver == 2)||(camel_ver == 1)){
	return  dissect_camel_CAMEL_AChBillingChargingCharacteristicsV2(false, parameter_tvb, 0, actx, subtree, hf_camel_CAMEL_AChBillingChargingCharacteristics);
 }
 dissect_camel_CAMEL_AChBillingChargingCharacteristics(false, parameter_tvb, 0, actx, subtree, hf_camel_CAMEL_AChBillingChargingCharacteristics);


  return offset;
}



static int
dissect_camel_CallSegmentID(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const value_string camel_AChChargingAddress_vals[] = {
  {   2, "legID" },
  {  50, "srfConnection" },
  { 0, NULL }
};

static const ber_choice_t AChChargingAddress_choice[] = {
  {   2, &hf_camel_legID         , BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_inap_LegID },
  {  50, &hf_camel_srfConnection , BER_CLASS_CON, 50, BER_FLAGS_IMPLTAG, dissect_camel_CallSegmentID },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_AChChargingAddress(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 AChChargingAddress_choice, hf_index, ett_camel_AChChargingAddress,
                                 NULL);

  return offset;
}



static int
dissect_camel_Digits(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
/*
* Digits {PARAMETERS-BOUND : bound} ::= OCTET STRING (SIZE(
*	bound.&minDigitsLength .. bound.&maxDigitsLength))
*-- Indicates the address signalling digits.
*-- Refer to ETSI EN 300 356 1 [23] Generic Number & Generic Digits parameters for encoding.
*-- The coding of the subfields 'NumberQualifier' in Generic Number and 'TypeOfDigits' in
*-- Generic Digits are irrelevant to the CAP;
*-- the ASN.1 tags are sufficient to identify the parameter.
*-- The ISUP format does not allow to exclude these subfields,
*-- therefore the value is network operator specific.
*--
*-- The following parameters shall use Generic Number:
*--  - AdditionalCallingPartyNumber for InitialDP
*--  - AssistingSSPIPRoutingAddress for EstablishTemporaryConnection
*--  - CorrelationID for AssistRequestInstructions
*--  - CalledAddressValue for all occurrences, CallingAddressValue for all occurrences.
*--
*-- The following parameters shall use Generic Digits:
*--  - CorrelationID in EstablishTemporaryConnection
*--  - number in VariablePart
*--  - digitsResponse in ReceivedInformationArg
*--	- midCallEvents in oMidCallSpecificInfo and tMidCallSpecificInfo
*--
*-- In the digitsResponse and midCallevents, the digits may also include the '*', '#',
*-- a, b, c and d digits by using the IA5 character encoding scheme. If the BCD even or
*-- BCD odd encoding scheme is used, then the following encoding shall be applied for the
*-- non-decimal characters: 1011 (*), 1100 (#).
*--
*-- AssistingSSPIPRoutingAddress in EstablishTemporaryConnection and CorrelationID in
*-- AssistRequestInstructions may contain a Hex B digit as address signal. Refer to
*-- Annex A.6 for the usage of the Hex B digit.
*--
*-- Note that when CorrelationID is transported in Generic Digits, then the digits shall
*-- always be BCD encoded.
*/
 tvbuff_t	*parameter_tvb;
 proto_tree *subtree;
 int ett = -1;
 bool digits = false;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);

  if (!parameter_tvb)
	return offset;

  if (hf_index == hf_camel_calledAddressValue) {
	ett = ett_camel_calledAddressValue;
  } else if (hf_index == hf_camel_callingAddressValue) {
	ett = ett_camel_callingAddressValue;
  } else if (hf_index == hf_camel_additionalCallingPartyNumber) {
	ett = ett_camel_additionalcallingpartynumber;
  } else if (hf_index == hf_camel_assistingSSPIPRoutingAddress) {
	ett = ett_camel_assistingSSPIPRoutingAddress;
  } else if (hf_index == hf_camel_correlationID) {
	ett = ett_camel_correlationID;
	digits = (opcode == opcode_establishTemporaryConnection) ? true : false;
  } else if (hf_index == hf_camel_dTMFDigitsCompleted) {
	ett = ett_camel_dTMFDigitsCompleted;
	digits = true;
  } else if (hf_index == hf_camel_dTMFDigitsTimeOut) {
	ett = ett_camel_dTMFDigitsTimeOut;
	digits = true;
  } else if (hf_index == hf_camel_number) {
	ett = ett_camel_number;
	digits = true;
  } else if (hf_index == hf_camel_digitsResponse) {
	ett = ett_camel_digitsResponse;
	digits = true;
  }

  subtree = proto_item_add_subtree(actx->created_item, ett);
  if (digits) {
	dissect_isup_generic_digits_parameter(parameter_tvb, subtree, NULL);
  } else {
	dissect_isup_generic_number_parameter(parameter_tvb, actx->pinfo, subtree, NULL);
  }


  return offset;
}



static int
dissect_camel_AdditionalCallingPartyNumber(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_camel_Digits(implicit_tag, tvb, offset, actx, tree, hf_index);

  return offset;
}



static int
dissect_camel_AlertingPattern(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}



static int
dissect_camel_INTEGER_0_8191(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const ber_sequence_t CAI_GSM0224_sequence[] = {
  { &hf_camel_e1            , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_8191 },
  { &hf_camel_e2            , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_8191 },
  { &hf_camel_e3            , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_8191 },
  { &hf_camel_e4            , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_8191 },
  { &hf_camel_e5            , BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_8191 },
  { &hf_camel_e6            , BER_CLASS_CON, 5, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_8191 },
  { &hf_camel_e7            , BER_CLASS_CON, 6, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_8191 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CAI_GSM0224(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   CAI_GSM0224_sequence, hf_index, ett_camel_CAI_GSM0224);

  return offset;
}



static int
dissect_camel_INTEGER_1_86400(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const ber_sequence_t AOCSubsequent_sequence[] = {
  { &hf_camel_cAI_GSM0224   , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_CAI_GSM0224 },
  { &hf_camel_aocSubsequent_tariffSwitchInterval, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_86400 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_AOCSubsequent(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   AOCSubsequent_sequence, hf_index, ett_camel_AOCSubsequent);

  return offset;
}


static const ber_sequence_t AOCBeforeAnswer_sequence[] = {
  { &hf_camel_aOCInitial    , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_CAI_GSM0224 },
  { &hf_camel_aOCSubsequent , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_AOCSubsequent },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_AOCBeforeAnswer(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   AOCBeforeAnswer_sequence, hf_index, ett_camel_AOCBeforeAnswer);

  return offset;
}


static const ber_sequence_t AOCGPRS_sequence[] = {
  { &hf_camel_aOCInitial    , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_CAI_GSM0224 },
  { &hf_camel_aOCSubsequent , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_AOCSubsequent },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_AOCGPRS(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   AOCGPRS_sequence, hf_index, ett_camel_AOCGPRS);

  return offset;
}


static const value_string camel_AppendFreeFormatData_vals[] = {
  {   0, "overwrite" },
  {   1, "append" },
  { 0, NULL }
};


static int
dissect_camel_AppendFreeFormatData(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}



static int
dissect_camel_ApplicationTimer(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}



static int
dissect_camel_AssistingSSPIPRoutingAddress(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_camel_Digits(implicit_tag, tvb, offset, actx, tree, hf_index);

  return offset;
}



static int
dissect_camel_BOOLEAN(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_boolean(implicit_tag, actx, tree, tvb, offset, hf_index, NULL);

  return offset;
}



static int
dissect_camel_INTEGER_1_1200(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}



static int
dissect_camel_INTEGER_1_3(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}



static int
dissect_camel_INTEGER_1_20(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const ber_sequence_t Burst_sequence[] = {
  { &hf_camel_numberOfBursts, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_3 },
  { &hf_camel_burstInterval , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_1200 },
  { &hf_camel_numberOfTonesInBurst, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_3 },
  { &hf_camel_burstToneDuration, BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_20 },
  { &hf_camel_toneInterval  , BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_20 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_Burst(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   Burst_sequence, hf_index, ett_camel_Burst);

  return offset;
}


static const ber_sequence_t BurstList_sequence[] = {
  { &hf_camel_warningPeriod , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_1200 },
  { &hf_camel_bursts        , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_Burst },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_BurstList(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   BurstList_sequence, hf_index, ett_camel_BurstList);

  return offset;
}


static const value_string camel_AudibleIndicator_vals[] = {
  {   0, "tone" },
  {   1, "burstList" },
  { 0, NULL }
};

static const ber_choice_t AudibleIndicator_choice[] = {
  {   0, &hf_camel_audibleIndicatorTone, BER_CLASS_UNI, BER_UNI_TAG_BOOLEAN, BER_FLAGS_NOOWNTAG, dissect_camel_BOOLEAN },
  {   1, &hf_camel_burstList     , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_BurstList },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_AudibleIndicator(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 AudibleIndicator_choice, hf_index, ett_camel_AudibleIndicator,
                                 NULL);

  return offset;
}



static int
dissect_camel_OCTET_STRING_SIZE_1(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}


static const ber_sequence_t BackwardServiceInteractionInd_sequence[] = {
  { &hf_camel_conferenceTreatmentIndicator, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_1 },
  { &hf_camel_callCompletionTreatmentIndicator, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_1 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_BackwardServiceInteractionInd(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   BackwardServiceInteractionInd_sequence, hf_index, ett_camel_BackwardServiceInteractionInd);

  return offset;
}


static const ber_sequence_t GapOnService_sequence[] = {
  { &hf_camel_serviceKey    , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_inap_ServiceKey },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_GapOnService(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   GapOnService_sequence, hf_index, ett_camel_GapOnService);

  return offset;
}


static const ber_sequence_t T_calledAddressAndService_sequence[] = {
  { &hf_camel_calledAddressValue, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_Digits },
  { &hf_camel_serviceKey    , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_inap_ServiceKey },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_calledAddressAndService(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_calledAddressAndService_sequence, hf_index, ett_camel_T_calledAddressAndService);

  return offset;
}


static const ber_sequence_t T_callingAddressAndService_sequence[] = {
  { &hf_camel_callingAddressValue, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_Digits },
  { &hf_camel_serviceKey    , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_inap_ServiceKey },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_callingAddressAndService(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_callingAddressAndService_sequence, hf_index, ett_camel_T_callingAddressAndService);

  return offset;
}


static const value_string camel_BasicGapCriteria_vals[] = {
  {   0, "calledAddressValue" },
  {   2, "gapOnService" },
  {  29, "calledAddressAndService" },
  {  30, "callingAddressAndService" },
  { 0, NULL }
};

static const ber_choice_t BasicGapCriteria_choice[] = {
  {   0, &hf_camel_calledAddressValue, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_Digits },
  {   2, &hf_camel_gapOnService  , BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_camel_GapOnService },
  {  29, &hf_camel_calledAddressAndService, BER_CLASS_CON, 29, BER_FLAGS_IMPLTAG, dissect_camel_T_calledAddressAndService },
  {  30, &hf_camel_callingAddressAndService, BER_CLASS_CON, 30, BER_FLAGS_IMPLTAG, dissect_camel_T_callingAddressAndService },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_BasicGapCriteria(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 BasicGapCriteria_choice, hf_index, ett_camel_BasicGapCriteria,
                                 NULL);

  return offset;
}


static const value_string camel_EventTypeBCSM_vals[] = {
  {   2, "collectedInfo" },
  {   3, "analyzedInformation" },
  {   4, "routeSelectFailure" },
  {   5, "oCalledPartyBusy" },
  {   6, "oNoAnswer" },
  {   7, "oAnswer" },
  {   8, "oMidCall" },
  {   9, "oDisconnect" },
  {  10, "oAbandon" },
  {  12, "termAttemptAuthorized" },
  {  13, "tBusy" },
  {  14, "tNoAnswer" },
  {  15, "tAnswer" },
  {  16, "tMidCall" },
  {  17, "tDisconnect" },
  {  18, "tAbandon" },
  {  19, "oTermSeized" },
  {  27, "callAccepted" },
  {  50, "oChangeOfPosition" },
  {  51, "tChangeOfPosition" },
  {  52, "oServiceChange" },
  {  53, "tServiceChange" },
  { 0, NULL }
};


static int
dissect_camel_EventTypeBCSM(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}


static const value_string camel_MonitorMode_vals[] = {
  {   0, "interrupted" },
  {   1, "notifyAndContinue" },
  {   2, "transparent" },
  { 0, NULL }
};


static int
dissect_camel_MonitorMode(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}



static int
dissect_camel_INTEGER_1_30(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}



static int
dissect_camel_OCTET_STRING_SIZE_1_2(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}



static int
dissect_camel_INTEGER_1_127(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const ber_sequence_t MidCallControlInfo_sequence[] = {
  { &hf_camel_minimumNumberOfDigits, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_30 },
  { &hf_camel_maximumNumberOfDigits, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_30 },
  { &hf_camel_endOfReplyDigit, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_1_2 },
  { &hf_camel_cancelDigit   , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_1_2 },
  { &hf_camel_startDigit    , BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_1_2 },
  { &hf_camel_interDigitTimeout, BER_CLASS_CON, 6, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_127 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_MidCallControlInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   MidCallControlInfo_sequence, hf_index, ett_camel_MidCallControlInfo);

  return offset;
}



static int
dissect_camel_NULL(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_null(implicit_tag, actx, tree, tvb, offset, hf_index);

  return offset;
}


static const ber_sequence_t ChangeOfLocationAlt_sequence[] = {
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ChangeOfLocationAlt(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ChangeOfLocationAlt_sequence, hf_index, ett_camel_ChangeOfLocationAlt);

  return offset;
}


static const value_string camel_ChangeOfLocation_vals[] = {
  {   0, "cellGlobalId" },
  {   1, "serviceAreaId" },
  {   2, "locationAreaId" },
  {   3, "inter-SystemHandOver" },
  {   4, "inter-PLMNHandOver" },
  {   5, "inter-MSCHandOver" },
  {   6, "changeOfLocationAlt" },
  { 0, NULL }
};

static const ber_choice_t ChangeOfLocation_choice[] = {
  {   0, &hf_camel_cellGlobalId  , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_gsm_map_CellGlobalIdOrServiceAreaIdFixedLength },
  {   1, &hf_camel_serviceAreaId , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_gsm_map_CellGlobalIdOrServiceAreaIdFixedLength },
  {   2, &hf_camel_locationAreaId, BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_gsm_map_LAIFixedLength },
  {   3, &hf_camel_inter_SystemHandOver, BER_CLASS_CON, 3, BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  {   4, &hf_camel_inter_PLMNHandOver, BER_CLASS_CON, 4, BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  {   5, &hf_camel_inter_MSCHandOver, BER_CLASS_CON, 5, BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  {   6, &hf_camel_changeOfLocationAlt, BER_CLASS_CON, 6, BER_FLAGS_IMPLTAG, dissect_camel_ChangeOfLocationAlt },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ChangeOfLocation(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 ChangeOfLocation_choice, hf_index, ett_camel_ChangeOfLocation,
                                 NULL);

  return offset;
}


static const ber_sequence_t ChangeOfPositionControlInfo_sequence_of[1] = {
  { &hf_camel_ChangeOfPositionControlInfo_item, BER_CLASS_ANY/*choice*/, -1/*choice*/, BER_FLAGS_NOOWNTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_ChangeOfLocation },
};

static int
dissect_camel_ChangeOfPositionControlInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence_of(implicit_tag, actx, tree, tvb, offset,
                                      ChangeOfPositionControlInfo_sequence_of, hf_index, ett_camel_ChangeOfPositionControlInfo);

  return offset;
}



static int
dissect_camel_NumberOfDigits(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const ber_sequence_t DpSpecificCriteriaAlt_sequence[] = {
  { &hf_camel_changeOfPositionControlInfo, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_ChangeOfPositionControlInfo },
  { &hf_camel_numberOfDigits, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NumberOfDigits },
  { &hf_camel_interDigitTimeout, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_127 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_DpSpecificCriteriaAlt(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   DpSpecificCriteriaAlt_sequence, hf_index, ett_camel_DpSpecificCriteriaAlt);

  return offset;
}


static const value_string camel_DpSpecificCriteria_vals[] = {
  {   1, "applicationTimer" },
  {   2, "midCallControlInfo" },
  {   3, "dpSpecificCriteriaAlt" },
  { 0, NULL }
};

static const ber_choice_t DpSpecificCriteria_choice[] = {
  {   1, &hf_camel_applicationTimer, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_ApplicationTimer },
  {   2, &hf_camel_midCallControlInfo, BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_camel_MidCallControlInfo },
  {   3, &hf_camel_dpSpecificCriteriaAlt, BER_CLASS_CON, 3, BER_FLAGS_IMPLTAG, dissect_camel_DpSpecificCriteriaAlt },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_DpSpecificCriteria(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 DpSpecificCriteria_choice, hf_index, ett_camel_DpSpecificCriteria,
                                 NULL);

  return offset;
}


static const ber_sequence_t BCSMEvent_sequence[] = {
  { &hf_camel_eventTypeBCSM , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_EventTypeBCSM },
  { &hf_camel_monitorMode   , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_MonitorMode },
  { &hf_camel_legID         , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_LegID },
  { &hf_camel_dpSpecificCriteria, BER_CLASS_CON, 30, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_DpSpecificCriteria },
  { &hf_camel_automaticRearm, BER_CLASS_CON, 50, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_BCSMEvent(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   BCSMEvent_sequence, hf_index, ett_camel_BCSMEvent);

  return offset;
}



static int
dissect_camel_Cause(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {

tvbuff_t *parameter_tvb;
uint8_t Cause_value;
proto_tree *subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);


 if (!parameter_tvb)
	return offset;
 subtree = proto_item_add_subtree(actx->created_item, ett_camel_cause);

 dissect_q931_cause_ie(parameter_tvb, 0, tvb_reported_length_remaining(parameter_tvb,0), subtree, hf_camel_cause_indicator, &Cause_value, isup_parameter_type_value);

  return offset;
}


static const ber_sequence_t BCSM_Failure_sequence[] = {
  { &hf_camel_legID         , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_LegID },
  { &hf_camel_cause         , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Cause },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_BCSM_Failure(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   BCSM_Failure_sequence, hf_index, ett_camel_BCSM_Failure);

  return offset;
}



static int
dissect_camel_T_bearerCap(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
 tvbuff_t	*parameter_tvb;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);


 if (!parameter_tvb)
	return offset;

 dissect_q931_bearer_capability_ie(parameter_tvb, 0, tvb_reported_length_remaining(parameter_tvb,0), tree);

  return offset;
}


static const value_string camel_BearerCapability_vals[] = {
  {   0, "bearerCap" },
  { 0, NULL }
};

static const ber_choice_t BearerCapability_choice[] = {
  {   0, &hf_camel_bearerCap     , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_T_bearerCap },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_BearerCapability(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 BearerCapability_choice, hf_index, ett_camel_BearerCapability,
                                 NULL);

  return offset;
}



static int
dissect_camel_CalledPartyBCDNumber(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
/* See 3GPP TS 29.078
 * and 3GPP TS 24.008, section 10.5.4.7
 * Indicates the Called Party Number, including service selection information.
 * Refer to 3GPP TS 24.008 [9] for encoding.
 * This data type carries only the 'type of number', 'numbering plan
 * identification' and 'number digit' fields defined in 3GPP TS 24.008 [9];
 * it does not carry the 'called party BCD number IEI' or 'length of called
 * party BCD number contents'.
 * In the context of the DestinationSubscriberNumber field in ConnectSMSArg or
 * InitialDPSMSArg, a CalledPartyBCDNumber may also contain an alphanumeric
 * character string. In this case, type-of-number '101'B is used, in accordance
 * with 3GPP TS 23.040 [6]. The address is coded in accordance with the
 * GSM 7-bit default alphabet definition and the SMS packing rules
 * as specified in 3GPP TS 23.038 [15] in this case.
 */
 tvbuff_t	*parameter_tvb;
 proto_tree *subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);


 if (!parameter_tvb)
	return offset;
 subtree = proto_item_add_subtree(actx->created_item, ett_camel_calledpartybcdnumber);
 de_cld_party_bcd_num(parameter_tvb, subtree, actx->pinfo, 0, tvb_reported_length(parameter_tvb), NULL, 0);


  return offset;
}



static int
dissect_camel_CalledPartyNumber(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  tvbuff_t *parameter_tvb;
  proto_tree *subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);


 if (!parameter_tvb)
	return offset;

 subtree = proto_item_add_subtree(actx->created_item, ett_camel_calledpartybcdnumber);
 dissect_isup_called_party_number_parameter(parameter_tvb, actx->pinfo, subtree, NULL);

  return offset;
}



static int
dissect_camel_CallingPartyNumber(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  tvbuff_t *parameter_tvb;
  proto_tree *subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);


 if (!parameter_tvb)
	return offset;

subtree = proto_item_add_subtree(actx->created_item, ett_camel_callingpartynumber);
dissect_isup_calling_party_number_parameter(parameter_tvb, actx->pinfo, subtree, NULL);

  return offset;
}



static int
dissect_camel_CallResult(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
 tvbuff_t	*parameter_tvb;
 proto_tree *subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);

 if (!parameter_tvb)
	return offset;
 subtree = proto_item_add_subtree(actx->created_item, ett_camel_CAMEL_CallResult);
 dissect_camel_CAMEL_CallResult(false, parameter_tvb, 0, actx, subtree, hf_camel_CAMEL_CallResult);


  return offset;
}


static const ber_sequence_t CallSegmentFailure_sequence[] = {
  { &hf_camel_callSegmentID , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CallSegmentID },
  { &hf_camel_cause         , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Cause },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CallSegmentFailure(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   CallSegmentFailure_sequence, hf_index, ett_camel_CallSegmentFailure);

  return offset;
}



static int
dissect_camel_InvokeID(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const ber_sequence_t CallSegmentToCancel_sequence[] = {
  { &hf_camel_invokeID      , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_InvokeID },
  { &hf_camel_callSegmentID , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CallSegmentID },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CallSegmentToCancel(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   CallSegmentToCancel_sequence, hf_index, ett_camel_CallSegmentToCancel);

  return offset;
}



static int
dissect_camel_INTEGER_1_864000(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}



static int
dissect_camel_T_audibleIndicator(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  if (tvb_reported_length_remaining(tvb,offset) < 2)
	offset = dissect_camel_BOOLEAN(true, tvb, offset, actx , tree, hf_camel_audibleIndicatorTone);
  else
  offset = dissect_camel_AudibleIndicator(implicit_tag, tvb, offset, actx, tree, hf_index);



  return offset;
}



static int
dissect_camel_T_local(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {

  if (is_ExtensionField){
	hf_index = hf_camel_extension_code_local;
  }else if (camel_opcode_type == CAMEL_OPCODE_RETURN_ERROR){
	hf_index = hf_camel_error_code_local;
  }
    offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                &opcode);

  if (is_ExtensionField == false){
	if (camel_opcode_type == CAMEL_OPCODE_RETURN_ERROR){
	  errorCode = opcode;
	  col_append_str(actx->pinfo->cinfo, COL_INFO,
	      val_to_str(errorCode, camel_err_code_string_vals, "Unknown CAMEL error (%u)"));
	  col_append_str(actx->pinfo->cinfo, COL_INFO, " ");
	  col_set_fence(actx->pinfo->cinfo, COL_INFO);
	}else{
	  col_append_str(actx->pinfo->cinfo, COL_INFO,
	     val_to_str(opcode, camel_opr_code_strings, "Unknown CAMEL (%u)"));
	  col_append_str(actx->pinfo->cinfo, COL_INFO, " ");
	  col_set_fence(actx->pinfo->cinfo, COL_INFO);
	}
	gp_camelsrt_info->opcode=opcode;
  }

  return offset;
}



static int
dissect_camel_T_global(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_object_identifier_str(implicit_tag, actx, tree, tvb, offset, hf_index, &camel_obj_id);

  return offset;
}


static const value_string camel_Code_vals[] = {
  {   0, "local" },
  {   1, "global" },
  { 0, NULL }
};

static const ber_choice_t Code_choice[] = {
  {   0, &hf_camel_local         , BER_CLASS_UNI, BER_UNI_TAG_INTEGER, BER_FLAGS_NOOWNTAG, dissect_camel_T_local },
  {   1, &hf_camel_global        , BER_CLASS_UNI, BER_UNI_TAG_OID, BER_FLAGS_NOOWNTAG, dissect_camel_T_global },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_Code(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 Code_choice, hf_index, ett_camel_Code,
                                 NULL);

  return offset;
}



static int
dissect_camel_T_value(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  /*XXX handle local form here */
  if(camel_obj_id){
    offset=call_ber_oid_callback(camel_obj_id, tvb, offset, actx->pinfo, tree, NULL);
  }
  is_ExtensionField = false;


  return offset;
}


static const ber_sequence_t ExtensionField_sequence[] = {
  { &hf_camel_type          , BER_CLASS_ANY/*choice*/, -1/*choice*/, BER_FLAGS_NOOWNTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_Code },
  { &hf_camel_criticality   , BER_CLASS_UNI, BER_UNI_TAG_ENUMERATED, BER_FLAGS_OPTIONAL|BER_FLAGS_NOOWNTAG, dissect_inap_CriticalityType },
  { &hf_camel_value         , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_T_value },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ExtensionField(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
	camel_obj_id = NULL;
	is_ExtensionField =true;

  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ExtensionField_sequence, hf_index, ett_camel_ExtensionField);

  return offset;
}


static const ber_sequence_t Extensions_sequence_of[1] = {
  { &hf_camel_Extensions_item, BER_CLASS_UNI, BER_UNI_TAG_SEQUENCE, BER_FLAGS_NOOWNTAG, dissect_camel_ExtensionField },
};

static int
dissect_camel_Extensions(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence_of(implicit_tag, actx, tree, tvb, offset,
                                      Extensions_sequence_of, hf_index, ett_camel_Extensions);

  return offset;
}


static const ber_sequence_t T_timeDurationCharging_sequence[] = {
  { &hf_camel_maxCallPeriodDuration, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_864000 },
  { &hf_camel_releaseIfdurationExceeded, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_BOOLEAN },
  { &hf_camel_timeDurationCharging_tariffSwitchInterval, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_86400 },
  { &hf_camel_audibleIndicator, BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_T_audibleIndicator },
  { &hf_camel_extensions    , BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_timeDurationCharging(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_timeDurationCharging_sequence, hf_index, ett_camel_T_timeDurationCharging);

  return offset;
}


static const value_string camel_CAMEL_AChBillingChargingCharacteristics_vals[] = {
  {   0, "timeDurationCharging" },
  { 0, NULL }
};

static const ber_choice_t CAMEL_AChBillingChargingCharacteristics_choice[] = {
  {   0, &hf_camel_timeDurationCharging, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_T_timeDurationCharging },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CAMEL_AChBillingChargingCharacteristics(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 CAMEL_AChBillingChargingCharacteristics_choice, hf_index, ett_camel_CAMEL_AChBillingChargingCharacteristics,
                                 NULL);

  return offset;
}



static int
dissect_camel_LegType(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}


static const value_string camel_ReceivingSideID_vals[] = {
  {   1, "receivingSideID" },
  { 0, NULL }
};

static const ber_choice_t ReceivingSideID_choice[] = {
  {   1, &hf_camel_receivingSideID, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_LegType },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ReceivingSideID(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 ReceivingSideID_choice, hf_index, ett_camel_ReceivingSideID,
                                 NULL);

  return offset;
}



static int
dissect_camel_TimeIfNoTariffSwitch(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}



static int
dissect_camel_INTEGER_0_864000(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const ber_sequence_t TimeIfTariffSwitch_sequence[] = {
  { &hf_camel_timeSinceTariffSwitch, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_864000 },
  { &hf_camel_timeIfTariffSwitch_tariffSwitchInterval, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_864000 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_TimeIfTariffSwitch(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   TimeIfTariffSwitch_sequence, hf_index, ett_camel_TimeIfTariffSwitch);

  return offset;
}


static const value_string camel_TimeInformation_vals[] = {
  {   0, "timeIfNoTariffSwitch" },
  {   1, "timeIfTariffSwitch" },
  { 0, NULL }
};

static const ber_choice_t TimeInformation_choice[] = {
  {   0, &hf_camel_timeIfNoTariffSwitch, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_TimeIfNoTariffSwitch },
  {   1, &hf_camel_timeIfTariffSwitch, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_TimeIfTariffSwitch },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_TimeInformation(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 TimeInformation_choice, hf_index, ett_camel_TimeInformation,
                                 NULL);

  return offset;
}


static const ber_sequence_t T_timeDurationChargingResult_sequence[] = {
  { &hf_camel_timeDurationChargingResultpartyToCharge, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_ReceivingSideID },
  { &hf_camel_timeInformation, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_TimeInformation },
  { &hf_camel_legActive     , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_BOOLEAN },
  { &hf_camel_callLegReleasedAtTcpExpiry, BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_extensions    , BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { &hf_camel_aChChargingAddress, BER_CLASS_CON, 5, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_AChChargingAddress },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_timeDurationChargingResult(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_timeDurationChargingResult_sequence, hf_index, ett_camel_T_timeDurationChargingResult);

  return offset;
}


static const value_string camel_CAMEL_CallResult_vals[] = {
  {   0, "timeDurationChargingResult" },
  { 0, NULL }
};

static const ber_choice_t CAMEL_CallResult_choice[] = {
  {   0, &hf_camel_timeDurationChargingResult, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_T_timeDurationChargingResult },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CAMEL_CallResult(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 CAMEL_CallResult_choice, hf_index, ett_camel_CAMEL_CallResult,
                                 NULL);

  return offset;
}



static int
dissect_camel_OCTET_STRING_SIZE_bound__minFCIBillingChargingDataLength_bound__maxFCIBillingChargingDataLength(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}


static const value_string camel_SendingSideID_vals[] = {
  {   0, "sendingSideID" },
  { 0, NULL }
};

static const ber_choice_t SendingSideID_choice[] = {
  {   0, &hf_camel_sendingSideID , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_LegType },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_SendingSideID(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 SendingSideID_choice, hf_index, ett_camel_SendingSideID,
                                 NULL);

  return offset;
}


static const ber_sequence_t T_fci_fCIBCCCAMELsequence1_sequence[] = {
  { &hf_camel_freeFormatData, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_bound__minFCIBillingChargingDataLength_bound__maxFCIBillingChargingDataLength },
  { &hf_camel_fCIBCCCAMELsequence1partyToCharge, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_SendingSideID },
  { &hf_camel_appendFreeFormatData, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_AppendFreeFormatData },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_fci_fCIBCCCAMELsequence1(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_fci_fCIBCCCAMELsequence1_sequence, hf_index, ett_camel_T_fci_fCIBCCCAMELsequence1);

  return offset;
}


static const value_string camel_CAMEL_FCIBillingChargingCharacteristics_vals[] = {
  {   0, "fCIBCCCAMELsequence1" },
  { 0, NULL }
};

static const ber_choice_t CAMEL_FCIBillingChargingCharacteristics_choice[] = {
  {   0, &hf_camel_fci_fCIBCCCAMELsequence1, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_T_fci_fCIBCCCAMELsequence1 },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CAMEL_FCIBillingChargingCharacteristics(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 CAMEL_FCIBillingChargingCharacteristics_choice, hf_index, ett_camel_CAMEL_FCIBillingChargingCharacteristics,
                                 NULL);

  return offset;
}



static int
dissect_camel_PDPID(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}


static const ber_sequence_t T_fciGPRS_fCIBCCCAMELsequence1_sequence[] = {
  { &hf_camel_freeFormatData, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_bound__minFCIBillingChargingDataLength_bound__maxFCIBillingChargingDataLength },
  { &hf_camel_pDPID         , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_PDPID },
  { &hf_camel_appendFreeFormatData, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_AppendFreeFormatData },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_fciGPRS_fCIBCCCAMELsequence1(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_fciGPRS_fCIBCCCAMELsequence1_sequence, hf_index, ett_camel_T_fciGPRS_fCIBCCCAMELsequence1);

  return offset;
}


static const ber_sequence_t CAMEL_FCIGPRSBillingChargingCharacteristics_sequence[] = {
  { &hf_camel_fciGPRS_fCIBCCCAMELsequence1, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_T_fciGPRS_fCIBCCCAMELsequence1 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CAMEL_FCIGPRSBillingChargingCharacteristics(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   CAMEL_FCIGPRSBillingChargingCharacteristics_sequence, hf_index, ett_camel_CAMEL_FCIGPRSBillingChargingCharacteristics);

  return offset;
}


static const ber_sequence_t T_fciSMS_fCIBCCCAMELsequence1_sequence[] = {
  { &hf_camel_freeFormatData, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_bound__minFCIBillingChargingDataLength_bound__maxFCIBillingChargingDataLength },
  { &hf_camel_appendFreeFormatData, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_AppendFreeFormatData },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_fciSMS_fCIBCCCAMELsequence1(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_fciSMS_fCIBCCCAMELsequence1_sequence, hf_index, ett_camel_T_fciSMS_fCIBCCCAMELsequence1);

  return offset;
}


static const value_string camel_CAMEL_FCISMSBillingChargingCharacteristics_vals[] = {
  {   0, "fCIBCCCAMELsequence1" },
  { 0, NULL }
};

static const ber_choice_t CAMEL_FCISMSBillingChargingCharacteristics_choice[] = {
  {   0, &hf_camel_fciSMS_fCIBCCCAMELsequence1, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_T_fciSMS_fCIBCCCAMELsequence1 },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CAMEL_FCISMSBillingChargingCharacteristics(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 CAMEL_FCISMSBillingChargingCharacteristics_choice, hf_index, ett_camel_CAMEL_FCISMSBillingChargingCharacteristics,
                                 NULL);

  return offset;
}


static const ber_sequence_t CAMEL_SCIBillingChargingCharacteristicsAlt_sequence[] = {
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CAMEL_SCIBillingChargingCharacteristicsAlt(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   CAMEL_SCIBillingChargingCharacteristicsAlt_sequence, hf_index, ett_camel_CAMEL_SCIBillingChargingCharacteristicsAlt);

  return offset;
}


static const value_string camel_CAMEL_SCIBillingChargingCharacteristics_vals[] = {
  {   0, "aOCBeforeAnswer" },
  {   1, "aOCAfterAnswer" },
  {   2, "aOC-extension" },
  { 0, NULL }
};

static const ber_choice_t CAMEL_SCIBillingChargingCharacteristics_choice[] = {
  {   0, &hf_camel_aOCBeforeAnswer, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_AOCBeforeAnswer },
  {   1, &hf_camel_aOCAfterAnswer, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_AOCSubsequent },
  {   2, &hf_camel_aOC_extension , BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_camel_CAMEL_SCIBillingChargingCharacteristicsAlt },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CAMEL_SCIBillingChargingCharacteristics(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 CAMEL_SCIBillingChargingCharacteristics_choice, hf_index, ett_camel_CAMEL_SCIBillingChargingCharacteristics,
                                 NULL);

  return offset;
}


static const ber_sequence_t CAMEL_SCIGPRSBillingChargingCharacteristics_sequence[] = {
  { &hf_camel_aOCGPRS       , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_AOCGPRS },
  { &hf_camel_pDPID         , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_PDPID },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CAMEL_SCIGPRSBillingChargingCharacteristics(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   CAMEL_SCIGPRSBillingChargingCharacteristics_sequence, hf_index, ett_camel_CAMEL_SCIGPRSBillingChargingCharacteristics);

  return offset;
}



static int
dissect_camel_Carrier(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}


static const value_string camel_CGEncountered_vals[] = {
  {   0, "noCGencountered" },
  {   1, "manualCGencountered" },
  {   2, "scpOverload" },
  { 0, NULL }
};


static int
dissect_camel_CGEncountered(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}



static int
dissect_camel_ChargeIndicator(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}



static int
dissect_camel_LocationNumber(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  tvbuff_t *parameter_tvb;
  proto_tree *subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);


 if (!parameter_tvb)
        return offset;

subtree = proto_item_add_subtree(actx->created_item, ett_camel_locationnumber);
dissect_isup_location_number_parameter(parameter_tvb, actx->pinfo, subtree, NULL);

  return offset;
}



static int
dissect_camel_ChargeNumber(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_camel_LocationNumber(implicit_tag, tvb, offset, actx, tree, hf_index);

  return offset;
}



static int
dissect_camel_INTEGER_1_4294967295(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const value_string camel_ChargingCharacteristics_vals[] = {
  {   0, "maxTransferredVolume" },
  {   1, "maxElapsedTime" },
  { 0, NULL }
};

static const ber_choice_t ChargingCharacteristics_choice[] = {
  {   0, &hf_camel_maxTransferredVolume, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_4294967295 },
  {   1, &hf_camel_maxElapsedTime, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_86400 },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ChargingCharacteristics(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 ChargingCharacteristics_choice, hf_index, ett_camel_ChargingCharacteristics,
                                 NULL);

  return offset;
}



static int
dissect_camel_INTEGER_0_4294967295(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const ber_sequence_t T_volumeIfTariffSwitch_sequence[] = {
  { &hf_camel_volumeSinceLastTariffSwitch, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_4294967295 },
  { &hf_camel_volumeTariffSwitchInterval, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_4294967295 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_volumeIfTariffSwitch(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_volumeIfTariffSwitch_sequence, hf_index, ett_camel_T_volumeIfTariffSwitch);

  return offset;
}


static const value_string camel_TransferredVolume_vals[] = {
  {   0, "volumeIfNoTariffSwitch" },
  {   1, "volumeIfTariffSwitch" },
  { 0, NULL }
};

static const ber_choice_t TransferredVolume_choice[] = {
  {   0, &hf_camel_volumeIfNoTariffSwitch, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_4294967295 },
  {   1, &hf_camel_volumeIfTariffSwitch, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_T_volumeIfTariffSwitch },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_TransferredVolume(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 TransferredVolume_choice, hf_index, ett_camel_TransferredVolume,
                                 NULL);

  return offset;
}



static int
dissect_camel_INTEGER_0_86400(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const ber_sequence_t T_timeGPRSIfTariffSwitch_sequence[] = {
  { &hf_camel_timeGPRSSinceLastTariffSwitch, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_86400 },
  { &hf_camel_timeGPRSTariffSwitchInterval, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_86400 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_timeGPRSIfTariffSwitch(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_timeGPRSIfTariffSwitch_sequence, hf_index, ett_camel_T_timeGPRSIfTariffSwitch);

  return offset;
}


static const value_string camel_ElapsedTime_vals[] = {
  {   0, "timeGPRSIfNoTariffSwitch" },
  {   1, "timeGPRSIfTariffSwitch" },
  { 0, NULL }
};

static const ber_choice_t ElapsedTime_choice[] = {
  {   0, &hf_camel_timeGPRSIfNoTariffSwitch, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_86400 },
  {   1, &hf_camel_timeGPRSIfTariffSwitch, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_T_timeGPRSIfTariffSwitch },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ElapsedTime(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 ElapsedTime_choice, hf_index, ett_camel_ElapsedTime,
                                 NULL);

  return offset;
}


static const value_string camel_ChargingResult_vals[] = {
  {   0, "transferredVolume" },
  {   1, "elapsedTime" },
  { 0, NULL }
};

static const ber_choice_t ChargingResult_choice[] = {
  {   0, &hf_camel_transferredVolume, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_TransferredVolume },
  {   1, &hf_camel_elapsedTime   , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_ElapsedTime },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ChargingResult(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 ChargingResult_choice, hf_index, ett_camel_ChargingResult,
                                 NULL);

  return offset;
}



static int
dissect_camel_INTEGER_0_255(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const ber_sequence_t T_rO_VolumeIfTariffSwitch_sequence[] = {
  { &hf_camel_rO_VolumeSinceLastTariffSwitch, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_255 },
  { &hf_camel_rO_VolumeTariffSwitchInterval, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_255 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_rO_VolumeIfTariffSwitch(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_rO_VolumeIfTariffSwitch_sequence, hf_index, ett_camel_T_rO_VolumeIfTariffSwitch);

  return offset;
}


static const value_string camel_TransferredVolumeRollOver_vals[] = {
  {   0, "rO-VolumeIfNoTariffSwitch" },
  {   1, "rO-VolumeIfTariffSwitch" },
  { 0, NULL }
};

static const ber_choice_t TransferredVolumeRollOver_choice[] = {
  {   0, &hf_camel_rO_VolumeIfNoTariffSwitch, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_255 },
  {   1, &hf_camel_rO_VolumeIfTariffSwitch, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_T_rO_VolumeIfTariffSwitch },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_TransferredVolumeRollOver(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 TransferredVolumeRollOver_choice, hf_index, ett_camel_TransferredVolumeRollOver,
                                 NULL);

  return offset;
}


static const ber_sequence_t T_rO_TimeGPRSIfTariffSwitch_sequence[] = {
  { &hf_camel_rO_TimeGPRSSinceLastTariffSwitch, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_255 },
  { &hf_camel_rO_TimeGPRSTariffSwitchInterval, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_255 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_rO_TimeGPRSIfTariffSwitch(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_rO_TimeGPRSIfTariffSwitch_sequence, hf_index, ett_camel_T_rO_TimeGPRSIfTariffSwitch);

  return offset;
}


static const value_string camel_ElapsedTimeRollOver_vals[] = {
  {   0, "rO-TimeGPRSIfNoTariffSwitch" },
  {   1, "rO-TimeGPRSIfTariffSwitch" },
  { 0, NULL }
};

static const ber_choice_t ElapsedTimeRollOver_choice[] = {
  {   0, &hf_camel_rO_TimeGPRSIfNoTariffSwitch, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_255 },
  {   1, &hf_camel_rO_TimeGPRSIfTariffSwitch, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_T_rO_TimeGPRSIfTariffSwitch },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ElapsedTimeRollOver(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 ElapsedTimeRollOver_choice, hf_index, ett_camel_ElapsedTimeRollOver,
                                 NULL);

  return offset;
}


static const value_string camel_ChargingRollOver_vals[] = {
  {   0, "transferredVolumeRollOver" },
  {   1, "elapsedTimeRollOver" },
  { 0, NULL }
};

static const ber_choice_t ChargingRollOver_choice[] = {
  {   0, &hf_camel_transferredVolumeRollOver, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_TransferredVolumeRollOver },
  {   1, &hf_camel_elapsedTimeRollOver, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_ElapsedTimeRollOver },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ChargingRollOver(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 ChargingRollOver_choice, hf_index, ett_camel_ChargingRollOver,
                                 NULL);

  return offset;
}


static const value_string camel_ErrorTreatment_vals[] = {
  {   0, "stdErrorAndInfo" },
  {   1, "help" },
  {   2, "repeatPrompt" },
  { 0, NULL }
};


static int
dissect_camel_ErrorTreatment(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}


static const ber_sequence_t CollectedDigits_sequence[] = {
  { &hf_camel_minimumNbOfDigits, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_30 },
  { &hf_camel_maximumNbOfDigits, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_30 },
  { &hf_camel_endOfReplyDigit, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_1_2 },
  { &hf_camel_cancelDigit   , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_1_2 },
  { &hf_camel_startDigit    , BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_1_2 },
  { &hf_camel_firstDigitTimeOut, BER_CLASS_CON, 5, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_127 },
  { &hf_camel_interDigitTimeOut, BER_CLASS_CON, 6, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_127 },
  { &hf_camel_errorTreatment, BER_CLASS_CON, 7, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ErrorTreatment },
  { &hf_camel_interruptableAnnInd, BER_CLASS_CON, 8, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_BOOLEAN },
  { &hf_camel_voiceInformation, BER_CLASS_CON, 9, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_BOOLEAN },
  { &hf_camel_voiceBack     , BER_CLASS_CON, 10, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_BOOLEAN },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CollectedDigits(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   CollectedDigits_sequence, hf_index, ett_camel_CollectedDigits);

  return offset;
}


static const value_string camel_CollectedInfo_vals[] = {
  {   0, "collectedDigits" },
  { 0, NULL }
};

static const ber_choice_t CollectedInfo_choice[] = {
  {   0, &hf_camel_collectedDigits, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_CollectedDigits },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CollectedInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 CollectedInfo_choice, hf_index, ett_camel_CollectedInfo,
                                 NULL);

  return offset;
}


static const value_string camel_ConnectedNumberTreatmentInd_vals[] = {
  {   0, "noINImpact" },
  {   1, "presentationRestricted" },
  {   2, "presentCalledINNumber" },
  {   3, "presentCallINNumberRestricted" },
  { 0, NULL }
};


static int
dissect_camel_ConnectedNumberTreatmentInd(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}


static const value_string camel_ControlType_vals[] = {
  {   0, "sCPOverloaded" },
  {   1, "manuallyInitiated" },
  { 0, NULL }
};


static int
dissect_camel_ControlType(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}



static int
dissect_camel_ScfID(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}


static const ber_sequence_t CompoundCriteria_sequence[] = {
  { &hf_camel_basicGapCriteria, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_BasicGapCriteria },
  { &hf_camel_scfID         , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ScfID },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CompoundCriteria(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   CompoundCriteria_sequence, hf_index, ett_camel_CompoundCriteria);

  return offset;
}



static int
dissect_camel_CorrelationID(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_camel_Digits(implicit_tag, tvb, offset, actx, tree, hf_index);

  return offset;
}



static int
dissect_camel_DateAndTime(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {


/*
* date_option = 1 european dd:mm:yyyy
* date_option = 2 american mm:dd:yyyy
*/

/*
* Output should be HH:MM:SS;dd/mm/yyyy
* if european is selected, and HH:MM:SS;mm/dd/yyyy
* otherwise.
*/

  uint8_t digit_pair;
  uint8_t i = 0, curr_offset;
  char camel_time[CAMEL_DATE_AND_TIME_LEN];
  char c[CAMEL_DATE_AND_TIME_LEN]; /*temporary container*/

  /* 2 digits per octet, 7 octets total + 5 delimiters */

  for (curr_offset = 0; curr_offset < 7 ; curr_offset++)
  /*Loop to extract date*/
  {
      digit_pair = tvb_get_uint8(tvb, curr_offset);

      proto_tree_add_uint(tree,
                          hf_digit,
                          tvb,
                          curr_offset,
                          1,
                          digit_pair & 0x0F);

      proto_tree_add_uint(tree,
                          hf_digit,
                          tvb,
                          curr_offset,
                          1,
                          digit_pair >>4);


      c[i] = camel_number_to_char( digit_pair & 0x0F);
      i++;
      c[i] = camel_number_to_char( digit_pair >>4);
      i++;
  }

  /* Pretty print date */
  /* XXX - Should we use snprintf here instead of assembling the string by
   * hand? */

  camel_time[0] = c[8];
  camel_time[1] = c[9];
  camel_time[2] = ':';
  camel_time[3] = c[10];
  camel_time[4] = c[11];
  camel_time[5] = ':';
  camel_time[6] = c[12];
  camel_time[7] = c[13];
  camel_time[8] = ';';
  if ( EUROPEAN_DATE == date_format) /*european*/
  {
    camel_time[9] = c[6]; /*day*/
    camel_time[10] = c[7];
    camel_time[11] = '/';
    camel_time[12] = c[4]; /*month*/
    camel_time[13] = c[5];
  }
  else /*american*/
  {
    camel_time[9] = c[4]; /*month*/
    camel_time[10] = c[5];
    camel_time[11] = '/';
    camel_time[12] = c[6]; /*day*/
    camel_time[13] = c[7];
  }
  camel_time[14] = '/';
  camel_time[15] = c[0];
  camel_time[16] = c[1];
  camel_time[17] = c[2];
  camel_time[18] = c[3];

  camel_time[CAMEL_DATE_AND_TIME_LEN - 1] = '\0';

/*start = 0, length = 7*/

  proto_tree_add_string(tree,
		      hf_index,
		      tvb,
		      0,
		      7,
		      camel_time);

  return 7; /* 7  octets eaten*/

  return offset;
}


static const ber_sequence_t DestinationRoutingAddress_sequence_of[1] = {
  { &hf_camel_DestinationRoutingAddress_item, BER_CLASS_UNI, BER_UNI_TAG_OCTETSTRING, BER_FLAGS_NOOWNTAG, dissect_camel_CalledPartyNumber },
};

static int
dissect_camel_DestinationRoutingAddress(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence_of(implicit_tag, actx, tree, tvb, offset,
                                      DestinationRoutingAddress_sequence_of, hf_index, ett_camel_DestinationRoutingAddress);

  return offset;
}


static const value_string camel_InitiatorOfServiceChange_vals[] = {
  {   0, "a-side" },
  {   1, "b-side" },
  { 0, NULL }
};


static int
dissect_camel_InitiatorOfServiceChange(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}


static const value_string camel_NatureOfServiceChange_vals[] = {
  {   0, "userInitiated" },
  {   1, "networkInitiated" },
  { 0, NULL }
};


static int
dissect_camel_NatureOfServiceChange(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}


static const ber_sequence_t T_oServiceChangeSpecificInfo_sequence[] = {
  { &hf_camel_ext_basicServiceCode, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_Ext_BasicServiceCode },
  { &hf_camel_initiatorOfServiceChange, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_InitiatorOfServiceChange },
  { &hf_camel_natureOfServiceChange, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NatureOfServiceChange },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_oServiceChangeSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_oServiceChangeSpecificInfo_sequence, hf_index, ett_camel_T_oServiceChangeSpecificInfo);

  return offset;
}


static const ber_sequence_t T_tServiceChangeSpecificInfo_sequence[] = {
  { &hf_camel_ext_basicServiceCode, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_Ext_BasicServiceCode },
  { &hf_camel_initiatorOfServiceChange, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_InitiatorOfServiceChange },
  { &hf_camel_natureOfServiceChange, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NatureOfServiceChange },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_tServiceChangeSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_tServiceChangeSpecificInfo_sequence, hf_index, ett_camel_T_tServiceChangeSpecificInfo);

  return offset;
}


static const ber_sequence_t T_collectedInfoSpecificInfo_sequence[] = {
  { &hf_camel_calledPartyNumber, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CalledPartyNumber },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_collectedInfoSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_collectedInfoSpecificInfo_sequence, hf_index, ett_camel_T_collectedInfoSpecificInfo);

  return offset;
}


static const ber_sequence_t DpSpecificInfoAlt_sequence[] = {
  { &hf_camel_oServiceChangeSpecificInfo, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_T_oServiceChangeSpecificInfo },
  { &hf_camel_tServiceChangeSpecificInfo, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_T_tServiceChangeSpecificInfo },
  { &hf_camel_collectedInfoSpecificInfo, BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_camel_T_collectedInfoSpecificInfo },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_DpSpecificInfoAlt(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   DpSpecificInfoAlt_sequence, hf_index, ett_camel_DpSpecificInfoAlt);

  return offset;
}



static int
dissect_camel_T_pDPTypeOrganization(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {

 tvbuff_t	*parameter_tvb;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);


 if (!parameter_tvb)
	return offset;
 PDPTypeOrganization  = (tvb_get_uint8(parameter_tvb,0) &0x0f);

  return offset;
}



static int
dissect_camel_T_pDPTypeNumber(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {

 tvbuff_t	*parameter_tvb;
 proto_tree *subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);


 if (!parameter_tvb)
	return offset;
 PDPTypeNumber = tvb_get_uint8(parameter_tvb,0);
 subtree = proto_item_add_subtree(actx->created_item, ett_camel_pdptypenumber);
 switch (PDPTypeOrganization){
 case 0: /* ETSI */
	proto_tree_add_item(subtree, hf_camel_PDPTypeNumber_etsi, parameter_tvb, 0, 1, ENC_BIG_ENDIAN);
	break;
 case 1: /* IETF */
	proto_tree_add_item(subtree, hf_camel_PDPTypeNumber_ietf, parameter_tvb, 0, 1, ENC_BIG_ENDIAN);
	break;
 default:
	break;
 }

  return offset;
}



static int
dissect_camel_T_pDPAddress(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {

 tvbuff_t	*parameter_tvb;
 proto_tree *subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);


 if (!parameter_tvb)
	return offset;
 subtree = proto_item_add_subtree(actx->created_item, ett_camel_pdptypenumber);
 switch (PDPTypeOrganization){
 case 0: /* ETSI */
	break;
 case 1: /* IETF */
	switch(PDPTypeNumber){
	case 0x21: /* IPv4 */
		proto_tree_add_item(subtree, hf_camel_PDPAddress_IPv4, parameter_tvb, 0, tvb_reported_length(parameter_tvb), ENC_BIG_ENDIAN);
		break;
	case 0x57: /* IPv6 */
		proto_tree_add_item(subtree, hf_camel_PDPAddress_IPv6, parameter_tvb, 0, tvb_reported_length(parameter_tvb), ENC_NA);
		break;
	default:
		break;
	}
 default:
	break;

 }

  return offset;
}


static const ber_sequence_t EndUserAddress_sequence[] = {
  { &hf_camel_pDPTypeOrganization, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_T_pDPTypeOrganization },
  { &hf_camel_pDPTypeNumber , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_T_pDPTypeNumber },
  { &hf_camel_pDPAddress    , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_T_pDPAddress },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_EndUserAddress(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   EndUserAddress_sequence, hf_index, ett_camel_EndUserAddress);

  return offset;
}


static const ber_sequence_t T_routeSelectFailureSpecificInfo_sequence[] = {
  { &hf_camel_routeSelectfailureCause, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Cause },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_routeSelectFailureSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_routeSelectFailureSpecificInfo_sequence, hf_index, ett_camel_T_routeSelectFailureSpecificInfo);

  return offset;
}


static const ber_sequence_t T_oCalledPartyBusySpecificInfo_sequence[] = {
  { &hf_camel_busyCause     , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Cause },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_oCalledPartyBusySpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_oCalledPartyBusySpecificInfo_sequence, hf_index, ett_camel_T_oCalledPartyBusySpecificInfo);

  return offset;
}


static const ber_sequence_t T_oNoAnswerSpecificInfo_sequence[] = {
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_oNoAnswerSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_oNoAnswerSpecificInfo_sequence, hf_index, ett_camel_T_oNoAnswerSpecificInfo);

  return offset;
}


static const ber_sequence_t T_oAnswerSpecificInfo_sequence[] = {
  { &hf_camel_destinationAddress, BER_CLASS_CON, 50, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CalledPartyNumber },
  { &hf_camel_or_Call       , BER_CLASS_CON, 51, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_forwardedCall , BER_CLASS_CON, 52, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_chargeIndicator, BER_CLASS_CON, 53, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ChargeIndicator },
  { &hf_camel_ext_basicServiceCode, BER_CLASS_CON, 54, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_Ext_BasicServiceCode },
  { &hf_camel_ext_basicServiceCode2, BER_CLASS_CON, 55, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_Ext_BasicServiceCode },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_oAnswerSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_oAnswerSpecificInfo_sequence, hf_index, ett_camel_T_oAnswerSpecificInfo);

  return offset;
}


static const value_string camel_T_omidCallEvents_vals[] = {
  {   3, "dTMFDigitsCompleted" },
  {   4, "dTMFDigitsTimeOut" },
  { 0, NULL }
};

static const ber_choice_t T_omidCallEvents_choice[] = {
  {   3, &hf_camel_dTMFDigitsCompleted, BER_CLASS_CON, 3, BER_FLAGS_IMPLTAG, dissect_camel_Digits },
  {   4, &hf_camel_dTMFDigitsTimeOut, BER_CLASS_CON, 4, BER_FLAGS_IMPLTAG, dissect_camel_Digits },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_omidCallEvents(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 T_omidCallEvents_choice, hf_index, ett_camel_T_omidCallEvents,
                                 NULL);

  return offset;
}


static const ber_sequence_t T_oMidCallSpecificInfo_sequence[] = {
  { &hf_camel_omidCallEvents, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_T_omidCallEvents },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_oMidCallSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_oMidCallSpecificInfo_sequence, hf_index, ett_camel_T_oMidCallSpecificInfo);

  return offset;
}


static const ber_sequence_t T_oDisconnectSpecificInfo_sequence[] = {
  { &hf_camel_releaseCause  , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Cause },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_oDisconnectSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_oDisconnectSpecificInfo_sequence, hf_index, ett_camel_T_oDisconnectSpecificInfo);

  return offset;
}


static const ber_sequence_t T_tBusySpecificInfo_sequence[] = {
  { &hf_camel_busyCause     , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Cause },
  { &hf_camel_callForwarded , BER_CLASS_CON, 50, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_routeNotPermitted, BER_CLASS_CON, 51, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_forwardingDestinationNumber, BER_CLASS_CON, 52, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CalledPartyNumber },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_tBusySpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_tBusySpecificInfo_sequence, hf_index, ett_camel_T_tBusySpecificInfo);

  return offset;
}


static const ber_sequence_t T_tNoAnswerSpecificInfo_sequence[] = {
  { &hf_camel_callForwarded , BER_CLASS_CON, 50, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_forwardingDestinationNumber, BER_CLASS_CON, 52, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CalledPartyNumber },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_tNoAnswerSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_tNoAnswerSpecificInfo_sequence, hf_index, ett_camel_T_tNoAnswerSpecificInfo);

  return offset;
}


static const ber_sequence_t T_tAnswerSpecificInfo_sequence[] = {
  { &hf_camel_destinationAddress, BER_CLASS_CON, 50, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CalledPartyNumber },
  { &hf_camel_or_Call       , BER_CLASS_CON, 51, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_forwardedCall , BER_CLASS_CON, 52, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_chargeIndicator, BER_CLASS_CON, 53, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ChargeIndicator },
  { &hf_camel_ext_basicServiceCode, BER_CLASS_CON, 54, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_Ext_BasicServiceCode },
  { &hf_camel_ext_basicServiceCode2, BER_CLASS_CON, 55, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_Ext_BasicServiceCode },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_tAnswerSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_tAnswerSpecificInfo_sequence, hf_index, ett_camel_T_tAnswerSpecificInfo);

  return offset;
}


static const value_string camel_T_tmidCallEvents_vals[] = {
  {   3, "dTMFDigitsCompleted" },
  {   4, "dTMFDigitsTimeOut" },
  { 0, NULL }
};

static const ber_choice_t T_tmidCallEvents_choice[] = {
  {   3, &hf_camel_dTMFDigitsCompleted, BER_CLASS_CON, 3, BER_FLAGS_IMPLTAG, dissect_camel_Digits },
  {   4, &hf_camel_dTMFDigitsTimeOut, BER_CLASS_CON, 4, BER_FLAGS_IMPLTAG, dissect_camel_Digits },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_tmidCallEvents(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 T_tmidCallEvents_choice, hf_index, ett_camel_T_tmidCallEvents,
                                 NULL);

  return offset;
}


static const ber_sequence_t T_tMidCallSpecificInfo_sequence[] = {
  { &hf_camel_tmidCallEvents, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_T_tmidCallEvents },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_tMidCallSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_tMidCallSpecificInfo_sequence, hf_index, ett_camel_T_tMidCallSpecificInfo);

  return offset;
}


static const ber_sequence_t T_tDisconnectSpecificInfo_sequence[] = {
  { &hf_camel_releaseCause  , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Cause },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_tDisconnectSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_tDisconnectSpecificInfo_sequence, hf_index, ett_camel_T_tDisconnectSpecificInfo);

  return offset;
}


static const ber_sequence_t T_oTermSeizedSpecificInfo_sequence[] = {
  { &hf_camel_locationInformation, BER_CLASS_CON, 50, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_LocationInformation },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_oTermSeizedSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_oTermSeizedSpecificInfo_sequence, hf_index, ett_camel_T_oTermSeizedSpecificInfo);

  return offset;
}


static const ber_sequence_t T_callAcceptedSpecificInfo_sequence[] = {
  { &hf_camel_locationInformation, BER_CLASS_CON, 50, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_LocationInformation },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_callAcceptedSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_callAcceptedSpecificInfo_sequence, hf_index, ett_camel_T_callAcceptedSpecificInfo);

  return offset;
}


static const ber_sequence_t T_oAbandonSpecificInfo_sequence[] = {
  { &hf_camel_routeNotPermitted, BER_CLASS_CON, 50, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_oAbandonSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_oAbandonSpecificInfo_sequence, hf_index, ett_camel_T_oAbandonSpecificInfo);

  return offset;
}


static const ber_sequence_t MetDPCriterionAlt_sequence[] = {
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_MetDPCriterionAlt(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   MetDPCriterionAlt_sequence, hf_index, ett_camel_MetDPCriterionAlt);

  return offset;
}


static const value_string camel_MetDPCriterion_vals[] = {
  {   0, "enteringCellGlobalId" },
  {   1, "leavingCellGlobalId" },
  {   2, "enteringServiceAreaId" },
  {   3, "leavingServiceAreaId" },
  {   4, "enteringLocationAreaId" },
  {   5, "leavingLocationAreaId" },
  {   6, "inter-SystemHandOverToUMTS" },
  {   7, "inter-SystemHandOverToGSM" },
  {   8, "inter-PLMNHandOver" },
  {   9, "inter-MSCHandOver" },
  {  10, "metDPCriterionAlt" },
  { 0, NULL }
};

static const ber_choice_t MetDPCriterion_choice[] = {
  {   0, &hf_camel_enteringCellGlobalId, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_gsm_map_CellGlobalIdOrServiceAreaIdFixedLength },
  {   1, &hf_camel_leavingCellGlobalId, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_gsm_map_CellGlobalIdOrServiceAreaIdFixedLength },
  {   2, &hf_camel_enteringServiceAreaId, BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_gsm_map_CellGlobalIdOrServiceAreaIdFixedLength },
  {   3, &hf_camel_leavingServiceAreaId, BER_CLASS_CON, 3, BER_FLAGS_IMPLTAG, dissect_gsm_map_CellGlobalIdOrServiceAreaIdFixedLength },
  {   4, &hf_camel_enteringLocationAreaId, BER_CLASS_CON, 4, BER_FLAGS_IMPLTAG, dissect_gsm_map_LAIFixedLength },
  {   5, &hf_camel_leavingLocationAreaId, BER_CLASS_CON, 5, BER_FLAGS_IMPLTAG, dissect_gsm_map_LAIFixedLength },
  {   6, &hf_camel_inter_SystemHandOverToUMTS, BER_CLASS_CON, 6, BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  {   7, &hf_camel_inter_SystemHandOverToGSM, BER_CLASS_CON, 7, BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  {   8, &hf_camel_inter_PLMNHandOver, BER_CLASS_CON, 8, BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  {   9, &hf_camel_inter_MSCHandOver, BER_CLASS_CON, 9, BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  {  10, &hf_camel_metDPCriterionAlt, BER_CLASS_CON, 10, BER_FLAGS_IMPLTAG, dissect_camel_MetDPCriterionAlt },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_MetDPCriterion(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 MetDPCriterion_choice, hf_index, ett_camel_MetDPCriterion,
                                 NULL);

  return offset;
}


static const ber_sequence_t MetDPCriteriaList_sequence_of[1] = {
  { &hf_camel_MetDPCriteriaList_item, BER_CLASS_ANY/*choice*/, -1/*choice*/, BER_FLAGS_NOOWNTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_MetDPCriterion },
};

static int
dissect_camel_MetDPCriteriaList(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence_of(implicit_tag, actx, tree, tvb, offset,
                                      MetDPCriteriaList_sequence_of, hf_index, ett_camel_MetDPCriteriaList);

  return offset;
}


static const ber_sequence_t T_oChangeOfPositionSpecificInfo_sequence[] = {
  { &hf_camel_locationInformation, BER_CLASS_CON, 50, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_LocationInformation },
  { &hf_camel_metDPCriteriaList, BER_CLASS_CON, 51, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_MetDPCriteriaList },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_oChangeOfPositionSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_oChangeOfPositionSpecificInfo_sequence, hf_index, ett_camel_T_oChangeOfPositionSpecificInfo);

  return offset;
}


static const ber_sequence_t T_tChangeOfPositionSpecificInfo_sequence[] = {
  { &hf_camel_locationInformation, BER_CLASS_CON, 50, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_LocationInformation },
  { &hf_camel_metDPCriteriaList, BER_CLASS_CON, 51, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_MetDPCriteriaList },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_tChangeOfPositionSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_tChangeOfPositionSpecificInfo_sequence, hf_index, ett_camel_T_tChangeOfPositionSpecificInfo);

  return offset;
}


static const value_string camel_EventSpecificInformationBCSM_vals[] = {
  {   2, "routeSelectFailureSpecificInfo" },
  {   3, "oCalledPartyBusySpecificInfo" },
  {   4, "oNoAnswerSpecificInfo" },
  {   5, "oAnswerSpecificInfo" },
  {   6, "oMidCallSpecificInfo" },
  {   7, "oDisconnectSpecificInfo" },
  {   8, "tBusySpecificInfo" },
  {   9, "tNoAnswerSpecificInfo" },
  {  10, "tAnswerSpecificInfo" },
  {  11, "tMidCallSpecificInfo" },
  {  12, "tDisconnectSpecificInfo" },
  {  13, "oTermSeizedSpecificInfo" },
  {  20, "callAcceptedSpecificInfo" },
  {  21, "oAbandonSpecificInfo" },
  {  50, "oChangeOfPositionSpecificInfo" },
  {  51, "tChangeOfPositionSpecificInfo" },
  {  52, "dpSpecificInfoAlt" },
  { 0, NULL }
};

static const ber_choice_t EventSpecificInformationBCSM_choice[] = {
  {   2, &hf_camel_routeSelectFailureSpecificInfo, BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_camel_T_routeSelectFailureSpecificInfo },
  {   3, &hf_camel_oCalledPartyBusySpecificInfo, BER_CLASS_CON, 3, BER_FLAGS_IMPLTAG, dissect_camel_T_oCalledPartyBusySpecificInfo },
  {   4, &hf_camel_oNoAnswerSpecificInfo, BER_CLASS_CON, 4, BER_FLAGS_IMPLTAG, dissect_camel_T_oNoAnswerSpecificInfo },
  {   5, &hf_camel_oAnswerSpecificInfo, BER_CLASS_CON, 5, BER_FLAGS_IMPLTAG, dissect_camel_T_oAnswerSpecificInfo },
  {   6, &hf_camel_oMidCallSpecificInfo, BER_CLASS_CON, 6, BER_FLAGS_IMPLTAG, dissect_camel_T_oMidCallSpecificInfo },
  {   7, &hf_camel_oDisconnectSpecificInfo, BER_CLASS_CON, 7, BER_FLAGS_IMPLTAG, dissect_camel_T_oDisconnectSpecificInfo },
  {   8, &hf_camel_tBusySpecificInfo, BER_CLASS_CON, 8, BER_FLAGS_IMPLTAG, dissect_camel_T_tBusySpecificInfo },
  {   9, &hf_camel_tNoAnswerSpecificInfo, BER_CLASS_CON, 9, BER_FLAGS_IMPLTAG, dissect_camel_T_tNoAnswerSpecificInfo },
  {  10, &hf_camel_tAnswerSpecificInfo, BER_CLASS_CON, 10, BER_FLAGS_IMPLTAG, dissect_camel_T_tAnswerSpecificInfo },
  {  11, &hf_camel_tMidCallSpecificInfo, BER_CLASS_CON, 11, BER_FLAGS_IMPLTAG, dissect_camel_T_tMidCallSpecificInfo },
  {  12, &hf_camel_tDisconnectSpecificInfo, BER_CLASS_CON, 12, BER_FLAGS_IMPLTAG, dissect_camel_T_tDisconnectSpecificInfo },
  {  13, &hf_camel_oTermSeizedSpecificInfo, BER_CLASS_CON, 13, BER_FLAGS_IMPLTAG, dissect_camel_T_oTermSeizedSpecificInfo },
  {  20, &hf_camel_callAcceptedSpecificInfo, BER_CLASS_CON, 20, BER_FLAGS_IMPLTAG, dissect_camel_T_callAcceptedSpecificInfo },
  {  21, &hf_camel_oAbandonSpecificInfo, BER_CLASS_CON, 21, BER_FLAGS_IMPLTAG, dissect_camel_T_oAbandonSpecificInfo },
  {  50, &hf_camel_oChangeOfPositionSpecificInfo, BER_CLASS_CON, 50, BER_FLAGS_IMPLTAG, dissect_camel_T_oChangeOfPositionSpecificInfo },
  {  51, &hf_camel_tChangeOfPositionSpecificInfo, BER_CLASS_CON, 51, BER_FLAGS_IMPLTAG, dissect_camel_T_tChangeOfPositionSpecificInfo },
  {  52, &hf_camel_dpSpecificInfoAlt, BER_CLASS_CON, 52, BER_FLAGS_IMPLTAG, dissect_camel_DpSpecificInfoAlt },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_EventSpecificInformationBCSM(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 EventSpecificInformationBCSM_choice, hf_index, ett_camel_EventSpecificInformationBCSM,
                                 NULL);

  return offset;
}


static const value_string camel_MO_SMSCause_vals[] = {
  {   0, "systemFailure" },
  {   1, "unexpectedDataValue" },
  {   2, "facilityNotSupported" },
  {   3, "sM-DeliveryFailure" },
  {   4, "releaseFromRadioInterface" },
  { 0, NULL }
};


static int
dissect_camel_MO_SMSCause(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}


static const ber_sequence_t T_o_smsFailureSpecificInfo_sequence[] = {
  { &hf_camel_mo_smsfailureCause, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_MO_SMSCause },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_o_smsFailureSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_o_smsFailureSpecificInfo_sequence, hf_index, ett_camel_T_o_smsFailureSpecificInfo);

  return offset;
}


static const ber_sequence_t T_o_smsSubmissionSpecificInfo_sequence[] = {
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_o_smsSubmissionSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_o_smsSubmissionSpecificInfo_sequence, hf_index, ett_camel_T_o_smsSubmissionSpecificInfo);

  return offset;
}



static int
dissect_camel_MT_SMSCause(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}


static const ber_sequence_t T_t_smsFailureSpecificInfo_sequence[] = {
  { &hf_camel_t_smsfailureCause, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_MT_SMSCause },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_t_smsFailureSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_t_smsFailureSpecificInfo_sequence, hf_index, ett_camel_T_t_smsFailureSpecificInfo);

  return offset;
}


static const ber_sequence_t T_t_smsDeliverySpecificInfo_sequence[] = {
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_t_smsDeliverySpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_t_smsDeliverySpecificInfo_sequence, hf_index, ett_camel_T_t_smsDeliverySpecificInfo);

  return offset;
}


static const value_string camel_EventSpecificInformationSMS_vals[] = {
  {   0, "o-smsFailureSpecificInfo" },
  {   1, "o-smsSubmissionSpecificInfo" },
  {   2, "t-smsFailureSpecificInfo" },
  {   3, "t-smsDeliverySpecificInfo" },
  { 0, NULL }
};

static const ber_choice_t EventSpecificInformationSMS_choice[] = {
  {   0, &hf_camel_o_smsFailureSpecificInfo, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_T_o_smsFailureSpecificInfo },
  {   1, &hf_camel_o_smsSubmissionSpecificInfo, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_T_o_smsSubmissionSpecificInfo },
  {   2, &hf_camel_t_smsFailureSpecificInfo, BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_camel_T_t_smsFailureSpecificInfo },
  {   3, &hf_camel_t_smsDeliverySpecificInfo, BER_CLASS_CON, 3, BER_FLAGS_IMPLTAG, dissect_camel_T_t_smsDeliverySpecificInfo },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_EventSpecificInformationSMS(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 EventSpecificInformationSMS_choice, hf_index, ett_camel_EventSpecificInformationSMS,
                                 NULL);

  return offset;
}


static const value_string camel_EventTypeSMS_vals[] = {
  {   1, "sms-CollectedInfo" },
  {   2, "o-smsFailure" },
  {   3, "o-smsSubmission" },
  {  11, "sms-DeliveryRequested" },
  {  12, "t-smsFailure" },
  {  13, "t-smsDelivery" },
  { 0, NULL }
};


static int
dissect_camel_EventTypeSMS(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}



static int
dissect_camel_FCIBillingChargingCharacteristics(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
 tvbuff_t	*parameter_tvb;
 proto_tree *subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);

 if (!parameter_tvb)
	return offset;
 subtree = proto_item_add_subtree(actx->created_item, ett_camel_CAMEL_FCIBillingChargingCharacteristics);
 dissect_camel_CAMEL_FCIBillingChargingCharacteristics(false, parameter_tvb, 0, actx, subtree, hf_camel_CAMEL_FCIBillingChargingCharacteristics);


  return offset;
}



static int
dissect_camel_FCIGPRSBillingChargingCharacteristics(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
 tvbuff_t	*parameter_tvb;
 proto_tree *subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);

 if (!parameter_tvb)
	return offset;
 subtree = proto_item_add_subtree(actx->created_item, ett_camel_CAMEL_FCIGPRSBillingChargingCharacteristics);
 dissect_camel_CAMEL_FCIGPRSBillingChargingCharacteristics(false, parameter_tvb, 0, actx, subtree, hf_camel_CAMEL_FCIGPRSBillingChargingCharacteristics);


  return offset;
}



static int
dissect_camel_FCISMSBillingChargingCharacteristics(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
 tvbuff_t	*parameter_tvb;
 proto_tree *subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);

 if (!parameter_tvb)
	return offset;
 subtree = proto_item_add_subtree(actx->created_item, ett_camel_CAMEL_FCISMSBillingChargingCharacteristics);
 dissect_camel_CAMEL_FCISMSBillingChargingCharacteristics(false, parameter_tvb, 0, actx, subtree, hf_camel_CAMEL_FCISMSBillingChargingCharacteristics);


  return offset;
}


static const ber_sequence_t ForwardServiceInteractionInd_sequence[] = {
  { &hf_camel_conferenceTreatmentIndicator, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_1 },
  { &hf_camel_callDiversionTreatmentIndicator, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_1 },
  { &hf_camel_callingPartyRestrictionIndicator, BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_1 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ForwardServiceInteractionInd(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ForwardServiceInteractionInd_sequence, hf_index, ett_camel_ForwardServiceInteractionInd);

  return offset;
}


static const value_string camel_GapCriteria_vals[] = {
  {   0, "basicGapCriteria" },
  {   1, "compoundGapCriteria" },
  { 0, NULL }
};

static const ber_choice_t GapCriteria_choice[] = {
  {   0, &hf_camel_basicGapCriteria, BER_CLASS_ANY/*choice*/, -1/*choice*/, BER_FLAGS_NOOWNTAG, dissect_camel_BasicGapCriteria },
  {   1, &hf_camel_compoundGapCriteria, BER_CLASS_UNI, BER_UNI_TAG_SEQUENCE, BER_FLAGS_NOOWNTAG, dissect_camel_CompoundCriteria },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_GapCriteria(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 GapCriteria_choice, hf_index, ett_camel_GapCriteria,
                                 NULL);

  return offset;
}


static const ber_sequence_t GapIndicators_sequence[] = {
  { &hf_camel_gapIndicatorsDuration, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_inap_Duration },
  { &hf_camel_gapInterval   , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_inap_Interval },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_GapIndicators(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   GapIndicators_sequence, hf_index, ett_camel_GapIndicators);

  return offset;
}



static int
dissect_camel_IA5String_SIZE_bound__minMessageContentLength_bound__maxMessageContentLength(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_restricted_string(implicit_tag, BER_UNI_TAG_IA5String,
                                            actx, tree, tvb, offset, hf_index,
                                            NULL);

  return offset;
}



static int
dissect_camel_OCTET_STRING_SIZE_bound__minAttributesLength_bound__maxAttributesLength(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}


static const ber_sequence_t T_text_sequence[] = {
  { &hf_camel_messageContent, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_IA5String_SIZE_bound__minMessageContentLength_bound__maxMessageContentLength },
  { &hf_camel_attributes    , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_bound__minAttributesLength_bound__maxAttributesLength },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_text(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_text_sequence, hf_index, ett_camel_T_text);

  return offset;
}


static const ber_sequence_t SEQUENCE_SIZE_1_bound__numOfMessageIDs_OF_Integer4_sequence_of[1] = {
  { &hf_camel_elementaryMessageIDs_item, BER_CLASS_UNI, BER_UNI_TAG_INTEGER, BER_FLAGS_NOOWNTAG, dissect_inap_Integer4 },
};

static int
dissect_camel_SEQUENCE_SIZE_1_bound__numOfMessageIDs_OF_Integer4(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence_of(implicit_tag, actx, tree, tvb, offset,
                                      SEQUENCE_SIZE_1_bound__numOfMessageIDs_OF_Integer4_sequence_of, hf_index, ett_camel_SEQUENCE_SIZE_1_bound__numOfMessageIDs_OF_Integer4);

  return offset;
}



static int
dissect_camel_OCTET_STRING_SIZE_2(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}



static int
dissect_camel_OCTET_STRING_SIZE_4(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}


static const value_string camel_VariablePart_vals[] = {
  {   0, "integer" },
  {   1, "number" },
  {   2, "time" },
  {   3, "date" },
  {   4, "price" },
  { 0, NULL }
};

static const ber_choice_t VariablePart_choice[] = {
  {   0, &hf_camel_integer       , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_inap_Integer4 },
  {   1, &hf_camel_number        , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_Digits },
  {   2, &hf_camel_time          , BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_2 },
  {   3, &hf_camel_date          , BER_CLASS_CON, 3, BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_4 },
  {   4, &hf_camel_price         , BER_CLASS_CON, 4, BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_4 },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_VariablePart(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 VariablePart_choice, hf_index, ett_camel_VariablePart,
                                 NULL);

  return offset;
}


static const ber_sequence_t SEQUENCE_SIZE_1_5_OF_VariablePart_sequence_of[1] = {
  { &hf_camel_variableParts_item, BER_CLASS_ANY/*choice*/, -1/*choice*/, BER_FLAGS_NOOWNTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_VariablePart },
};

static int
dissect_camel_SEQUENCE_SIZE_1_5_OF_VariablePart(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence_of(implicit_tag, actx, tree, tvb, offset,
                                      SEQUENCE_SIZE_1_5_OF_VariablePart_sequence_of, hf_index, ett_camel_SEQUENCE_SIZE_1_5_OF_VariablePart);

  return offset;
}


static const ber_sequence_t T_variableMessage_sequence[] = {
  { &hf_camel_elementaryMessageID, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_inap_Integer4 },
  { &hf_camel_variableParts , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_SEQUENCE_SIZE_1_5_OF_VariablePart },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_variableMessage(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_variableMessage_sequence, hf_index, ett_camel_T_variableMessage);

  return offset;
}


static const value_string camel_MessageID_vals[] = {
  {   0, "elementaryMessageID" },
  {   1, "text" },
  {  29, "elementaryMessageIDs" },
  {  30, "variableMessage" },
  { 0, NULL }
};

static const ber_choice_t MessageID_choice[] = {
  {   0, &hf_camel_elementaryMessageID, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_inap_Integer4 },
  {   1, &hf_camel_text          , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_T_text },
  {  29, &hf_camel_elementaryMessageIDs, BER_CLASS_CON, 29, BER_FLAGS_IMPLTAG, dissect_camel_SEQUENCE_SIZE_1_bound__numOfMessageIDs_OF_Integer4 },
  {  30, &hf_camel_variableMessage, BER_CLASS_CON, 30, BER_FLAGS_IMPLTAG, dissect_camel_T_variableMessage },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_MessageID(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 MessageID_choice, hf_index, ett_camel_MessageID,
                                 NULL);

  return offset;
}



static int
dissect_camel_INTEGER_0_32767(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const ber_sequence_t InbandInfo_sequence[] = {
  { &hf_camel_messageID     , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_MessageID },
  { &hf_camel_numberOfRepetitions, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_127 },
  { &hf_camel_inbandInfoDuration, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_32767 },
  { &hf_camel_interval      , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_32767 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_InbandInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   InbandInfo_sequence, hf_index, ett_camel_InbandInfo);

  return offset;
}


static const ber_sequence_t Tone_sequence[] = {
  { &hf_camel_toneID        , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_inap_Integer4 },
  { &hf_camel_toneDuration  , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_Integer4 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_Tone(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   Tone_sequence, hf_index, ett_camel_Tone);

  return offset;
}


static const value_string camel_InformationToSend_vals[] = {
  {   0, "inbandInfo" },
  {   1, "tone" },
  { 0, NULL }
};

static const ber_choice_t InformationToSend_choice[] = {
  {   0, &hf_camel_inbandInfo    , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_InbandInfo },
  {   1, &hf_camel_tone          , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_Tone },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_InformationToSend(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 InformationToSend_choice, hf_index, ett_camel_InformationToSend,
                                 NULL);

  return offset;
}


static const value_string camel_GapTreatment_vals[] = {
  {   0, "informationToSend" },
  {   1, "releaseCause" },
  { 0, NULL }
};

static const ber_choice_t GapTreatment_choice[] = {
  {   0, &hf_camel_informationToSend, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_InformationToSend },
  {   1, &hf_camel_releaseCause  , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_Cause },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_GapTreatment(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 GapTreatment_choice, hf_index, ett_camel_GapTreatment,
                                 NULL);

  return offset;
}



static int
dissect_camel_GenericNumber(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  tvbuff_t *parameter_tvb;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);


 if (!parameter_tvb)
        return offset;

dissect_isup_generic_number_parameter(parameter_tvb, actx->pinfo, tree, NULL);

  return offset;
}


static const ber_sequence_t GenericNumbers_set_of[1] = {
  { &hf_camel_GenericNumbers_item, BER_CLASS_UNI, BER_UNI_TAG_OCTETSTRING, BER_FLAGS_NOOWNTAG, dissect_camel_GenericNumber },
};

static int
dissect_camel_GenericNumbers(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_set_of(implicit_tag, actx, tree, tvb, offset,
                                 GenericNumbers_set_of, hf_index, ett_camel_GenericNumbers);

  return offset;
}


static const value_string camel_GPRS_QoS_vals[] = {
  {   0, "short-QoS-format" },
  {   1, "long-QoS-format" },
  { 0, NULL }
};

static const ber_choice_t GPRS_QoS_choice[] = {
  {   0, &hf_camel_short_QoS_format, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_QoS_Subscribed },
  {   1, &hf_camel_long_QoS_format, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_Ext_QoS_Subscribed },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_GPRS_QoS(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 GPRS_QoS_choice, hf_index, ett_camel_GPRS_QoS,
                                 NULL);

  return offset;
}


static const ber_sequence_t GPRS_QoS_Extension_sequence[] = {
  { &hf_camel_supplement_to_long_QoS_format, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_Ext2_QoS_Subscribed },
  { &hf_camel_additionalSupplement, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_Ext3_QoS_Subscribed },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_GPRS_QoS_Extension(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   GPRS_QoS_Extension_sequence, hf_index, ett_camel_GPRS_QoS_Extension);

  return offset;
}



static int
dissect_camel_GPRSCause(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}


static const value_string camel_GPRSEventType_vals[] = {
  {   1, "attach" },
  {   2, "attachChangeOfPosition" },
  {   3, "detached" },
  {  11, "pdp-ContextEstablishment" },
  {  12, "pdp-ContextEstablishmentAcknowledgement" },
  {  13, "disonnect" },
  {  14, "pdp-ContextChangeOfPosition" },
  { 0, NULL }
};


static int
dissect_camel_GPRSEventType(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}


static const ber_sequence_t GPRSEvent_sequence[] = {
  { &hf_camel_gPRSEventType , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_GPRSEventType },
  { &hf_camel_monitorMode   , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_MonitorMode },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_GPRSEvent(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   GPRSEvent_sequence, hf_index, ett_camel_GPRSEvent);

  return offset;
}



static int
dissect_camel_T_cellGlobalIdOrServiceAreaIdOrLAI(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
	proto_tree *subtree;
	int start_offset;

 start_offset = offset;
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);


 subtree = proto_item_add_subtree(actx->created_item, ett_camel_pdptypenumber);

 if (tvb_reported_length_remaining(tvb,start_offset) == 7){
	dissect_gsm_map_CellGlobalIdOrServiceAreaIdFixedLength(true, tvb, start_offset, actx, subtree, hf_camel_cellGlobalIdOrServiceAreaIdFixedLength);
 }else{
	dissect_gsm_map_LAIFixedLength(true, tvb, start_offset, actx, subtree, hf_camel_locationAreaId);
 }

  return offset;
}



static int
dissect_camel_UserCSGInformation(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}


static const ber_sequence_t LocationInformationGPRS_sequence[] = {
  { &hf_camel_cellGlobalIdOrServiceAreaIdOrLAI, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_T_cellGlobalIdOrServiceAreaIdOrLAI },
  { &hf_camel_routeingAreaIdentity, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_RAIdentity },
  { &hf_camel_geographicalInformation, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_GeographicalInformation },
  { &hf_camel_sgsn_Number   , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ISDN_AddressString },
  { &hf_camel_selectedLSAIdentity, BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_LSAIdentity },
  { &hf_camel_extensionContainer, BER_CLASS_CON, 5, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ExtensionContainer },
  { &hf_camel_sai_Present   , BER_CLASS_CON, 6, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_userCSGInformation, BER_CLASS_CON, 7, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_UserCSGInformation },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_LocationInformationGPRS(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   LocationInformationGPRS_sequence, hf_index, ett_camel_LocationInformationGPRS);

  return offset;
}


static const ber_sequence_t T_attachChangeOfPositionSpecificInformation_sequence[] = {
  { &hf_camel_locationInformationGPRS, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_LocationInformationGPRS },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_attachChangeOfPositionSpecificInformation(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_attachChangeOfPositionSpecificInformation_sequence, hf_index, ett_camel_T_attachChangeOfPositionSpecificInformation);

  return offset;
}


static const ber_sequence_t QualityOfService_sequence[] = {
  { &hf_camel_requested_QoS , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_GPRS_QoS },
  { &hf_camel_subscribed_QoS, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_GPRS_QoS },
  { &hf_camel_negotiated_QoS, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_GPRS_QoS },
  { &hf_camel_requested_QoS_Extension, BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_GPRS_QoS_Extension },
  { &hf_camel_subscribed_QoS_Extension, BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_GPRS_QoS_Extension },
  { &hf_camel_negotiated_QoS_Extension, BER_CLASS_CON, 5, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_GPRS_QoS_Extension },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_QualityOfService(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   QualityOfService_sequence, hf_index, ett_camel_QualityOfService);

  return offset;
}



static int
dissect_camel_TimeAndTimezone(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
 tvbuff_t  *parameter_tvb;
 proto_tree *subtree;
 proto_item *item;
 char *digit_str;
 unsigned length;
 char year[5];
 char month[3];
 char day[3];
 char hour[3];
 char minute[3];
 char second[3];

 uint8_t oct;
 int8_t tz;
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);


  if (!parameter_tvb)
    return offset;
  length = tvb_reported_length(parameter_tvb);
  if (length < 8 /*cAPSpecificBoundSetminTimeAndTimezoneLength*/){
    expert_add_info(actx->pinfo, actx->created_item, &ei_camel_par_wrong_length);
    return offset;
  }
  subtree = proto_item_add_subtree(actx->created_item, ett_camel_timeandtimezone);
  item = proto_tree_add_item_ret_display_string(subtree, hf_camel_timeandtimezone_time, parameter_tvb, 0, 7, ENC_BCD_DIGITS_0_9|ENC_LITTLE_ENDIAN, actx->pinfo->pool, &digit_str);

/*
The Time Zone indicates the difference, expressed in quarters of an hour, between the local time and GMT. In the first of the two semi octets,
the first bit (bit 3 of the seventh octet of the TP Service Centre Time Stamp field) represents the algebraic sign of this difference (0: positive, 1: negative).
*/
  oct = tvb_get_uint8(parameter_tvb,7);

  /* packet-gsm_sms.c time dis_field_scts_aux() */
  tz = (oct >> 4) + (oct & 0x07) * 10;
  tz = (oct & 0x08) ? -tz : tz;

  proto_tree_add_int_format_value(subtree, hf_camel_timeandtimezone_tz, parameter_tvb, 7, 1, tz, "GMT %+d hours %d minutes", tz / 4, tz % 4 * 15);

  /* ENC_BCD_DIGITS_0_9 truncates if the nibble is 0xf. */
  if (strlen(digit_str) < 14 || strchr(digit_str, '?')) {
    expert_add_info(actx->pinfo, item, &ei_camel_bcd_not_digit);
    return offset;
  }

  (void) g_strlcpy(year, digit_str, 5);
  (void) g_strlcpy(month, digit_str+4, 3);
  (void) g_strlcpy(day, digit_str+6, 3);
  (void) g_strlcpy(hour, digit_str+8, 3);
  (void) g_strlcpy(minute, digit_str+10, 3);
  (void) g_strlcpy(second, digit_str+12, 3);

  proto_item_append_text(item, " (%s-%s-%s %s:%s:%s)",year,month,day,hour,minute,second);


  return offset;
}


static const ber_sequence_t T_pdp_ContextchangeOfPositionSpecificInformation_sequence[] = {
  { &hf_camel_accessPointName, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_AccessPointName },
  { &hf_camel_chargingID    , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_GPRSChargingID },
  { &hf_camel_locationInformationGPRS, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_LocationInformationGPRS },
  { &hf_camel_endUserAddress, BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_EndUserAddress },
  { &hf_camel_qualityOfService, BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_QualityOfService },
  { &hf_camel_timeAndTimeZone, BER_CLASS_CON, 5, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_TimeAndTimezone },
  { &hf_camel_gGSNAddress   , BER_CLASS_CON, 6, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_GSN_Address },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_pdp_ContextchangeOfPositionSpecificInformation(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_pdp_ContextchangeOfPositionSpecificInformation_sequence, hf_index, ett_camel_T_pdp_ContextchangeOfPositionSpecificInformation);

  return offset;
}


static const value_string camel_InitiatingEntity_vals[] = {
  {   0, "mobileStation" },
  {   1, "sgsn" },
  {   2, "hlr" },
  {   3, "ggsn" },
  { 0, NULL }
};


static int
dissect_camel_InitiatingEntity(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}


static const ber_sequence_t T_detachSpecificInformation_sequence[] = {
  { &hf_camel_initiatingEntity, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_InitiatingEntity },
  { &hf_camel_routeingAreaUpdate, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_detachSpecificInformation(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_detachSpecificInformation_sequence, hf_index, ett_camel_T_detachSpecificInformation);

  return offset;
}


static const ber_sequence_t T_disconnectSpecificInformation_sequence[] = {
  { &hf_camel_initiatingEntity, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_InitiatingEntity },
  { &hf_camel_routeingAreaUpdate, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_disconnectSpecificInformation(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_disconnectSpecificInformation_sequence, hf_index, ett_camel_T_disconnectSpecificInformation);

  return offset;
}


static const value_string camel_PDPInitiationType_vals[] = {
  {   0, "mSInitiated" },
  {   1, "networkInitiated" },
  { 0, NULL }
};


static int
dissect_camel_PDPInitiationType(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}


static const ber_sequence_t T_pDPContextEstablishmentSpecificInformation_sequence[] = {
  { &hf_camel_accessPointName, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_AccessPointName },
  { &hf_camel_endUserAddress, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_EndUserAddress },
  { &hf_camel_qualityOfService, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_QualityOfService },
  { &hf_camel_locationInformationGPRS, BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_LocationInformationGPRS },
  { &hf_camel_timeAndTimeZone, BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_TimeAndTimezone },
  { &hf_camel_pDPInitiationType, BER_CLASS_CON, 5, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_PDPInitiationType },
  { &hf_camel_secondaryPDP_context, BER_CLASS_CON, 6, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_pDPContextEstablishmentSpecificInformation(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_pDPContextEstablishmentSpecificInformation_sequence, hf_index, ett_camel_T_pDPContextEstablishmentSpecificInformation);

  return offset;
}


static const ber_sequence_t T_pDPContextEstablishmentAcknowledgementSpecificInformation_sequence[] = {
  { &hf_camel_accessPointName, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_AccessPointName },
  { &hf_camel_chargingID    , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_GPRSChargingID },
  { &hf_camel_endUserAddress, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_EndUserAddress },
  { &hf_camel_qualityOfService, BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_QualityOfService },
  { &hf_camel_locationInformationGPRS, BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_LocationInformationGPRS },
  { &hf_camel_timeAndTimeZone, BER_CLASS_CON, 5, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_TimeAndTimezone },
  { &hf_camel_gGSNAddress   , BER_CLASS_CON, 6, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_GSN_Address },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_pDPContextEstablishmentAcknowledgementSpecificInformation(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_pDPContextEstablishmentAcknowledgementSpecificInformation_sequence, hf_index, ett_camel_T_pDPContextEstablishmentAcknowledgementSpecificInformation);

  return offset;
}


static const value_string camel_GPRSEventSpecificInformation_vals[] = {
  {   0, "attachChangeOfPositionSpecificInformation" },
  {   1, "pdp-ContextchangeOfPositionSpecificInformation" },
  {   2, "detachSpecificInformation" },
  {   3, "disconnectSpecificInformation" },
  {   4, "pDPContextEstablishmentSpecificInformation" },
  {   5, "pDPContextEstablishmentAcknowledgementSpecificInformation" },
  { 0, NULL }
};

static const ber_choice_t GPRSEventSpecificInformation_choice[] = {
  {   0, &hf_camel_attachChangeOfPositionSpecificInformation, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_T_attachChangeOfPositionSpecificInformation },
  {   1, &hf_camel_pdp_ContextchangeOfPositionSpecificInformation, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_T_pdp_ContextchangeOfPositionSpecificInformation },
  {   2, &hf_camel_detachSpecificInformation, BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_camel_T_detachSpecificInformation },
  {   3, &hf_camel_disconnectSpecificInformation, BER_CLASS_CON, 3, BER_FLAGS_IMPLTAG, dissect_camel_T_disconnectSpecificInformation },
  {   4, &hf_camel_pDPContextEstablishmentSpecificInformation, BER_CLASS_CON, 4, BER_FLAGS_IMPLTAG, dissect_camel_T_pDPContextEstablishmentSpecificInformation },
  {   5, &hf_camel_pDPContextEstablishmentAcknowledgementSpecificInformation, BER_CLASS_CON, 5, BER_FLAGS_IMPLTAG, dissect_camel_T_pDPContextEstablishmentAcknowledgementSpecificInformation },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_GPRSEventSpecificInformation(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 GPRSEventSpecificInformation_choice, hf_index, ett_camel_GPRSEventSpecificInformation,
                                 NULL);

  return offset;
}



static int
dissect_camel_IPRoutingAddress(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_camel_CalledPartyNumber(implicit_tag, tvb, offset, actx, tree, hf_index);

  return offset;
}



static int
dissect_camel_IPSSPCapabilities(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}


static const value_string camel_LegOrCallSegment_vals[] = {
  {   0, "callSegmentID" },
  {   1, "legID" },
  { 0, NULL }
};

static const ber_choice_t LegOrCallSegment_choice[] = {
  {   0, &hf_camel_callSegmentID , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_CallSegmentID },
  {   1, &hf_camel_legID         , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_inap_LegID },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_LegOrCallSegment(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 LegOrCallSegment_choice, hf_index, ett_camel_LegOrCallSegment,
                                 NULL);

  return offset;
}



static int
dissect_camel_LowLayerCompatibility(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}



static int
dissect_camel_NAOliInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}



static int
dissect_camel_OCSIApplicable(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_null(implicit_tag, actx, tree, tvb, offset, hf_index);

  return offset;
}



static int
dissect_camel_OriginalCalledPartyID(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {

 tvbuff_t	*parameter_tvb;
 proto_tree	*subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);


 if (!parameter_tvb)
	return offset;
 subtree = proto_item_add_subtree(actx->created_item, ett_camel_originalcalledpartyid);
 dissect_isup_original_called_number_parameter(parameter_tvb, actx->pinfo, subtree, NULL);

  return offset;
}



static int
dissect_camel_RedirectingPartyID(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {

 tvbuff_t	*parameter_tvb;
 proto_tree	*subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);


 if (!parameter_tvb)
	return offset;
 subtree = proto_item_add_subtree(actx->created_item, ett_camel_redirectingpartyid);
 dissect_isup_redirecting_number_parameter(parameter_tvb, actx->pinfo, subtree, NULL);

  return offset;
}


static const value_string camel_RequestedInformationType_vals[] = {
  {   0, "callAttemptElapsedTime" },
  {   1, "callStopTime" },
  {   2, "callConnectedElapsedTime" },
  {  30, "releaseCause" },
  { 0, NULL }
};


static int
dissect_camel_RequestedInformationType(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}


static const value_string camel_RequestedInformationValue_vals[] = {
  {   0, "callAttemptElapsedTimeValue" },
  {   1, "callStopTimeValue" },
  {   2, "callConnectedElapsedTimeValue" },
  {  30, "releaseCauseValue" },
  { 0, NULL }
};

static const ber_choice_t RequestedInformationValue_choice[] = {
  {   0, &hf_camel_callAttemptElapsedTimeValue, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_0_255 },
  {   1, &hf_camel_callStopTimeValue, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_DateAndTime },
  {   2, &hf_camel_callConnectedElapsedTimeValue, BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_inap_Integer4 },
  {  30, &hf_camel_releaseCauseValue, BER_CLASS_CON, 30, BER_FLAGS_IMPLTAG, dissect_camel_Cause },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_RequestedInformationValue(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 RequestedInformationValue_choice, hf_index, ett_camel_RequestedInformationValue,
                                 NULL);

  return offset;
}


static const ber_sequence_t RequestedInformation_sequence[] = {
  { &hf_camel_requestedInformationType, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_RequestedInformationType },
  { &hf_camel_requestedInformationValue, BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_RequestedInformationValue },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_RequestedInformation(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   RequestedInformation_sequence, hf_index, ett_camel_RequestedInformation);

  return offset;
}


static const ber_sequence_t RequestedInformationList_sequence_of[1] = {
  { &hf_camel_RequestedInformationList_item, BER_CLASS_UNI, BER_UNI_TAG_SEQUENCE, BER_FLAGS_NOOWNTAG, dissect_camel_RequestedInformation },
};

static int
dissect_camel_RequestedInformationList(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence_of(implicit_tag, actx, tree, tvb, offset,
                                      RequestedInformationList_sequence_of, hf_index, ett_camel_RequestedInformationList);

  return offset;
}


static const ber_sequence_t RequestedInformationTypeList_sequence_of[1] = {
  { &hf_camel_RequestedInformationTypeList_item, BER_CLASS_UNI, BER_UNI_TAG_ENUMERATED, BER_FLAGS_NOOWNTAG, dissect_camel_RequestedInformationType },
};

static int
dissect_camel_RequestedInformationTypeList(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence_of(implicit_tag, actx, tree, tvb, offset,
                                      RequestedInformationTypeList_sequence_of, hf_index, ett_camel_RequestedInformationTypeList);

  return offset;
}



static int
dissect_camel_RPCause(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {

tvbuff_t *parameter_tvb;
uint8_t Cause_value;
proto_tree *subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);


 if (!parameter_tvb)
	return offset;
 subtree = proto_item_add_subtree(actx->created_item, ett_camel_RPcause);

 dissect_RP_cause_ie(parameter_tvb, 0, tvb_reported_length_remaining(parameter_tvb,0), subtree, hf_camel_RP_Cause, &Cause_value);

  return offset;
}



static int
dissect_camel_SCIBillingChargingCharacteristics(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
 tvbuff_t	*parameter_tvb;
 proto_tree *subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);

 if (!parameter_tvb)
	return offset;
 subtree = proto_item_add_subtree(actx->created_item, ett_camel_CAMEL_SCIBillingChargingCharacteristics);
 dissect_camel_CAMEL_SCIBillingChargingCharacteristics(false, parameter_tvb, 0, actx, subtree, hf_camel_CAMEL_SCIBillingChargingCharacteristics);


  return offset;
}



static int
dissect_camel_SCIGPRSBillingChargingCharacteristics(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
 tvbuff_t	*parameter_tvb;
 proto_tree *subtree;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &parameter_tvb);

 if (!parameter_tvb)
	return offset;
 subtree = proto_item_add_subtree(actx->created_item, ett_camel_CAMEL_SCIGPRSBillingChargingCharacteristics);
 dissect_camel_CAMEL_SCIGPRSBillingChargingCharacteristics(false, parameter_tvb, 0, actx, subtree, hf_camel_CAMEL_SCIGPRSBillingChargingCharacteristics);


  return offset;
}


static const ber_sequence_t ServiceInteractionIndicatorsTwo_sequence[] = {
  { &hf_camel_forwardServiceInteractionInd, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ForwardServiceInteractionInd },
  { &hf_camel_backwardServiceInteractionInd, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_BackwardServiceInteractionInd },
  { &hf_camel_bothwayThroughConnectionInd, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_BothwayThroughConnectionInd },
  { &hf_camel_connectedNumberTreatmentInd, BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ConnectedNumberTreatmentInd },
  { &hf_camel_nonCUGCall    , BER_CLASS_CON, 13, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_holdTreatmentIndicator, BER_CLASS_CON, 50, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_1 },
  { &hf_camel_cwTreatmentIndicator, BER_CLASS_CON, 51, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_1 },
  { &hf_camel_ectTreatmentIndicator, BER_CLASS_CON, 52, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OCTET_STRING_SIZE_1 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ServiceInteractionIndicatorsTwo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ServiceInteractionIndicatorsTwo_sequence, hf_index, ett_camel_ServiceInteractionIndicatorsTwo);

  return offset;
}



static int
dissect_camel_SGSNCapabilities(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}



static int
dissect_camel_SMS_AddressString(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_gsm_map_AddressString(implicit_tag, tvb, offset, actx, tree, hf_index);

  return offset;
}


static const ber_sequence_t SMSEvent_sequence[] = {
  { &hf_camel_eventTypeSMS  , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_EventTypeSMS },
  { &hf_camel_monitorMode   , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_MonitorMode },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_SMSEvent(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   SMSEvent_sequence, hf_index, ett_camel_SMSEvent);

  return offset;
}


static const value_string camel_TimerID_vals[] = {
  {   0, "tssf" },
  { 0, NULL }
};


static int
dissect_camel_TimerID(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}



static int
dissect_camel_TimerValue(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_inap_Integer4(implicit_tag, tvb, offset, actx, tree, hf_index);

  return offset;
}



static int
dissect_camel_TPDataCodingScheme(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}



static int
dissect_camel_TPProtocolIdentifier(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}



static int
dissect_camel_TPShortMessageSpecificInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}



static int
dissect_camel_TPValidityPeriod(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}


static const value_string camel_UnavailableNetworkResource_vals[] = {
  {   0, "unavailableResources" },
  {   1, "componentFailure" },
  {   2, "basicCallProcessingException" },
  {   3, "resourceStatusFailure" },
  {   4, "endUserFailure" },
  { 0, NULL }
};


static int
dissect_camel_UnavailableNetworkResource(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}


static const value_string camel_T_par_cancelFailedProblem_vals[] = {
  {   0, "unknownOperation" },
  {   1, "tooLate" },
  {   2, "operationNotCancellable" },
  { 0, NULL }
};


static int
dissect_camel_T_par_cancelFailedProblem(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}


static const ber_sequence_t PAR_cancelFailed_sequence[] = {
  { &hf_camel_par_cancelFailedProblem, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_T_par_cancelFailedProblem },
  { &hf_camel_operation     , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_InvokeID },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_PAR_cancelFailed(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   PAR_cancelFailed_sequence, hf_index, ett_camel_PAR_cancelFailed);

  return offset;
}


static const value_string camel_PAR_requestedInfoError_vals[] = {
  {   1, "unknownRequestedInfo" },
  {   2, "requestedInfoNotAvailable" },
  { 0, NULL }
};


static int
dissect_camel_PAR_requestedInfoError(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}


static const value_string camel_PAR_taskRefused_vals[] = {
  {   0, "generic" },
  {   1, "unobtainable" },
  {   2, "congestion" },
  { 0, NULL }
};


static int
dissect_camel_PAR_taskRefused(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}


static const ber_sequence_t CAP_GPRS_ReferenceNumber_sequence[] = {
  { &hf_camel_destinationReference, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_Integer4 },
  { &hf_camel_originationReference, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_Integer4 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CAP_GPRS_ReferenceNumber(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   CAP_GPRS_ReferenceNumber_sequence, hf_index, ett_camel_CAP_GPRS_ReferenceNumber);

  return offset;
}


static const ber_sequence_t PlayAnnouncementArg_sequence[] = {
  { &hf_camel_informationToSend, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_InformationToSend },
  { &hf_camel_disconnectFromIPForbidden, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_BOOLEAN },
  { &hf_camel_requestAnnouncementCompleteNotification, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_BOOLEAN },
  { &hf_camel_extensions    , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { &hf_camel_callSegmentID , BER_CLASS_CON, 5, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CallSegmentID },
  { &hf_camel_requestAnnouncementStartedNotification, BER_CLASS_CON, 51, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_BOOLEAN },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_PlayAnnouncementArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   PlayAnnouncementArg_sequence, hf_index, ett_camel_PlayAnnouncementArg);

  return offset;
}


static const ber_sequence_t PromptAndCollectUserInformationArg_sequence[] = {
  { &hf_camel_collectedInfo , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_CollectedInfo },
  { &hf_camel_disconnectFromIPForbidden, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_BOOLEAN },
  { &hf_camel_informationToSend, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_InformationToSend },
  { &hf_camel_extensions    , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { &hf_camel_callSegmentID , BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CallSegmentID },
  { &hf_camel_requestAnnouncementStartedNotification, BER_CLASS_CON, 51, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_BOOLEAN },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_PromptAndCollectUserInformationArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   PromptAndCollectUserInformationArg_sequence, hf_index, ett_camel_PromptAndCollectUserInformationArg);

  return offset;
}


static const value_string camel_ReceivedInformationArg_vals[] = {
  {   0, "digitsResponse" },
  { 0, NULL }
};

static const ber_choice_t ReceivedInformationArg_choice[] = {
  {   0, &hf_camel_digitsResponse, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_Digits },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ReceivedInformationArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 ReceivedInformationArg_choice, hf_index, ett_camel_ReceivedInformationArg,
                                 NULL);

  return offset;
}


static const value_string camel_SpecializedResourceReportArg_vals[] = {
  {  50, "allAnnouncementsComplete" },
  {  51, "firstAnnouncementStarted" },
  { 0, NULL }
};

static const ber_choice_t SpecializedResourceReportArg_choice[] = {
  {  50, &hf_camel_allAnnouncementsComplete, BER_CLASS_CON, 50, BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  {  51, &hf_camel_firstAnnouncementStarted, BER_CLASS_CON, 51, BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_SpecializedResourceReportArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  if (camel_ver < 4) {
    return dissect_camel_SpecializedResourceReportArgV23(implicit_tag, tvb, offset, actx, tree, hf_camel_allAnnouncementsComplete);
  }
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 SpecializedResourceReportArg_choice, hf_index, ett_camel_SpecializedResourceReportArg,
                                 NULL);



  return offset;
}


static const ber_sequence_t ApplyChargingArg_sequence[] = {
  { &hf_camel_aChBillingChargingCharacteristics, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_AChBillingChargingCharacteristics },
  { &hf_camel_partyToCharge , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_SendingSideID },
  { &hf_camel_extensions    , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { &hf_camel_aChChargingAddress, BER_CLASS_CON, 50, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_AChChargingAddress },
  { &hf_camel_iTXcharging   , BER_CLASS_CON, 64, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_BOOLEAN },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ApplyChargingArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ApplyChargingArg_sequence, hf_index, ett_camel_ApplyChargingArg);

  return offset;
}



static int
dissect_camel_ApplyChargingReportArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_camel_CallResult(implicit_tag, tvb, offset, actx, tree, hf_index);

  return offset;
}


static const ber_sequence_t AssistRequestInstructionsArg_sequence[] = {
  { &hf_camel_correlationID , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_CorrelationID },
  { &hf_camel_iPSSPCapabilities, BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_camel_IPSSPCapabilities },
  { &hf_camel_extensions    , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_AssistRequestInstructionsArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   AssistRequestInstructionsArg_sequence, hf_index, ett_camel_AssistRequestInstructionsArg);

  return offset;
}


static const ber_sequence_t CallGapArg_sequence[] = {
  { &hf_camel_gapCriteria   , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_GapCriteria },
  { &hf_camel_gapIndicators , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_GapIndicators },
  { &hf_camel_controlType   , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ControlType },
  { &hf_camel_gapTreatment  , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_GapTreatment },
  { &hf_camel_extensions    , BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CallGapArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   CallGapArg_sequence, hf_index, ett_camel_CallGapArg);

  return offset;
}


static const ber_sequence_t CallInformationReportArg_sequence[] = {
  { &hf_camel_requestedInformationList, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_RequestedInformationList },
  { &hf_camel_extensions    , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { &hf_camel_legID_01      , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_ReceivingSideID },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CallInformationReportArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   CallInformationReportArg_sequence, hf_index, ett_camel_CallInformationReportArg);

  return offset;
}


static const ber_sequence_t CallInformationRequestArg_sequence[] = {
  { &hf_camel_requestedInformationTypeList, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_RequestedInformationTypeList },
  { &hf_camel_extensions    , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { &hf_camel_legID_02      , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_SendingSideID },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CallInformationRequestArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   CallInformationRequestArg_sequence, hf_index, ett_camel_CallInformationRequestArg);

  return offset;
}


static const value_string camel_CancelArg_vals[] = {
  {   0, "invokeID" },
  {   1, "allRequests" },
  {   2, "callSegmentToCancel" },
  { 0, NULL }
};

static const ber_choice_t CancelArg_choice[] = {
  {   0, &hf_camel_invokeID      , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_InvokeID },
  {   1, &hf_camel_allRequests   , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  {   2, &hf_camel_callSegmentToCancel, BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_camel_CallSegmentToCancel },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CancelArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 CancelArg_choice, hf_index, ett_camel_CancelArg,
                                 NULL);

  return offset;
}


static const ber_sequence_t CollectInformationArg_sequence[] = {
  { &hf_camel_extensions    , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CollectInformationArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   CollectInformationArg_sequence, hf_index, ett_camel_CollectInformationArg);

  return offset;
}


static const ber_sequence_t ConnectArg_sequence[] = {
  { &hf_camel_destinationRoutingAddress, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_DestinationRoutingAddress },
  { &hf_camel_alertingPattern, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_AlertingPattern },
  { &hf_camel_originalCalledPartyID, BER_CLASS_CON, 6, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OriginalCalledPartyID },
  { &hf_camel_extensions    , BER_CLASS_CON, 10, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { &hf_camel_carrier       , BER_CLASS_CON, 11, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Carrier },
  { &hf_camel_callingPartysCategory, BER_CLASS_CON, 28, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_CallingPartysCategory },
  { &hf_camel_redirectingPartyID, BER_CLASS_CON, 29, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_RedirectingPartyID },
  { &hf_camel_redirectionInformation, BER_CLASS_CON, 30, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_RedirectionInformation },
  { &hf_camel_genericNumbers, BER_CLASS_CON, 14, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_GenericNumbers },
  { &hf_camel_serviceInteractionIndicatorsTwo, BER_CLASS_CON, 15, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ServiceInteractionIndicatorsTwo },
  { &hf_camel_chargeNumber  , BER_CLASS_CON, 19, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ChargeNumber },
  { &hf_camel_legToBeConnected, BER_CLASS_CON, 21, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_LegID },
  { &hf_camel_cug_Interlock , BER_CLASS_CON, 31, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_CUG_Interlock },
  { &hf_camel_cug_OutgoingAccess, BER_CLASS_CON, 32, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_suppressionOfAnnouncement, BER_CLASS_CON, 55, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ch_SuppressionOfAnnouncement },
  { &hf_camel_oCSIApplicable, BER_CLASS_CON, 56, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OCSIApplicable },
  { &hf_camel_naOliInfo     , BER_CLASS_CON, 57, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NAOliInfo },
  { &hf_camel_bor_InterrogationRequested, BER_CLASS_CON, 58, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_suppress_N_CSI, BER_CLASS_CON, 59, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ConnectArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ConnectArg_sequence, hf_index, ett_camel_ConnectArg);

  return offset;
}


static const value_string camel_T_resourceAddress_vals[] = {
  {   0, "ipRoutingAddress" },
  {   3, "none" },
  { 0, NULL }
};

static const ber_choice_t T_resourceAddress_choice[] = {
  {   0, &hf_camel_ipRoutingAddress, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_IPRoutingAddress },
  {   3, &hf_camel_none          , BER_CLASS_CON, 3, BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_resourceAddress(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 T_resourceAddress_choice, hf_index, ett_camel_T_resourceAddress,
                                 NULL);

  return offset;
}


static const ber_sequence_t ConnectToResourceArg_sequence[] = {
  { &hf_camel_resourceAddress, BER_CLASS_ANY/*choice*/, -1/*choice*/, BER_FLAGS_NOOWNTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_T_resourceAddress },
  { &hf_camel_extensions    , BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { &hf_camel_serviceInteractionIndicatorsTwo, BER_CLASS_CON, 7, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ServiceInteractionIndicatorsTwo },
  { &hf_camel_callSegmentID , BER_CLASS_CON, 50, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CallSegmentID },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ConnectToResourceArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ConnectToResourceArg_sequence, hf_index, ett_camel_ConnectToResourceArg);

  return offset;
}


static const ber_sequence_t ContinueWithArgumentArgExtension_sequence[] = {
  { &hf_camel_suppress_D_CSI, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_suppress_N_CSI, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_suppressOutgoingCallBarring, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_legOrCallSegment, BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_LegOrCallSegment },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ContinueWithArgumentArgExtension(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ContinueWithArgumentArgExtension_sequence, hf_index, ett_camel_ContinueWithArgumentArgExtension);

  return offset;
}


static const ber_sequence_t ContinueWithArgumentArg_sequence[] = {
  { &hf_camel_alertingPattern, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_AlertingPattern },
  { &hf_camel_extensions    , BER_CLASS_CON, 6, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { &hf_camel_serviceInteractionIndicatorsTwo, BER_CLASS_CON, 7, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ServiceInteractionIndicatorsTwo },
  { &hf_camel_callingPartysCategory, BER_CLASS_CON, 12, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_CallingPartysCategory },
  { &hf_camel_genericNumbers, BER_CLASS_CON, 16, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_GenericNumbers },
  { &hf_camel_cug_Interlock , BER_CLASS_CON, 17, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_CUG_Interlock },
  { &hf_camel_cug_OutgoingAccess, BER_CLASS_CON, 18, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_chargeNumber  , BER_CLASS_CON, 50, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ChargeNumber },
  { &hf_camel_carrier       , BER_CLASS_CON, 52, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Carrier },
  { &hf_camel_suppressionOfAnnouncement, BER_CLASS_CON, 55, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ch_SuppressionOfAnnouncement },
  { &hf_camel_naOliInfo     , BER_CLASS_CON, 56, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NAOliInfo },
  { &hf_camel_bor_InterrogationRequested, BER_CLASS_CON, 57, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_suppress_O_CSI, BER_CLASS_CON, 58, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_continueWithArgumentArgExtension, BER_CLASS_CON, 59, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ContinueWithArgumentArgExtension },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ContinueWithArgumentArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ContinueWithArgumentArg_sequence, hf_index, ett_camel_ContinueWithArgumentArg);

  return offset;
}


static const ber_sequence_t DisconnectForwardConnectionWithArgumentArg_sequence[] = {
  { &hf_camel_callSegmentID , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CallSegmentID },
  { &hf_camel_extensions    , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_DisconnectForwardConnectionWithArgumentArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   DisconnectForwardConnectionWithArgumentArg_sequence, hf_index, ett_camel_DisconnectForwardConnectionWithArgumentArg);

  return offset;
}


static const ber_sequence_t DisconnectLegArg_sequence[] = {
  { &hf_camel_legToBeReleased, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_inap_LegID },
  { &hf_camel_releaseCause  , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Cause },
  { &hf_camel_extensions    , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_DisconnectLegArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   DisconnectLegArg_sequence, hf_index, ett_camel_DisconnectLegArg);

  return offset;
}


static const value_string camel_EntityReleasedArg_vals[] = {
  {   0, "callSegmentFailure" },
  {   1, "bCSM-Failure" },
  { 0, NULL }
};

static const ber_choice_t EntityReleasedArg_choice[] = {
  {   0, &hf_camel_callSegmentFailure, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_CallSegmentFailure },
  {   1, &hf_camel_bCSM_Failure  , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_BCSM_Failure },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_EntityReleasedArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 EntityReleasedArg_choice, hf_index, ett_camel_EntityReleasedArg,
                                 NULL);

  return offset;
}


static const ber_sequence_t EstablishTemporaryConnectionArg_sequence[] = {
  { &hf_camel_assistingSSPIPRoutingAddress, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_AssistingSSPIPRoutingAddress },
  { &hf_camel_correlationID , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CorrelationID },
  { &hf_camel_scfID         , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ScfID },
  { &hf_camel_extensions    , BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { &hf_camel_carrier       , BER_CLASS_CON, 5, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Carrier },
  { &hf_camel_serviceInteractionIndicatorsTwo, BER_CLASS_CON, 6, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ServiceInteractionIndicatorsTwo },
  { &hf_camel_callSegmentID , BER_CLASS_CON, 7, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CallSegmentID },
  { &hf_camel_naOliInfo     , BER_CLASS_CON, 50, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NAOliInfo },
  { &hf_camel_chargeNumber  , BER_CLASS_CON, 51, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ChargeNumber },
  { &hf_camel_originalCalledPartyID, BER_CLASS_CON, 52, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OriginalCalledPartyID },
  { &hf_camel_callingPartyNumber, BER_CLASS_CON, 53, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CallingPartyNumber },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_EstablishTemporaryConnectionArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {

  if(camel_ver==2){
    return dissect_camel_EstablishTemporaryConnectionArgV2(implicit_tag, tvb, offset, actx, tree, hf_index);
  }
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   EstablishTemporaryConnectionArg_sequence, hf_index, ett_camel_EstablishTemporaryConnectionArg);



  return offset;
}


static const ber_sequence_t EventReportBCSMArg_sequence[] = {
  { &hf_camel_eventTypeBCSM , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_EventTypeBCSM },
  { &hf_camel_eventSpecificInformationBCSM, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_EventSpecificInformationBCSM },
  { &hf_camel_legID_01      , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_ReceivingSideID },
  { &hf_camel_miscCallInfo  , BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_MiscCallInfo },
  { &hf_camel_extensions    , BER_CLASS_CON, 5, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_EventReportBCSMArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   EventReportBCSMArg_sequence, hf_index, ett_camel_EventReportBCSMArg);

  return offset;
}



static int
dissect_camel_FurnishChargingInformationArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_camel_FCIBillingChargingCharacteristics(implicit_tag, tvb, offset, actx, tree, hf_index);

  return offset;
}


static const ber_sequence_t InitialDPArgExtension_sequence[] = {
  { &hf_camel_gmscAddress   , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ISDN_AddressString },
  { &hf_camel_forwardingDestinationNumber, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CalledPartyNumber },
  { &hf_camel_ms_Classmark2 , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_MS_Classmark2 },
  { &hf_camel_iMEI          , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_IMEI },
  { &hf_camel_supportedCamelPhases, BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_SupportedCamelPhases },
  { &hf_camel_offeredCamel4Functionalities, BER_CLASS_CON, 5, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_OfferedCamel4Functionalities },
  { &hf_camel_bearerCapability2, BER_CLASS_CON, 6, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_BearerCapability },
  { &hf_camel_ext_basicServiceCode2, BER_CLASS_CON, 7, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_Ext_BasicServiceCode },
  { &hf_camel_highLayerCompatibility2, BER_CLASS_CON, 8, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_HighLayerCompatibility },
  { &hf_camel_lowLayerCompatibility, BER_CLASS_CON, 9, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_LowLayerCompatibility },
  { &hf_camel_lowLayerCompatibility2, BER_CLASS_CON, 10, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_LowLayerCompatibility },
  { &hf_camel_enhancedDialledServicesAllowed, BER_CLASS_CON, 11, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_uu_Data       , BER_CLASS_CON, 12, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ch_UU_Data },
  { &hf_camel_collectInformationAllowed, BER_CLASS_CON, 13, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_releaseCallArgExtensionAllowed, BER_CLASS_CON, 14, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_InitialDPArgExtension(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {

if((camel_ver == 2)||(camel_ver == 1)){
	return dissect_camel_InitialDPArgExtensionV2(implicit_tag, tvb, offset, actx, tree, hf_index);
}

  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   InitialDPArgExtension_sequence, hf_index, ett_camel_InitialDPArgExtension);



  return offset;
}


static const ber_sequence_t InitialDPArg_sequence[] = {
  { &hf_camel_serviceKey    , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_inap_ServiceKey },
  { &hf_camel_calledPartyNumber, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CalledPartyNumber },
  { &hf_camel_callingPartyNumber, BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CallingPartyNumber },
  { &hf_camel_callingPartysCategory, BER_CLASS_CON, 5, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_CallingPartysCategory },
  { &hf_camel_cGEncountered , BER_CLASS_CON, 7, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CGEncountered },
  { &hf_camel_iPSSPCapabilities, BER_CLASS_CON, 8, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_IPSSPCapabilities },
  { &hf_camel_locationNumber, BER_CLASS_CON, 10, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_LocationNumber },
  { &hf_camel_originalCalledPartyID, BER_CLASS_CON, 12, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_OriginalCalledPartyID },
  { &hf_camel_extensions    , BER_CLASS_CON, 15, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { &hf_camel_highLayerCompatibility, BER_CLASS_CON, 23, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_HighLayerCompatibility },
  { &hf_camel_additionalCallingPartyNumber, BER_CLASS_CON, 25, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_AdditionalCallingPartyNumber },
  { &hf_camel_bearerCapability, BER_CLASS_CON, 27, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_BearerCapability },
  { &hf_camel_eventTypeBCSM , BER_CLASS_CON, 28, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_EventTypeBCSM },
  { &hf_camel_redirectingPartyID, BER_CLASS_CON, 29, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_RedirectingPartyID },
  { &hf_camel_redirectionInformation, BER_CLASS_CON, 30, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_RedirectionInformation },
  { &hf_camel_cause         , BER_CLASS_CON, 17, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Cause },
  { &hf_camel_serviceInteractionIndicatorsTwo, BER_CLASS_CON, 32, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ServiceInteractionIndicatorsTwo },
  { &hf_camel_carrier       , BER_CLASS_CON, 37, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Carrier },
  { &hf_camel_cug_Index     , BER_CLASS_CON, 45, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_CUG_Index },
  { &hf_camel_cug_Interlock , BER_CLASS_CON, 46, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_CUG_Interlock },
  { &hf_camel_cug_OutgoingAccess, BER_CLASS_CON, 47, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_iMSI          , BER_CLASS_CON, 50, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_IMSI },
  { &hf_camel_subscriberState, BER_CLASS_CON, 51, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_SubscriberState },
  { &hf_camel_locationInformation, BER_CLASS_CON, 52, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_LocationInformation },
  { &hf_camel_ext_basicServiceCode, BER_CLASS_CON, 53, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_Ext_BasicServiceCode },
  { &hf_camel_callReferenceNumber, BER_CLASS_CON, 54, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ch_CallReferenceNumber },
  { &hf_camel_mscAddress    , BER_CLASS_CON, 55, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ISDN_AddressString },
  { &hf_camel_calledPartyBCDNumber, BER_CLASS_CON, 56, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CalledPartyBCDNumber },
  { &hf_camel_timeAndTimezone, BER_CLASS_CON, 57, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_TimeAndTimezone },
  { &hf_camel_callForwardingSS_Pending, BER_CLASS_CON, 58, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_initialDPArgExtension, BER_CLASS_CON, 59, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_InitialDPArgExtension },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_InitialDPArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   InitialDPArg_sequence, hf_index, ett_camel_InitialDPArg);

  return offset;
}


static const ber_sequence_t InitiateCallAttemptArg_sequence[] = {
  { &hf_camel_destinationRoutingAddress, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_DestinationRoutingAddress },
  { &hf_camel_extensions    , BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { &hf_camel_legToBeCreated, BER_CLASS_CON, 5, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_LegID },
  { &hf_camel_newCallSegment, BER_CLASS_CON, 6, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CallSegmentID },
  { &hf_camel_callingPartyNumber, BER_CLASS_CON, 30, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CallingPartyNumber },
  { &hf_camel_callReferenceNumber, BER_CLASS_CON, 51, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ch_CallReferenceNumber },
  { &hf_camel_gsmSCFAddress , BER_CLASS_CON, 52, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ISDN_AddressString },
  { &hf_camel_suppress_T_CSI, BER_CLASS_CON, 53, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_InitiateCallAttemptArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   InitiateCallAttemptArg_sequence, hf_index, ett_camel_InitiateCallAttemptArg);

  return offset;
}


static const ber_sequence_t InitiateCallAttemptRes_sequence[] = {
  { &hf_camel_supportedCamelPhases, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_SupportedCamelPhases },
  { &hf_camel_offeredCamel4Functionalities, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_OfferedCamel4Functionalities },
  { &hf_camel_extensions    , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { &hf_camel_releaseCallArgExtensionAllowed, BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_InitiateCallAttemptRes(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   InitiateCallAttemptRes_sequence, hf_index, ett_camel_InitiateCallAttemptRes);

  return offset;
}


static const ber_sequence_t MoveLegArg_sequence[] = {
  { &hf_camel_legIDToMove   , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_inap_LegID },
  { &hf_camel_extensions    , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_MoveLegArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   MoveLegArg_sequence, hf_index, ett_camel_MoveLegArg);

  return offset;
}


static const ber_sequence_t PlayToneArg_sequence[] = {
  { &hf_camel_legOrCallSegment, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_LegOrCallSegment },
  { &hf_camel_bursts        , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_Burst },
  { &hf_camel_extensions    , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_PlayToneArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   PlayToneArg_sequence, hf_index, ett_camel_PlayToneArg);

  return offset;
}



static int
dissect_camel_AllCallSegments(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_camel_Cause(implicit_tag, tvb, offset, actx, tree, hf_index);

  return offset;
}


static const ber_sequence_t AllCallSegmentsWithExtension_sequence[] = {
  { &hf_camel_allCallSegments, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_AllCallSegments },
  { &hf_camel_extensions    , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_AllCallSegmentsWithExtension(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   AllCallSegmentsWithExtension_sequence, hf_index, ett_camel_AllCallSegmentsWithExtension);

  return offset;
}


static const value_string camel_ReleaseCallArg_vals[] = {
  {   0, "allCallSegments" },
  {   1, "allCallSegmentsWithExtension" },
  { 0, NULL }
};

static const ber_choice_t ReleaseCallArg_choice[] = {
  {   0, &hf_camel_allCallSegments, BER_CLASS_UNI, BER_UNI_TAG_OCTETSTRING, BER_FLAGS_NOOWNTAG, dissect_camel_AllCallSegments },
  {   1, &hf_camel_allCallSegmentsWithExtension, BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_camel_AllCallSegmentsWithExtension },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ReleaseCallArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 ReleaseCallArg_choice, hf_index, ett_camel_ReleaseCallArg,
                                 NULL);

  return offset;
}


static const ber_sequence_t SEQUENCE_SIZE_1_bound__numOfBCSMEvents_OF_BCSMEvent_sequence_of[1] = {
  { &hf_camel_bcsmEvents_item, BER_CLASS_UNI, BER_UNI_TAG_SEQUENCE, BER_FLAGS_NOOWNTAG, dissect_camel_BCSMEvent },
};

static int
dissect_camel_SEQUENCE_SIZE_1_bound__numOfBCSMEvents_OF_BCSMEvent(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence_of(implicit_tag, actx, tree, tvb, offset,
                                      SEQUENCE_SIZE_1_bound__numOfBCSMEvents_OF_BCSMEvent_sequence_of, hf_index, ett_camel_SEQUENCE_SIZE_1_bound__numOfBCSMEvents_OF_BCSMEvent);

  return offset;
}


static const ber_sequence_t RequestReportBCSMEventArg_sequence[] = {
  { &hf_camel_bcsmEvents    , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_SEQUENCE_SIZE_1_bound__numOfBCSMEvents_OF_BCSMEvent },
  { &hf_camel_extensions    , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_RequestReportBCSMEventArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   RequestReportBCSMEventArg_sequence, hf_index, ett_camel_RequestReportBCSMEventArg);

  return offset;
}


static const ber_sequence_t ResetTimerArg_sequence[] = {
  { &hf_camel_timerID       , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_TimerID },
  { &hf_camel_timervalue    , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_TimerValue },
  { &hf_camel_extensions    , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { &hf_camel_callSegmentID , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CallSegmentID },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ResetTimerArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ResetTimerArg_sequence, hf_index, ett_camel_ResetTimerArg);

  return offset;
}


static const ber_sequence_t SendChargingInformationArg_sequence[] = {
  { &hf_camel_sCIBillingChargingCharacteristics, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_SCIBillingChargingCharacteristics },
  { &hf_camel_partyToCharge , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_SendingSideID },
  { &hf_camel_extensions    , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_SendChargingInformationArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   SendChargingInformationArg_sequence, hf_index, ett_camel_SendChargingInformationArg);

  return offset;
}


static const ber_sequence_t SplitLegArg_sequence[] = {
  { &hf_camel_legToBeSplit  , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_inap_LegID },
  { &hf_camel_newCallSegment, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CallSegmentID },
  { &hf_camel_extensions    , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_SplitLegArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   SplitLegArg_sequence, hf_index, ett_camel_SplitLegArg);

  return offset;
}


static const ber_sequence_t ApplyChargingGPRSArg_sequence[] = {
  { &hf_camel_chargingCharacteristics, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_ChargingCharacteristics },
  { &hf_camel_applyChargingGPRS_tariffSwitchInterval, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_86400 },
  { &hf_camel_pDPID         , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_PDPID },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ApplyChargingGPRSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ApplyChargingGPRSArg_sequence, hf_index, ett_camel_ApplyChargingGPRSArg);

  return offset;
}


static const ber_sequence_t ApplyChargingReportGPRSArg_sequence[] = {
  { &hf_camel_chargingResult, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_ChargingResult },
  { &hf_camel_qualityOfService, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_QualityOfService },
  { &hf_camel_active        , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_BOOLEAN },
  { &hf_camel_pDPID         , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_PDPID },
  { &hf_camel_chargingRollOver, BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_ChargingRollOver },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ApplyChargingReportGPRSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ApplyChargingReportGPRSArg_sequence, hf_index, ett_camel_ApplyChargingReportGPRSArg);

  return offset;
}


static const ber_sequence_t CancelGPRSArg_sequence[] = {
  { &hf_camel_pDPID         , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_PDPID },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CancelGPRSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   CancelGPRSArg_sequence, hf_index, ett_camel_CancelGPRSArg);

  return offset;
}


static const ber_sequence_t ConnectGPRSArg_sequence[] = {
  { &hf_camel_accessPointName, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_AccessPointName },
  { &hf_camel_pdpID         , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_PDPID },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ConnectGPRSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ConnectGPRSArg_sequence, hf_index, ett_camel_ConnectGPRSArg);

  return offset;
}


static const ber_sequence_t ContinueGPRSArg_sequence[] = {
  { &hf_camel_pDPID         , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_PDPID },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ContinueGPRSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ContinueGPRSArg_sequence, hf_index, ett_camel_ContinueGPRSArg);

  return offset;
}


static const ber_sequence_t EntityReleasedGPRSArg_sequence[] = {
  { &hf_camel_gPRSCause     , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_GPRSCause },
  { &hf_camel_pDPID         , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_PDPID },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_EntityReleasedGPRSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   EntityReleasedGPRSArg_sequence, hf_index, ett_camel_EntityReleasedGPRSArg);

  return offset;
}


static const ber_sequence_t EventReportGPRSArg_sequence[] = {
  { &hf_camel_gPRSEventType , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_GPRSEventType },
  { &hf_camel_miscGPRSInfo  , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_MiscCallInfo },
  { &hf_camel_gPRSEventSpecificInformation, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_GPRSEventSpecificInformation },
  { &hf_camel_pDPID         , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_PDPID },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_EventReportGPRSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   EventReportGPRSArg_sequence, hf_index, ett_camel_EventReportGPRSArg);

  return offset;
}



static int
dissect_camel_FurnishChargingInformationGPRSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_camel_FCIGPRSBillingChargingCharacteristics(implicit_tag, tvb, offset, actx, tree, hf_index);

  return offset;
}


static const ber_sequence_t InitialDPGPRSArg_sequence[] = {
  { &hf_camel_serviceKey    , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_inap_ServiceKey },
  { &hf_camel_gPRSEventType , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_GPRSEventType },
  { &hf_camel_mSISDN        , BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_gsm_map_ISDN_AddressString },
  { &hf_camel_iMSI          , BER_CLASS_CON, 3, BER_FLAGS_IMPLTAG, dissect_gsm_map_IMSI },
  { &hf_camel_timeAndTimeZone, BER_CLASS_CON, 4, BER_FLAGS_IMPLTAG, dissect_camel_TimeAndTimezone },
  { &hf_camel_gPRSMSClass   , BER_CLASS_CON, 5, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_GPRSMSClass },
  { &hf_camel_endUserAddress, BER_CLASS_CON, 6, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_EndUserAddress },
  { &hf_camel_qualityOfService, BER_CLASS_CON, 7, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_QualityOfService },
  { &hf_camel_accessPointName, BER_CLASS_CON, 8, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_AccessPointName },
  { &hf_camel_routeingAreaIdentity, BER_CLASS_CON, 9, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_RAIdentity },
  { &hf_camel_chargingID    , BER_CLASS_CON, 10, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_GPRSChargingID },
  { &hf_camel_sGSNCapabilities, BER_CLASS_CON, 11, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_SGSNCapabilities },
  { &hf_camel_locationInformationGPRS, BER_CLASS_CON, 12, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_LocationInformationGPRS },
  { &hf_camel_pDPInitiationType, BER_CLASS_CON, 13, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_PDPInitiationType },
  { &hf_camel_extensions    , BER_CLASS_CON, 14, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { &hf_camel_gGSNAddress   , BER_CLASS_CON, 15, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_GSN_Address },
  { &hf_camel_secondaryPDP_context, BER_CLASS_CON, 16, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { &hf_camel_iMEI          , BER_CLASS_CON, 17, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_IMEI },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_InitialDPGPRSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   InitialDPGPRSArg_sequence, hf_index, ett_camel_InitialDPGPRSArg);

  return offset;
}


static const ber_sequence_t ReleaseGPRSArg_sequence[] = {
  { &hf_camel_gprsCause     , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_GPRSCause },
  { &hf_camel_pDPID         , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_PDPID },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ReleaseGPRSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ReleaseGPRSArg_sequence, hf_index, ett_camel_ReleaseGPRSArg);

  return offset;
}


static const ber_sequence_t SEQUENCE_SIZE_1_bound__numOfGPRSEvents_OF_GPRSEvent_sequence_of[1] = {
  { &hf_camel_gPRSEvent_item, BER_CLASS_UNI, BER_UNI_TAG_SEQUENCE, BER_FLAGS_NOOWNTAG, dissect_camel_GPRSEvent },
};

static int
dissect_camel_SEQUENCE_SIZE_1_bound__numOfGPRSEvents_OF_GPRSEvent(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence_of(implicit_tag, actx, tree, tvb, offset,
                                      SEQUENCE_SIZE_1_bound__numOfGPRSEvents_OF_GPRSEvent_sequence_of, hf_index, ett_camel_SEQUENCE_SIZE_1_bound__numOfGPRSEvents_OF_GPRSEvent);

  return offset;
}


static const ber_sequence_t RequestReportGPRSEventArg_sequence[] = {
  { &hf_camel_gPRSEvent     , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_SEQUENCE_SIZE_1_bound__numOfGPRSEvents_OF_GPRSEvent },
  { &hf_camel_pDPID         , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_PDPID },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_RequestReportGPRSEventArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   RequestReportGPRSEventArg_sequence, hf_index, ett_camel_RequestReportGPRSEventArg);

  return offset;
}


static const ber_sequence_t ResetTimerGPRSArg_sequence[] = {
  { &hf_camel_timerID       , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_TimerID },
  { &hf_camel_timervalue    , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_TimerValue },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ResetTimerGPRSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ResetTimerGPRSArg_sequence, hf_index, ett_camel_ResetTimerGPRSArg);

  return offset;
}


static const ber_sequence_t SendChargingInformationGPRSArg_sequence[] = {
  { &hf_camel_sCIGPRSBillingChargingCharacteristics, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_SCIGPRSBillingChargingCharacteristics },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_SendChargingInformationGPRSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   SendChargingInformationGPRSArg_sequence, hf_index, ett_camel_SendChargingInformationGPRSArg);

  return offset;
}


static const ber_sequence_t ConnectSMSArg_sequence[] = {
  { &hf_camel_callingPartysNumber, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_SMS_AddressString },
  { &hf_camel_destinationSubscriberNumber, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CalledPartyBCDNumber },
  { &hf_camel_sMSCAddress   , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ISDN_AddressString },
  { &hf_camel_extensions    , BER_CLASS_CON, 10, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ConnectSMSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ConnectSMSArg_sequence, hf_index, ett_camel_ConnectSMSArg);

  return offset;
}


static const ber_sequence_t EventReportSMSArg_sequence[] = {
  { &hf_camel_eventTypeSMS  , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_EventTypeSMS },
  { &hf_camel_eventSpecificInformationSMS, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_EventSpecificInformationSMS },
  { &hf_camel_miscCallInfo  , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_inap_MiscCallInfo },
  { &hf_camel_extensions    , BER_CLASS_CON, 10, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_EventReportSMSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   EventReportSMSArg_sequence, hf_index, ett_camel_EventReportSMSArg);

  return offset;
}



static int
dissect_camel_FurnishChargingInformationSMSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_camel_FCISMSBillingChargingCharacteristics(implicit_tag, tvb, offset, actx, tree, hf_index);

  return offset;
}


static const ber_sequence_t InitialDPSMSArg_sequence[] = {
  { &hf_camel_serviceKey    , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_inap_ServiceKey },
  { &hf_camel_destinationSubscriberNumber, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CalledPartyBCDNumber },
  { &hf_camel_callingPartyNumber_01, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_SMS_AddressString },
  { &hf_camel_eventTypeSMS  , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_EventTypeSMS },
  { &hf_camel_iMSI          , BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_IMSI },
  { &hf_camel_locationInformationMSC, BER_CLASS_CON, 5, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_LocationInformation },
  { &hf_camel_locationInformationGPRS, BER_CLASS_CON, 6, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_LocationInformationGPRS },
  { &hf_camel_sMSCAddress   , BER_CLASS_CON, 7, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ISDN_AddressString },
  { &hf_camel_timeAndTimezone, BER_CLASS_CON, 8, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_TimeAndTimezone },
  { &hf_camel_tPShortMessageSpecificInfo, BER_CLASS_CON, 9, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_TPShortMessageSpecificInfo },
  { &hf_camel_tPProtocolIdentifier, BER_CLASS_CON, 10, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_TPProtocolIdentifier },
  { &hf_camel_tPDataCodingScheme, BER_CLASS_CON, 11, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_TPDataCodingScheme },
  { &hf_camel_tPValidityPeriod, BER_CLASS_CON, 12, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_TPValidityPeriod },
  { &hf_camel_extensions    , BER_CLASS_CON, 13, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { &hf_camel_smsReferenceNumber, BER_CLASS_CON, 14, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ch_CallReferenceNumber },
  { &hf_camel_mscAddress    , BER_CLASS_CON, 15, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ISDN_AddressString },
  { &hf_camel_sgsn_Number   , BER_CLASS_CON, 16, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ISDN_AddressString },
  { &hf_camel_ms_Classmark2 , BER_CLASS_CON, 17, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_MS_Classmark2 },
  { &hf_camel_gPRSMSClass   , BER_CLASS_CON, 18, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ms_GPRSMSClass },
  { &hf_camel_iMEI          , BER_CLASS_CON, 19, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_IMEI },
  { &hf_camel_calledPartyNumber_01, BER_CLASS_CON, 20, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ISDN_AddressString },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_InitialDPSMSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   InitialDPSMSArg_sequence, hf_index, ett_camel_InitialDPSMSArg);

  return offset;
}



static int
dissect_camel_ReleaseSMSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_camel_RPCause(implicit_tag, tvb, offset, actx, tree, hf_index);

  return offset;
}


static const ber_sequence_t SEQUENCE_SIZE_1_bound__numOfSMSEvents_OF_SMSEvent_sequence_of[1] = {
  { &hf_camel_sMSEvents_item, BER_CLASS_UNI, BER_UNI_TAG_SEQUENCE, BER_FLAGS_NOOWNTAG, dissect_camel_SMSEvent },
};

static int
dissect_camel_SEQUENCE_SIZE_1_bound__numOfSMSEvents_OF_SMSEvent(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence_of(implicit_tag, actx, tree, tvb, offset,
                                      SEQUENCE_SIZE_1_bound__numOfSMSEvents_OF_SMSEvent_sequence_of, hf_index, ett_camel_SEQUENCE_SIZE_1_bound__numOfSMSEvents_OF_SMSEvent);

  return offset;
}


static const ber_sequence_t RequestReportSMSEventArg_sequence[] = {
  { &hf_camel_sMSEvents     , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_SEQUENCE_SIZE_1_bound__numOfSMSEvents_OF_SMSEvent },
  { &hf_camel_extensions    , BER_CLASS_CON, 10, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_RequestReportSMSEventArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   RequestReportSMSEventArg_sequence, hf_index, ett_camel_RequestReportSMSEventArg);

  return offset;
}


static const ber_sequence_t ResetTimerSMSArg_sequence[] = {
  { &hf_camel_timerID       , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_TimerID },
  { &hf_camel_timervalue    , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_TimerValue },
  { &hf_camel_extensions    , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_Extensions },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ResetTimerSMSArg(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ResetTimerSMSArg_sequence, hf_index, ett_camel_ResetTimerSMSArg);

  return offset;
}


static const value_string camel_CAP_U_ABORT_REASON_vals[] = {
  {   1, "no-reason-given" },
  {   2, "application-timer-expired" },
  {   3, "not-allowed-procedures" },
  {   4, "abnormal-processing" },
  {   5, "congestion" },
  {   6, "invalid-reference" },
  {   7, "missing-reference" },
  {   8, "overlapping-dialogue" },
  { 0, NULL }
};


static int
dissect_camel_CAP_U_ABORT_REASON(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}


static const ber_sequence_t SEQUENCE_SIZE_1_numOfExtensions_OF_ExtensionField_sequence_of[1] = {
  { &hf_camel_extensions_item, BER_CLASS_UNI, BER_UNI_TAG_SEQUENCE, BER_FLAGS_NOOWNTAG, dissect_camel_ExtensionField },
};

static int
dissect_camel_SEQUENCE_SIZE_1_numOfExtensions_OF_ExtensionField(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence_of(implicit_tag, actx, tree, tvb, offset,
                                      SEQUENCE_SIZE_1_numOfExtensions_OF_ExtensionField_sequence_of, hf_index, ett_camel_SEQUENCE_SIZE_1_numOfExtensions_OF_ExtensionField);

  return offset;
}



static int
dissect_camel_NACarrierSelectionInfo(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}


static const ber_sequence_t NACarrierInformation_sequence[] = {
  { &hf_camel_naCarrierId   , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_NAEA_CIC },
  { &hf_camel_naCICSelectionType, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NACarrierSelectionInfo },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_NACarrierInformation(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   NACarrierInformation_sequence, hf_index, ett_camel_NACarrierInformation);

  return offset;
}



static int
dissect_camel_NAChargeNumber(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}


static const ber_sequence_t NA_Info_sequence[] = {
  { &hf_camel_naCarrierInformation, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NACarrierInformation },
  { &hf_camel_naOliInfo     , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NAOliInfo },
  { &hf_camel_naChargeNumber, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NAChargeNumber },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_NA_Info(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   NA_Info_sequence, hf_index, ett_camel_NA_Info);

  return offset;
}


static const ber_sequence_t EstablishTemporaryConnectionArgV2_sequence[] = {
  { &hf_camel_assistingSSPIPRoutingAddress, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_AssistingSSPIPRoutingAddress },
  { &hf_camel_correlationID , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_CorrelationID },
  { &hf_camel_scfID         , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ScfID },
  { &hf_camel_extensions_01 , BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_SEQUENCE_SIZE_1_numOfExtensions_OF_ExtensionField },
  { &hf_camel_serviceInteractionIndicatorsTwo, BER_CLASS_CON, 7, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ServiceInteractionIndicatorsTwo },
  { &hf_camel_na_info       , BER_CLASS_CON, 50, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NA_Info },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_EstablishTemporaryConnectionArgV2(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   EstablishTemporaryConnectionArgV2_sequence, hf_index, ett_camel_EstablishTemporaryConnectionArgV2);

  return offset;
}


static const ber_sequence_t InitialDPArgExtensionV2_sequence[] = {
  { &hf_camel_naCarrierInformation, BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_NACarrierInformation },
  { &hf_camel_gmscAddress   , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_gsm_map_ISDN_AddressString },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_InitialDPArgExtensionV2(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   InitialDPArgExtensionV2_sequence, hf_index, ett_camel_InitialDPArgExtensionV2);

  return offset;
}


static const ber_sequence_t ReleaseIfDurationExceeded_sequence[] = {
  { &hf_camel_tone_01       , BER_CLASS_UNI, BER_UNI_TAG_BOOLEAN, BER_FLAGS_OPTIONAL|BER_FLAGS_NOOWNTAG, dissect_camel_BOOLEAN },
  { &hf_camel_extensions_01 , BER_CLASS_CON, 10, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_SEQUENCE_SIZE_1_numOfExtensions_OF_ExtensionField },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ReleaseIfDurationExceeded(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ReleaseIfDurationExceeded_sequence, hf_index, ett_camel_ReleaseIfDurationExceeded);

  return offset;
}


static const ber_sequence_t T_timeDurationCharging_01_sequence[] = {
  { &hf_camel_maxCallPeriodDuration, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_864000 },
  { &hf_camel_releaseIfdurationExceeded_01, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_ReleaseIfDurationExceeded },
  { &hf_camel_tariffSwitchInterval, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL|BER_FLAGS_IMPLTAG, dissect_camel_INTEGER_1_86400 },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_timeDurationCharging_01(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_timeDurationCharging_01_sequence, hf_index, ett_camel_T_timeDurationCharging_01);

  return offset;
}


static const ber_choice_t CAMEL_AChBillingChargingCharacteristicsV2_choice[] = {
  {   0, &hf_camel_timeDurationCharging_01, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_T_timeDurationCharging_01 },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_CAMEL_AChBillingChargingCharacteristicsV2(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 CAMEL_AChBillingChargingCharacteristicsV2_choice, hf_index, ett_camel_CAMEL_AChBillingChargingCharacteristicsV2,
                                 NULL);

  return offset;
}



static int
dissect_camel_SpecializedResourceReportArgV23(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_null(implicit_tag, actx, tree, tvb, offset, hf_index);

  return offset;
}



static int
dissect_camel_INTEGER(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const value_string camel_InvokeId_vals[] = {
  {   0, "present" },
  {   1, "absent" },
  { 0, NULL }
};

static const ber_choice_t InvokeId_choice[] = {
  {   0, &hf_camel_present       , BER_CLASS_UNI, BER_UNI_TAG_INTEGER, BER_FLAGS_NOOWNTAG, dissect_camel_INTEGER },
  {   1, &hf_camel_absent        , BER_CLASS_UNI, BER_UNI_TAG_NULL, BER_FLAGS_NOOWNTAG, dissect_camel_NULL },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_InvokeId(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 InvokeId_choice, hf_index, ett_camel_InvokeId,
                                 NULL);

  return offset;
}



static int
dissect_camel_InvokeId_present(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}



static int
dissect_camel_T_linkedIdPresent(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_camel_InvokeId_present(implicit_tag, tvb, offset, actx, tree, hf_index);

  return offset;
}


static const value_string camel_T_linkedId_vals[] = {
  {   0, "present" },
  {   1, "absent" },
  { 0, NULL }
};

static const ber_choice_t T_linkedId_choice[] = {
  {   0, &hf_camel_linkedIdPresent, BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_T_linkedIdPresent },
  {   1, &hf_camel_absent        , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_NULL },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_linkedId(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 T_linkedId_choice, hf_index, ett_camel_T_linkedId,
                                 NULL);

  return offset;
}



static int
dissect_camel_T_argument(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
	offset = dissect_invokeData(tree, tvb, offset, actx);


  return offset;
}


static const ber_sequence_t Invoke_sequence[] = {
  { &hf_camel_invokeId      , BER_CLASS_ANY/*choice*/, -1/*choice*/, BER_FLAGS_NOOWNTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_InvokeId },
  { &hf_camel_linkedId      , BER_CLASS_ANY/*choice*/, -1/*choice*/, BER_FLAGS_OPTIONAL|BER_FLAGS_NOOWNTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_T_linkedId },
  { &hf_camel_opcode        , BER_CLASS_ANY/*choice*/, -1/*choice*/, BER_FLAGS_NOOWNTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_Code },
  { &hf_camel_argument      , BER_CLASS_ANY, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_NOOWNTAG, dissect_camel_T_argument },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_Invoke(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  camel_opcode_type=CAMEL_OPCODE_INVOKE;

  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   Invoke_sequence, hf_index, ett_camel_Invoke);

  return offset;
}



static int
dissect_camel_ResultArgument(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
	offset = dissect_returnResultData(tree, tvb, offset, actx);


  return offset;
}


static const ber_sequence_t T_result_sequence[] = {
  { &hf_camel_opcode        , BER_CLASS_ANY/*choice*/, -1/*choice*/, BER_FLAGS_NOOWNTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_Code },
  { &hf_camel_resultArgument, BER_CLASS_ANY, 0, BER_FLAGS_NOOWNTAG, dissect_camel_ResultArgument },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_result(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   T_result_sequence, hf_index, ett_camel_T_result);

  return offset;
}


static const ber_sequence_t ReturnResult_sequence[] = {
  { &hf_camel_invokeId      , BER_CLASS_ANY/*choice*/, -1/*choice*/, BER_FLAGS_NOOWNTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_InvokeId },
  { &hf_camel_result        , BER_CLASS_UNI, BER_UNI_TAG_SEQUENCE, BER_FLAGS_OPTIONAL|BER_FLAGS_NOOWNTAG, dissect_camel_T_result },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ReturnResult(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  camel_opcode_type=CAMEL_OPCODE_RETURN_RESULT;

  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ReturnResult_sequence, hf_index, ett_camel_ReturnResult);

  return offset;
}



static int
dissect_camel_T_parameter(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
	offset = dissect_returnErrorData(tree, tvb, offset, actx);



  return offset;
}


static const ber_sequence_t ReturnError_sequence[] = {
  { &hf_camel_invokeId      , BER_CLASS_ANY/*choice*/, -1/*choice*/, BER_FLAGS_NOOWNTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_InvokeId },
  { &hf_camel_errcode       , BER_CLASS_ANY/*choice*/, -1/*choice*/, BER_FLAGS_NOOWNTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_Code },
  { &hf_camel_parameter     , BER_CLASS_ANY, 0, BER_FLAGS_OPTIONAL|BER_FLAGS_NOOWNTAG, dissect_camel_T_parameter },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ReturnError(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  camel_opcode_type=CAMEL_OPCODE_RETURN_ERROR;

  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   ReturnError_sequence, hf_index, ett_camel_ReturnError);

  return offset;
}


static const value_string camel_GeneralProblem_vals[] = {
  {   0, "unrecognizedPDU" },
  {   1, "mistypedPDU" },
  {   2, "badlyStructuredPDU" },
  { 0, NULL }
};


static int
dissect_camel_GeneralProblem(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const value_string camel_InvokeProblem_vals[] = {
  {   0, "duplicateInvocation" },
  {   1, "unrecognizedOperation" },
  {   2, "mistypedArgument" },
  {   3, "resourceLimitation" },
  {   4, "releaseInProgress" },
  {   5, "unrecognizedLinkedId" },
  {   6, "linkedResponseUnexpected" },
  {   7, "unexpectedLinkedOperation" },
  { 0, NULL }
};


static int
dissect_camel_InvokeProblem(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const value_string camel_ReturnResultProblem_vals[] = {
  {   0, "unrecognizedInvocation" },
  {   1, "resultResponseUnexpected" },
  {   2, "mistypedResult" },
  { 0, NULL }
};


static int
dissect_camel_ReturnResultProblem(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const value_string camel_ReturnErrorProblem_vals[] = {
  {   0, "unrecognizedInvocation" },
  {   1, "errorResponseUnexpected" },
  {   2, "unrecognizedError" },
  {   3, "unexpectedError" },
  {   4, "mistypedParameter" },
  { 0, NULL }
};


static int
dissect_camel_ReturnErrorProblem(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                                NULL);

  return offset;
}


static const value_string camel_T_problem_vals[] = {
  {   0, "general" },
  {   1, "invoke" },
  {   2, "returnResult" },
  {   3, "returnError" },
  { 0, NULL }
};

static const ber_choice_t T_problem_choice[] = {
  {   0, &hf_camel_general       , BER_CLASS_CON, 0, BER_FLAGS_IMPLTAG, dissect_camel_GeneralProblem },
  {   1, &hf_camel_invokeProblem , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_InvokeProblem },
  {   2, &hf_camel_problemReturnResult, BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_camel_ReturnResultProblem },
  {   3, &hf_camel_returnErrorProblem, BER_CLASS_CON, 3, BER_FLAGS_IMPLTAG, dissect_camel_ReturnErrorProblem },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_T_problem(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 T_problem_choice, hf_index, ett_camel_T_problem,
                                 NULL);

  return offset;
}


static const ber_sequence_t Reject_sequence[] = {
  { &hf_camel_invokeId      , BER_CLASS_ANY/*choice*/, -1/*choice*/, BER_FLAGS_NOOWNTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_InvokeId },
  { &hf_camel_problem       , BER_CLASS_ANY/*choice*/, -1/*choice*/, BER_FLAGS_NOOWNTAG|BER_FLAGS_NOTCHKTAG, dissect_camel_T_problem },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_Reject(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  camel_opcode_type=CAMEL_OPCODE_REJECT;


  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   Reject_sequence, hf_index, ett_camel_Reject);

  return offset;
}


static const ber_choice_t ROS_choice[] = {
  {   1, &hf_camel_invoke        , BER_CLASS_CON, 1, BER_FLAGS_IMPLTAG, dissect_camel_Invoke },
  {   2, &hf_camel_returnResult  , BER_CLASS_CON, 2, BER_FLAGS_IMPLTAG, dissect_camel_ReturnResult },
  {   3, &hf_camel_returnError   , BER_CLASS_CON, 3, BER_FLAGS_IMPLTAG, dissect_camel_ReturnError },
  {   4, &hf_camel_reject        , BER_CLASS_CON, 4, BER_FLAGS_IMPLTAG, dissect_camel_Reject },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_camel_ROS(bool implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 ROS_choice, hf_index, ett_camel_ROS,
                                 NULL);

  return offset;
}

/*--- PDUs ---*/

static int dissect_PAR_cancelFailed_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_PAR_cancelFailed(false, tvb, offset, &asn1_ctx, tree, hf_camel_PAR_cancelFailed_PDU);
  return offset;
}
static int dissect_PAR_requestedInfoError_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_PAR_requestedInfoError(false, tvb, offset, &asn1_ctx, tree, hf_camel_PAR_requestedInfoError_PDU);
  return offset;
}
static int dissect_UnavailableNetworkResource_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_UnavailableNetworkResource(false, tvb, offset, &asn1_ctx, tree, hf_camel_UnavailableNetworkResource_PDU);
  return offset;
}
static int dissect_PAR_taskRefused_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_PAR_taskRefused(false, tvb, offset, &asn1_ctx, tree, hf_camel_PAR_taskRefused_PDU);
  return offset;
}
static int dissect_CAP_GPRS_ReferenceNumber_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_CAP_GPRS_ReferenceNumber(false, tvb, offset, &asn1_ctx, tree, hf_camel_CAP_GPRS_ReferenceNumber_PDU);
  return offset;
}
static int dissect_PlayAnnouncementArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_PlayAnnouncementArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_PlayAnnouncementArg_PDU);
  return offset;
}
static int dissect_PromptAndCollectUserInformationArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_PromptAndCollectUserInformationArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_PromptAndCollectUserInformationArg_PDU);
  return offset;
}
static int dissect_ReceivedInformationArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_ReceivedInformationArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_ReceivedInformationArg_PDU);
  return offset;
}
static int dissect_SpecializedResourceReportArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_SpecializedResourceReportArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_SpecializedResourceReportArg_PDU);
  return offset;
}
static int dissect_ApplyChargingArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_ApplyChargingArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_ApplyChargingArg_PDU);
  return offset;
}
static int dissect_ApplyChargingReportArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_ApplyChargingReportArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_ApplyChargingReportArg_PDU);
  return offset;
}
static int dissect_AssistRequestInstructionsArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_AssistRequestInstructionsArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_AssistRequestInstructionsArg_PDU);
  return offset;
}
static int dissect_CallGapArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_CallGapArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_CallGapArg_PDU);
  return offset;
}
static int dissect_CallInformationReportArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_CallInformationReportArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_CallInformationReportArg_PDU);
  return offset;
}
static int dissect_CallInformationRequestArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_CallInformationRequestArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_CallInformationRequestArg_PDU);
  return offset;
}
static int dissect_CancelArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_CancelArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_CancelArg_PDU);
  return offset;
}
static int dissect_CollectInformationArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_CollectInformationArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_CollectInformationArg_PDU);
  return offset;
}
static int dissect_ConnectArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_ConnectArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_ConnectArg_PDU);
  return offset;
}
static int dissect_ConnectToResourceArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_ConnectToResourceArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_ConnectToResourceArg_PDU);
  return offset;
}
static int dissect_ContinueWithArgumentArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_ContinueWithArgumentArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_ContinueWithArgumentArg_PDU);
  return offset;
}
static int dissect_DisconnectForwardConnectionWithArgumentArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_DisconnectForwardConnectionWithArgumentArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_DisconnectForwardConnectionWithArgumentArg_PDU);
  return offset;
}
static int dissect_DisconnectLegArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_DisconnectLegArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_DisconnectLegArg_PDU);
  return offset;
}
static int dissect_EntityReleasedArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_EntityReleasedArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_EntityReleasedArg_PDU);
  return offset;
}
static int dissect_EstablishTemporaryConnectionArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_EstablishTemporaryConnectionArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_EstablishTemporaryConnectionArg_PDU);
  return offset;
}
static int dissect_EventReportBCSMArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_EventReportBCSMArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_EventReportBCSMArg_PDU);
  return offset;
}
static int dissect_FurnishChargingInformationArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_FurnishChargingInformationArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_FurnishChargingInformationArg_PDU);
  return offset;
}
static int dissect_InitialDPArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_InitialDPArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_InitialDPArg_PDU);
  return offset;
}
static int dissect_InitiateCallAttemptArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_InitiateCallAttemptArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_InitiateCallAttemptArg_PDU);
  return offset;
}
static int dissect_InitiateCallAttemptRes_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_InitiateCallAttemptRes(false, tvb, offset, &asn1_ctx, tree, hf_camel_InitiateCallAttemptRes_PDU);
  return offset;
}
static int dissect_MoveLegArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_MoveLegArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_MoveLegArg_PDU);
  return offset;
}
static int dissect_PlayToneArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_PlayToneArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_PlayToneArg_PDU);
  return offset;
}
static int dissect_ReleaseCallArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_ReleaseCallArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_ReleaseCallArg_PDU);
  return offset;
}
static int dissect_RequestReportBCSMEventArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_RequestReportBCSMEventArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_RequestReportBCSMEventArg_PDU);
  return offset;
}
static int dissect_ResetTimerArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_ResetTimerArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_ResetTimerArg_PDU);
  return offset;
}
static int dissect_SendChargingInformationArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_SendChargingInformationArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_SendChargingInformationArg_PDU);
  return offset;
}
static int dissect_SplitLegArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_SplitLegArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_SplitLegArg_PDU);
  return offset;
}
static int dissect_ApplyChargingGPRSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_ApplyChargingGPRSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_ApplyChargingGPRSArg_PDU);
  return offset;
}
static int dissect_ApplyChargingReportGPRSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_ApplyChargingReportGPRSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_ApplyChargingReportGPRSArg_PDU);
  return offset;
}
static int dissect_CancelGPRSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_CancelGPRSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_CancelGPRSArg_PDU);
  return offset;
}
static int dissect_ConnectGPRSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_ConnectGPRSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_ConnectGPRSArg_PDU);
  return offset;
}
static int dissect_ContinueGPRSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_ContinueGPRSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_ContinueGPRSArg_PDU);
  return offset;
}
static int dissect_EntityReleasedGPRSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_EntityReleasedGPRSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_EntityReleasedGPRSArg_PDU);
  return offset;
}
static int dissect_EventReportGPRSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_EventReportGPRSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_EventReportGPRSArg_PDU);
  return offset;
}
static int dissect_FurnishChargingInformationGPRSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_FurnishChargingInformationGPRSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_FurnishChargingInformationGPRSArg_PDU);
  return offset;
}
static int dissect_InitialDPGPRSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_InitialDPGPRSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_InitialDPGPRSArg_PDU);
  return offset;
}
static int dissect_ReleaseGPRSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_ReleaseGPRSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_ReleaseGPRSArg_PDU);
  return offset;
}
static int dissect_RequestReportGPRSEventArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_RequestReportGPRSEventArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_RequestReportGPRSEventArg_PDU);
  return offset;
}
static int dissect_ResetTimerGPRSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_ResetTimerGPRSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_ResetTimerGPRSArg_PDU);
  return offset;
}
static int dissect_SendChargingInformationGPRSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_SendChargingInformationGPRSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_SendChargingInformationGPRSArg_PDU);
  return offset;
}
static int dissect_ConnectSMSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_ConnectSMSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_ConnectSMSArg_PDU);
  return offset;
}
static int dissect_EventReportSMSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_EventReportSMSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_EventReportSMSArg_PDU);
  return offset;
}
static int dissect_FurnishChargingInformationSMSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_FurnishChargingInformationSMSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_FurnishChargingInformationSMSArg_PDU);
  return offset;
}
static int dissect_InitialDPSMSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_InitialDPSMSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_InitialDPSMSArg_PDU);
  return offset;
}
static int dissect_ReleaseSMSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_ReleaseSMSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_ReleaseSMSArg_PDU);
  return offset;
}
static int dissect_RequestReportSMSEventArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_RequestReportSMSEventArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_RequestReportSMSEventArg_PDU);
  return offset;
}
static int dissect_ResetTimerSMSArg_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_ResetTimerSMSArg(false, tvb, offset, &asn1_ctx, tree, hf_camel_ResetTimerSMSArg_PDU);
  return offset;
}
static int dissect_CAP_U_ABORT_REASON_PDU(tvbuff_t *tvb _U_, packet_info *pinfo _U_, proto_tree *tree _U_, void *data _U_) {
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);
  offset = dissect_camel_CAP_U_ABORT_REASON(false, tvb, offset, &asn1_ctx, tree, hf_camel_CAP_U_ABORT_REASON_PDU);
  return offset;
}



static int dissect_invokeData(proto_tree *tree, tvbuff_t *tvb, int offset, asn1_ctx_t *actx) {

  switch(opcode){
    case opcode_playAnnouncement:  /* playAnnouncement */
      offset= dissect_PlayAnnouncementArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_promptAndCollectUserInformation:  /* promptAndCollectUserInformation */
      offset= dissect_PromptAndCollectUserInformationArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_specializedResourceReport:  /* specializedResourceReport */
      offset= dissect_SpecializedResourceReportArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_applyCharging:  /* applyCharging */
      offset= dissect_ApplyChargingArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_applyChargingReport:  /* applyChargingReport */
      offset= dissect_ApplyChargingReportArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_assistRequestInstructions:  /* assistRequestInstructions */
      offset= dissect_AssistRequestInstructionsArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_callGap:  /* callGap */
      offset= dissect_CallGapArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_callInformationReport:  /* callInformationReport */
      offset= dissect_CallInformationReportArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_callInformationRequest:  /* callInformationRequest */
      offset= dissect_CallInformationRequestArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_cancel:  /* cancel */
      offset= dissect_CancelArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_collectInformation:  /* collectInformation */
      offset= dissect_CollectInformationArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_connect:  /* connect */
      offset= dissect_ConnectArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_connectToResource:  /* connectToResource */
      offset= dissect_ConnectToResourceArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_continueWithArgument:  /* continueWithArgument */
      offset= dissect_ContinueWithArgumentArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_dFCWithArgument:  /* disconnectForwardConnectionWithArgument */
      offset= dissect_DisconnectForwardConnectionWithArgumentArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_disconnectLeg:  /* disconnectLeg */
      offset= dissect_DisconnectLegArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_entityReleased:  /* entityReleased */
      offset= dissect_EntityReleasedArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_establishTemporaryConnection:  /* establishTemporaryConnection */
      offset= dissect_EstablishTemporaryConnectionArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_eventReportBCSM:  /* eventReportBCSM */
      offset= dissect_EventReportBCSMArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_furnishChargingInformation:  /* furnishChargingInformation */
      offset= dissect_FurnishChargingInformationArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_initialDP:  /* initialDP */
      offset= dissect_InitialDPArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_initiateCallAttempt:  /* initiateCallAttempt */
      offset= dissect_InitiateCallAttemptArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_moveLeg:  /* moveLeg */
      offset= dissect_MoveLegArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_playTone:  /* playTone */
      offset= dissect_PlayToneArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_releaseCall:  /* releaseCall */
      offset= dissect_ReleaseCallArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_requestReportBCSMEvent:  /* requestReportBCSMEvent */
      offset= dissect_RequestReportBCSMEventArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_resetTimer:  /* resetTimer */
      offset= dissect_ResetTimerArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_sendChargingInformation:  /* sendChargingInformation */
      offset= dissect_SendChargingInformationArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_splitLeg:  /* splitLeg */
      offset= dissect_SplitLegArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_applyChargingGPRS:  /* applyChargingGPRS */
      offset= dissect_ApplyChargingGPRSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_applyChargingReportGPRS:  /* applyChargingReportGPRS */
      offset= dissect_ApplyChargingReportGPRSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_cancelGPRS:  /* cancelGPRS */
      offset= dissect_CancelGPRSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_connectGPRS:  /* connectGPRS */
      offset= dissect_ConnectGPRSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_continueGPRS:  /* continueGPRS */
      offset= dissect_ContinueGPRSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_entityReleasedGPRS:  /* entityReleasedGPRS */
      offset= dissect_EntityReleasedGPRSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_eventReportGPRS:  /* eventReportGPRS */
      offset= dissect_EventReportGPRSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_furnishChargingInformationGPRS:  /* furnishChargingInformationGPRS */
      offset= dissect_FurnishChargingInformationGPRSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_initialDPGPRS:  /* initialDPGPRS */
      offset= dissect_InitialDPGPRSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_releaseGPRS:  /* releaseGPRS */
      offset= dissect_ReleaseGPRSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_requestReportGPRSEvent:  /* requestReportGPRSEvent */
      offset= dissect_RequestReportGPRSEventArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_resetTimerGPRS:  /* resetTimerGPRS */
      offset= dissect_ResetTimerGPRSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_sendChargingInformationGPRS:  /* sendChargingInformationGPRS */
      offset= dissect_SendChargingInformationGPRSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_connectSMS:  /* connectSMS */
      offset= dissect_ConnectSMSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_eventReportSMS:  /* eventReportSMS */
      offset= dissect_EventReportSMSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_furnishChargingInformationSMS:  /* furnishChargingInformationSMS */
      offset= dissect_FurnishChargingInformationSMSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_initialDPSMS:  /* initialDPSMS */
      offset= dissect_InitialDPSMSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_releaseSMS:  /* releaseSMS */
      offset= dissect_ReleaseSMSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_requestReportSMSEvent:  /* requestReportSMSEvent */
      offset= dissect_RequestReportSMSEventArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_resetTimerSMS:  /* resetTimerSMS */
      offset= dissect_ResetTimerSMSArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    default:
      proto_tree_add_expert_format(tree, actx->pinfo, &ei_camel_unknown_invokeData,
                                   tvb, offset, -1, "Unknown invokeData %d", opcode);
      /* todo call the asn.1 dissector */
      break;
  }
  return offset;
}


static int dissect_returnResultData(proto_tree *tree, tvbuff_t *tvb, int offset,asn1_ctx_t *actx) {

  switch(opcode){
    case opcode_promptAndCollectUserInformation:  /* promptAndCollectUserInformation */
	  offset= dissect_ReceivedInformationArg_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case opcode_initiateCallAttempt:  /* initiateCallAttempt */
	  offset= dissect_InitiateCallAttemptRes_PDU(tvb, actx->pinfo , tree , NULL);
      break;
  default:
    proto_tree_add_expert_format(tree, actx->pinfo, &ei_camel_unknown_returnResultData,
	                             tvb, offset, -1, "Unknown returnResultData %d",opcode);
  }
  return offset;
}


static int dissect_returnErrorData(proto_tree *tree, tvbuff_t *tvb, int offset,asn1_ctx_t *actx) {

  switch(errorCode) {
    case errcode_cancelFailed:  /* cancelFailed */
      dissect_PAR_cancelFailed_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case errcode_requestedInfoError:  /* requestedInfoError */
      dissect_PAR_requestedInfoError_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case errcode_systemFailure:  /* systemFailure */
      dissect_UnavailableNetworkResource_PDU(tvb, actx->pinfo , tree , NULL);
      break;
    case errcode_taskRefused:  /* taskRefused */
      dissect_PAR_taskRefused_PDU(tvb, actx->pinfo , tree , NULL);
      break;
  default:
    proto_tree_add_expert_format(tree, actx->pinfo, &ei_camel_unknown_returnErrorData,
                                 tvb, offset, -1, "Unknown returnErrorData %d",errorCode);
  }
  return offset;
}


/*
 * Functions needed for Hash-Table
 */

/* compare 2 keys */
static int
camelsrt_call_equal(const void *k1, const void *k2)
{
  const struct camelsrt_call_info_key_t *key1 = (const struct camelsrt_call_info_key_t *) k1;
  const struct camelsrt_call_info_key_t *key2 = (const struct camelsrt_call_info_key_t *) k2;

  return (key1->SessionIdKey == key2->SessionIdKey) ;
}

/* calculate a hash key */
static unsigned
camelsrt_call_hash(const void *k)
{
  const struct camelsrt_call_info_key_t *key = (const struct camelsrt_call_info_key_t *) k;
  return key->SessionIdKey;
}

/*
 * Find the dialog by Key and Time
 */
static struct camelsrt_call_t *
find_camelsrt_call(struct camelsrt_call_info_key_t *p_camelsrt_call_key)
{
  struct camelsrt_call_t *p_camelsrt_call = NULL;
  p_camelsrt_call = (struct camelsrt_call_t *)wmem_map_lookup(srt_calls, p_camelsrt_call_key);

#ifdef DEBUG_CAMELSRT
  if(p_camelsrt_call) {
    dbg(10,"D%d ", p_camelsrt_call->session_id);
  } else {
    dbg(23,"Not in hash ");
  }
#endif

  return p_camelsrt_call;
}

/*
 * Initialize the data per call for the Service Response Time Statistics
 * Data are linked to a Camel operation in a TCAP transaction
 */
static void
raz_camelsrt_call (struct camelsrt_call_t *p_camelsrt_call)
{
  memset(p_camelsrt_call,0,sizeof(struct camelsrt_call_t));
}

/*
 * New record to create, to identify a new transaction
 */
static struct camelsrt_call_t *
new_camelsrt_call(struct camelsrt_call_info_key_t *p_camelsrt_call_key)
{
  struct camelsrt_call_info_key_t *p_new_camelsrt_call_key;
  struct camelsrt_call_t *p_new_camelsrt_call = NULL;

  /* Register the transaction in the hash table
     with the tcap transaction Id as main Key
     Once created, this entry will be updated later */

  p_new_camelsrt_call_key = wmem_new(wmem_file_scope(), struct camelsrt_call_info_key_t);
  p_new_camelsrt_call_key->SessionIdKey = p_camelsrt_call_key->SessionIdKey;
  p_new_camelsrt_call = wmem_new(wmem_file_scope(), struct camelsrt_call_t);
  raz_camelsrt_call(p_new_camelsrt_call);
  p_new_camelsrt_call->session_id = camelsrt_global_SessionId++;
#ifdef DEBUG_CAMELSRT
  dbg(10,"D%d ", p_new_camelsrt_call->session_id);
#endif
  /* store it */
  wmem_map_insert(srt_calls, p_new_camelsrt_call_key, p_new_camelsrt_call);
  return p_new_camelsrt_call;
}

/*
 * Routine called when the TAP is initialized.
 * so hash table are (re)created
 */
static void
camelsrt_init_routine(void)
{
  /* Reset the session counter */
  camelsrt_global_SessionId=1;

  /* The Display of SRT is enable
   * 1) For wireshark only if Persistent Stat is enable
   * 2) For Tshark, if the SRT CLI tap is registered
   */
  gcamel_DisplaySRT=gcamel_PersistentSRT || gcamel_StatSRT;
}


/*
 * Update a record with the data of the Request
 */
static void
update_camelsrt_call(struct camelsrt_call_t *p_camelsrt_call, packet_info *pinfo,
                     unsigned msg_category)
{
  p_camelsrt_call->category[msg_category].req_num = pinfo->num;
  p_camelsrt_call->category[msg_category].rsp_num = 0;
  p_camelsrt_call->category[msg_category].responded = false;
  p_camelsrt_call->category[msg_category].req_time = pinfo->abs_ts;
}

/*
 * Update the Camel session info, and close the session.
 * Then remove the associated context, if we do not have persistentSRT enable
 */
static void
camelsrt_close_call_matching(packet_info *pinfo,
                             struct camelsrt_info_t *p_camelsrt_info)
{
  struct camelsrt_call_t *p_camelsrt_call;
  struct camelsrt_call_info_key_t camelsrt_call_key;
  nstime_t delta;

  p_camelsrt_info->bool_msginfo[CAMELSRT_SESSION]=true;
#ifdef DEBUG_CAMELSRT
  dbg(10,"\n Session end #%u\n", pinfo->num);
#endif
  /* look only for matching request, if matching conversation is available. */
  camelsrt_call_key.SessionIdKey = p_camelsrt_info->tcap_session_id;

#ifdef DEBUG_CAMELSRT
  dbg(11,"Search key %lu ",camelsrt_call_key.SessionIdKey);
#endif
  p_camelsrt_call = find_camelsrt_call(&camelsrt_call_key);
  if(p_camelsrt_call) {
#ifdef DEBUG_CAMELSRT
    dbg(12,"Found ");
#endif
    /* Calculate Service Response Time */
    nstime_delta(&delta, &pinfo->abs_ts, &p_camelsrt_call->category[CAMELSRT_SESSION].req_time);
    p_camelsrt_call->category[CAMELSRT_SESSION].responded = true;
    p_camelsrt_info->msginfo[CAMELSRT_SESSION].request_available = true;
    p_camelsrt_info->msginfo[CAMELSRT_SESSION].is_delta_time = true;
    p_camelsrt_info->msginfo[CAMELSRT_SESSION].delta_time = delta; /* give it to tap */
    p_camelsrt_info->msginfo[CAMELSRT_SESSION].req_time = p_camelsrt_call->category[CAMELSRT_SESSION].req_time;

    if ( !gcamel_PersistentSRT ) {
      wmem_map_remove(srt_calls, &camelsrt_call_key);
#ifdef DEBUG_CAMELSRT
      dbg(20,"remove hash ");
#endif
    } else {
#ifdef DEBUG_CAMELSRT
      dbg(20,"keep hash ");
#endif
    }
  } /* call reference found */
}

/*
 * Callback function for the TCAP dissector
 * This callback function is used to inform the camel layer, that the session
 * has been Closed or Aborted by a TCAP message without Camel component
 * So, we can close the context for camel session, and update the stats.
 */
static void
camelsrt_tcap_matching(tvbuff_t *tvb _U_, packet_info *pinfo,
                       proto_tree *tree _U_,
                       struct tcaphash_context_t *p_tcap_context)
{
  struct camelsrt_info_t *p_camelsrt_info;

#ifdef DEBUG_CAMELSRT
  dbg(11,"Camel_CallBack ");
#endif
  p_camelsrt_info=camelsrt_razinfo();

  p_camelsrt_info->tcap_context=p_tcap_context;
  if (p_tcap_context) {
#ifdef DEBUG_CAMELSRT
    dbg(11,"Close TCAP ");
#endif
    p_camelsrt_info->tcap_session_id = p_tcap_context->session_id;
    camelsrt_close_call_matching(pinfo, p_camelsrt_info);
    tap_queue_packet(camel_tap, pinfo, p_camelsrt_info);
  }
}

/*
 * Create the record identifiying the Camel session
 * As the Tcap session id given by the TCAP dissector is uniq, it will be
 * used as main key.
 */
static void
camelsrt_begin_call_matching(packet_info *pinfo,
                             struct camelsrt_info_t *p_camelsrt_info)
{
  struct camelsrt_call_t *p_camelsrt_call;
  struct camelsrt_call_info_key_t camelsrt_call_key;

  p_camelsrt_info->bool_msginfo[CAMELSRT_SESSION]=true;

  /* prepare the key data */
  camelsrt_call_key.SessionIdKey = p_camelsrt_info->tcap_session_id;

  /* look up the request */
#ifdef DEBUG_CAMELSRT
  dbg(10,"\n Session begin #%u\n", pinfo->num);
  dbg(11,"Search key %lu ",camelsrt_call_key.SessionIdKey);
#endif
  p_camelsrt_call = (struct camelsrt_call_t *)wmem_map_lookup(srt_calls, &camelsrt_call_key);
  if (p_camelsrt_call) {
    /* We have seen this request before -> do nothing */
#ifdef DEBUG_CAMELSRT
    dbg(22,"Already seen ");
#endif
  } else { /* p_camelsrt_call has not been found */
#ifdef DEBUG_CAMELSRT
    dbg(10,"New key %lu ",camelsrt_call_key.SessionIdKey);
#endif
    p_camelsrt_call = new_camelsrt_call(&camelsrt_call_key);
    p_camelsrt_call->tcap_context=(struct tcaphash_context_t *)p_camelsrt_info->tcap_context;
    update_camelsrt_call(p_camelsrt_call, pinfo,CAMELSRT_SESSION);

#ifdef DEBUG_CAMELSRT
    dbg(11,"Update Callback ");
#endif
    p_camelsrt_call->tcap_context->callback=camelsrt_tcap_matching;
  }
}

/*
 * Register the request, and try to find the response
 *
 */
static void
camelsrt_request_call_matching(tvbuff_t *tvb, packet_info *pinfo,
                               proto_tree *tree,
                               struct camelsrt_info_t *p_camelsrt_info,
                               unsigned srt_type )
{
  struct camelsrt_call_t *p_camelsrt_call;
  struct camelsrt_call_info_key_t camelsrt_call_key;
  proto_item *ti, *hidden_item;

#ifdef DEBUG_CAMELSRT
  dbg(10,"\n %s #%u\n", val_to_str_const(srt_type, camelSRTtype_naming, "Unk"),pinfo->num);
#endif

  /* look only for matching request, if matching conversation is available. */
  camelsrt_call_key.SessionIdKey = p_camelsrt_info->tcap_session_id;

#ifdef DEBUG_CAMELSRT
  dbg(11,"Search key %lu ", camelsrt_call_key.SessionIdKey);
#endif
  p_camelsrt_call = find_camelsrt_call(&camelsrt_call_key);
  if(p_camelsrt_call) {
#ifdef DEBUG_CAMELSRT
    dbg(12,"Found ");
#endif
    if (gcamel_DisplaySRT)
      proto_tree_add_uint(tree, hf_camelsrt_SessionId, tvb, 0,0, p_camelsrt_call->session_id);


    /* Hmm.. As there are several slices ApplyChargingReport/ApplyCharging
     * we will prepare the measurement for 3 slices with 3 categories */
    if (srt_type==CAMELSRT_VOICE_ACR1) {
      if (p_camelsrt_call->category[CAMELSRT_VOICE_ACR1].req_num == 0) {
        srt_type=CAMELSRT_VOICE_ACR1;
      } else  if ( (p_camelsrt_call->category[CAMELSRT_VOICE_ACR2].req_num == 0)
                   && (p_camelsrt_call->category[CAMELSRT_VOICE_ACR1].rsp_num != 0)
                   && (p_camelsrt_call->category[CAMELSRT_VOICE_ACR1].rsp_num < pinfo->num) ) {
        srt_type=CAMELSRT_VOICE_ACR2;
      } else  if ( (p_camelsrt_call->category[CAMELSRT_VOICE_ACR3].req_num == 0)
                   && (p_camelsrt_call->category[CAMELSRT_VOICE_ACR2].rsp_num != 0)
                   && (p_camelsrt_call->category[CAMELSRT_VOICE_ACR2].rsp_num < pinfo->num) ) {
        srt_type=CAMELSRT_VOICE_ACR3;
      } else if (p_camelsrt_call->category[CAMELSRT_VOICE_ACR1].rsp_num != 0
                 && p_camelsrt_call->category[CAMELSRT_VOICE_ACR1].rsp_num > pinfo->num) {
        srt_type=CAMELSRT_VOICE_ACR1;
      } else  if ( p_camelsrt_call->category[CAMELSRT_VOICE_ACR2].rsp_num != 0
                   && p_camelsrt_call->category[CAMELSRT_VOICE_ACR2].rsp_num > pinfo->num) {
        srt_type=CAMELSRT_VOICE_ACR2;
      } else  if (p_camelsrt_call->category[CAMELSRT_VOICE_ACR1].rsp_num != 0
                  && p_camelsrt_call->category[CAMELSRT_VOICE_ACR3].rsp_num > pinfo->num) {
        srt_type=CAMELSRT_VOICE_ACR3;
      }
#ifdef DEBUG_CAMELSRT
      dbg(70,"Request ACR %u ",srt_type);
      dbg(70,"ACR1 %u %u",p_camelsrt_call->category[CAMELSRT_VOICE_ACR1].req_num, p_camelsrt_call->category[CAMELSRT_VOICE_ACR1].rsp_num);
      dbg(70,"ACR2 %u %u",p_camelsrt_call->category[CAMELSRT_VOICE_ACR2].req_num, p_camelsrt_call->category[CAMELSRT_VOICE_ACR2].rsp_num);
      dbg(70,"ACR3 %u %u",p_camelsrt_call->category[CAMELSRT_VOICE_ACR3].req_num, p_camelsrt_call->category[CAMELSRT_VOICE_ACR3].rsp_num);
#endif
    } /* not ACR */
    p_camelsrt_info->bool_msginfo[srt_type]=true;


    if (p_camelsrt_call->category[srt_type].req_num == 0) {
      /* We have not yet seen a request to that call, so this must be the first request
         remember its frame number. */
#ifdef DEBUG_CAMELSRT
      dbg(5,"Set reqlink #%u ", pinfo->num);
#endif
      update_camelsrt_call(p_camelsrt_call, pinfo, srt_type);
    } else {
      /* We have seen a request to this call - but was it *this* request? */
      if (p_camelsrt_call->category[srt_type].req_num != pinfo->num) {

        if (srt_type!=CAMELSRT_VOICE_DISC) {
          /* No, so it's a duplicate request. Mark it as such. */
#ifdef DEBUG_CAMELSRT
          dbg(21,"Display_duplicate with req %d ", p_camelsrt_call->category[srt_type].req_num);
#endif
          p_camelsrt_info->msginfo[srt_type].is_duplicate = true;
          if (gcamel_DisplaySRT){
            hidden_item = proto_tree_add_uint(tree, hf_camelsrt_Duplicate, tvb, 0,0, 77);
                proto_item_set_hidden(hidden_item);
          }

        } else {
          /* Ignore duplicate frame */
          if (pinfo->num > p_camelsrt_call->category[srt_type].req_num) {
            p_camelsrt_call->category[srt_type].req_num = pinfo->num;
#ifdef DEBUG_CAMELSRT
            dbg(5,"DISC Set reqlink #%u ", pinfo->num);
#endif
            update_camelsrt_call(p_camelsrt_call, pinfo, srt_type);
          } /* greater frame */
        } /* DISC */
      } /* req_num already seen */
    } /* req_num != 0 */

      /* add link to response frame, if available */
    if ( gcamel_DisplaySRT &&
         (p_camelsrt_call->category[srt_type].rsp_num != 0) &&
         (p_camelsrt_call->category[srt_type].req_num != 0) &&
         (p_camelsrt_call->category[srt_type].req_num == pinfo->num) ) {
#ifdef DEBUG_CAMELSRT
      dbg(20,"Display_framersplink %d ",p_camelsrt_call->category[srt_type].rsp_num);
#endif
      ti = proto_tree_add_uint_format(tree, hf_camelsrt_RequestFrame, tvb, 0, 0,
                                      p_camelsrt_call->category[srt_type].rsp_num,
                                      "Linked response %s in frame %u",
                                      val_to_str_const(srt_type, camelSRTtype_naming, "Unk"),
                                      p_camelsrt_call->category[srt_type].rsp_num);
      proto_item_set_generated(ti);
    } /* frame valid */
  } /* call reference */
}

/*
 * Display the delta time between two messages in a field corresponding
 * to the category (hf_camelsrt_DeltaTimexx).
 */
static void
camelsrt_display_DeltaTime(proto_tree *tree, tvbuff_t *tvb, nstime_t *value_ptr,
                           unsigned category)
{
  proto_item *ti;

  if ( gcamel_DisplaySRT ) {
    switch(category) {
    case CAMELSRT_VOICE_INITIALDP:
      ti = proto_tree_add_time(tree, hf_camelsrt_DeltaTime31, tvb, 0, 0, value_ptr);
      proto_item_set_generated(ti);
      break;

    case CAMELSRT_VOICE_ACR1:
    case CAMELSRT_VOICE_ACR2:
    case CAMELSRT_VOICE_ACR3:
      ti = proto_tree_add_time(tree, hf_camelsrt_DeltaTime22, tvb, 0, 0, value_ptr);
      proto_item_set_generated(ti);
      break;

    case CAMELSRT_VOICE_DISC:
      ti = proto_tree_add_time(tree, hf_camelsrt_DeltaTime35, tvb, 0, 0, value_ptr);
      proto_item_set_generated(ti);
      break;

    case CAMELSRT_GPRS_INITIALDP:
      ti = proto_tree_add_time(tree, hf_camelsrt_DeltaTime75, tvb, 0, 0, value_ptr);
      proto_item_set_generated(ti);
      break;

    case CAMELSRT_GPRS_REPORT:
      ti = proto_tree_add_time(tree, hf_camelsrt_DeltaTime80, tvb, 0, 0, value_ptr);
      proto_item_set_generated(ti);
      break;

    case CAMELSRT_SMS_INITIALDP:
      ti = proto_tree_add_time(tree, hf_camelsrt_DeltaTime65, tvb, 0, 0, value_ptr);
      proto_item_set_generated(ti);
      break;

    default:
      break;
    }
  }
}

/*
 * Check if the received message is a response to a previous request
 * registered is the camel session context.
 */
static void
camelsrt_report_call_matching(tvbuff_t *tvb, packet_info *pinfo,
                              proto_tree *tree,
                              struct camelsrt_info_t *p_camelsrt_info,
                              unsigned srt_type)
{
  struct camelsrt_call_t *p_camelsrt_call;
  struct camelsrt_call_info_key_t camelsrt_call_key;
  nstime_t delta;
  proto_item *ti, *hidden_item;

#ifdef DEBUG_CAMELSRT
  dbg(10,"\n %s #%u\n", val_to_str_const(srt_type, camelSRTtype_naming, "Unk"),pinfo->num);
#endif
  camelsrt_call_key.SessionIdKey = p_camelsrt_info->tcap_session_id;
  /* look only for matching request, if matching conversation is available. */

#ifdef DEBUG_CAMELSRT
  dbg(11,"Search key %lu ",camelsrt_call_key.SessionIdKey);
#endif
  p_camelsrt_call = find_camelsrt_call(&camelsrt_call_key);
  if(p_camelsrt_call) {
#ifdef DEBUG_CAMELSRT
    dbg(12,"Found, req=%d ",p_camelsrt_call->category[srt_type].req_num);
#endif
    if ( gcamel_DisplaySRT )
      proto_tree_add_uint(tree, hf_camelsrt_SessionId, tvb, 0,0, p_camelsrt_call->session_id);

    if (srt_type==CAMELSRT_VOICE_ACR1) {
      if (p_camelsrt_call->category[CAMELSRT_VOICE_ACR3].req_num != 0
          && p_camelsrt_call->category[CAMELSRT_VOICE_ACR3].req_num < pinfo->num) {
        srt_type=CAMELSRT_VOICE_ACR1;
      } else  if ( p_camelsrt_call->category[CAMELSRT_VOICE_ACR2].req_num != 0
                   && p_camelsrt_call->category[CAMELSRT_VOICE_ACR2].req_num < pinfo->num) {
        srt_type=CAMELSRT_VOICE_ACR2;
      } else  if (p_camelsrt_call->category[CAMELSRT_VOICE_ACR1].req_num != 0
                  && p_camelsrt_call->category[CAMELSRT_VOICE_ACR1].req_num < pinfo->num) {
        srt_type=CAMELSRT_VOICE_ACR1;
      }
#ifdef DEBUG_CAMELSRT
      dbg(70,"Report ACR %u ",srt_type);
#endif
    } /* not ACR */
    p_camelsrt_info->bool_msginfo[srt_type]=true;

    if (p_camelsrt_call->category[srt_type].rsp_num == 0) {
      if  ( (p_camelsrt_call->category[srt_type].req_num != 0)
            && (pinfo->num > p_camelsrt_call->category[srt_type].req_num) ){
        /* We have not yet seen a response to that call, so this must be the first response;
           remember its frame number only if response comes after request */
#ifdef DEBUG_CAMELSRT
        dbg(14,"Set reslink #%d req %u ",pinfo->num, p_camelsrt_call->category[srt_type].req_num);
#endif
        p_camelsrt_call->category[srt_type].rsp_num = pinfo->num;

      } else {
#ifdef DEBUG_CAMELSRT
        dbg(2,"badreslink #%u req %u ",pinfo->num, p_camelsrt_call->category[srt_type].req_num);
#endif
      } /* req_num != 0 */
    } else { /* rsp_num != 0 */
      /* We have seen a response to this call - but was it *this* response? */
      if (p_camelsrt_call->category[srt_type].rsp_num != pinfo->num) {
        /* No, so it's a duplicate response. Mark it as such. */
#ifdef DEBUG_CAMELSRT
        dbg(21,"Display_duplicate rsp=%d ", p_camelsrt_call->category[srt_type].rsp_num);
#endif
        p_camelsrt_info->msginfo[srt_type].is_duplicate = true;
        if ( gcamel_DisplaySRT ){
          hidden_item = proto_tree_add_uint(tree, hf_camelsrt_Duplicate, tvb, 0,0, 77);
          proto_item_set_hidden(hidden_item);
        }
      }
    } /* rsp_num != 0 */

    if ( (p_camelsrt_call->category[srt_type].req_num != 0) &&
         (p_camelsrt_call->category[srt_type].rsp_num != 0) &&
         (p_camelsrt_call->category[srt_type].rsp_num == pinfo->num) ) {

      p_camelsrt_call->category[srt_type].responded = true;
      p_camelsrt_info->msginfo[srt_type].request_available = true;
#ifdef DEBUG_CAMELSRT
      dbg(20,"Display_frameReqlink %d ",p_camelsrt_call->category[srt_type].req_num);
#endif
      /* Indicate the frame to which this is a reply. */
      if ( gcamel_DisplaySRT ) {
        ti = proto_tree_add_uint_format(tree, hf_camelsrt_ResponseFrame, tvb, 0, 0,
                                        p_camelsrt_call->category[srt_type].req_num,
                                        "Linked request %s in frame %u",
                                        val_to_str_const(srt_type, camelSRTtype_naming, "Unk"),
                                        p_camelsrt_call->category[srt_type].req_num);
        proto_item_set_generated(ti);
      }
      /* Calculate Service Response Time */
      nstime_delta(&delta, &pinfo->abs_ts, &p_camelsrt_call->category[srt_type].req_time);

      p_camelsrt_info->msginfo[srt_type].is_delta_time = true;
      p_camelsrt_info->msginfo[srt_type].delta_time = delta; /* give it to tap */
      p_camelsrt_info->msginfo[srt_type].req_time = p_camelsrt_call->category[srt_type].req_time;

      /* display Service Response Time and make it filterable */
      camelsrt_display_DeltaTime(tree, tvb, &delta, srt_type);

    } /*req_num != 0 && not duplicate */
  } /* call reference found */
}

/*
 * Service Response Time analyze, called just after the camel dissector
 * According to the camel operation, we
 * - open/close a context for the camel session
 * - look for a request, or look for the corresponding response
 */
void
camelsrt_call_matching(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree,
                       struct camelsrt_info_t *p_camelsrt_info)
{

#ifdef DEBUG_CAMELSRT
  dbg(10,"tcap_session #%d ", p_camelsrt_info->tcap_session_id);
#endif

  switch (p_camelsrt_info->opcode) {

  case 0:  /*InitialDP*/
    camelsrt_begin_call_matching(pinfo, p_camelsrt_info);
    camelsrt_request_call_matching(tvb, pinfo, tree, p_camelsrt_info,
                                   CAMELSRT_VOICE_INITIALDP);
    break;
  case 60: /*InitialDPSMS*/
    camelsrt_begin_call_matching(pinfo, p_camelsrt_info);
    camelsrt_request_call_matching(tvb, pinfo, tree, p_camelsrt_info,
                                   CAMELSRT_SMS_INITIALDP);
    break;
  case 78: /*InitialDPGPRS*/
    camelsrt_begin_call_matching(pinfo, p_camelsrt_info);
    camelsrt_request_call_matching(tvb, pinfo, tree, p_camelsrt_info,
                                   CAMELSRT_GPRS_INITIALDP);
    break;

  case 23: /*RequestReportBCSMEvent*/
    break;

  case 63: /*RequestReportSMSEvent*/
    break;

  case 81: /*RequestReportGPRSEvent*/
    break;

  case 24: /*EventReportBCSMEvent*/
    camelsrt_request_call_matching(tvb, pinfo, tree, p_camelsrt_info,
                                   CAMELSRT_VOICE_DISC );
    break;

  case 64: /*EventReportSMS*/
    /* Session has been explicitly closed without TC_END */
    camelsrt_close_call_matching(pinfo, p_camelsrt_info);
    tcapsrt_close((struct tcaphash_context_t *)p_camelsrt_info->tcap_context, pinfo);
    break;

  case 80: /*EventReportGPRS*/
    camelsrt_begin_call_matching(pinfo, p_camelsrt_info);
    camelsrt_request_call_matching(tvb, pinfo, tree, p_camelsrt_info,
                                   CAMELSRT_GPRS_REPORT);
    break;

  case 35: /*ApplyCharging*/
    camelsrt_report_call_matching(tvb, pinfo, tree, p_camelsrt_info,
                                  CAMELSRT_VOICE_ACR1 );
    break;

  case 71: /*ApplyChargingGPRS*/
    break;

  case 36: /*ApplyChargingReport*/
    camelsrt_request_call_matching(tvb, pinfo, tree, p_camelsrt_info,
                                   CAMELSRT_VOICE_ACR1 );
    break;

  case 72: /*ApplyChargingReportGPRS*/
    break;

  case 31: /*Continue*/
    camelsrt_report_call_matching(tvb, pinfo, tree, p_camelsrt_info,
                                      CAMELSRT_VOICE_INITIALDP);
    break;
  case 65: /*ContinueSMS*/
    camelsrt_report_call_matching(tvb, pinfo, tree, p_camelsrt_info,
                                  CAMELSRT_SMS_INITIALDP);
    break;
  case 75: /*ContinueGPRS*/
    camelsrt_report_call_matching(tvb, pinfo, tree, p_camelsrt_info,
                                  CAMELSRT_GPRS_INITIALDP);
    camelsrt_report_call_matching(tvb, pinfo, tree, p_camelsrt_info,
                                  CAMELSRT_GPRS_REPORT);
    break;

  case 22: /*ReleaseCall*/
    camelsrt_report_call_matching(tvb, pinfo, tree, p_camelsrt_info,
                                      CAMELSRT_VOICE_DISC);
    /* Session has been closed by Network */
    camelsrt_close_call_matching(pinfo, p_camelsrt_info);
    break;

  case 66: /*ReleaseSMS*/
    /* Session has been closed by Network */
    camelsrt_close_call_matching(pinfo, p_camelsrt_info);
    tcapsrt_close((struct tcaphash_context_t *)p_camelsrt_info->tcap_context,pinfo);
    break;

  case 79: /*ReleaseGPRS*/
    /* Session has been closed by Network */
    camelsrt_close_call_matching(pinfo, p_camelsrt_info);
    break;
  } /* switch opcode */
}

/*
 * Initialize the Message Info used by the main dissector
 * Data are linked to a TCAP transaction
 */
struct camelsrt_info_t *
camelsrt_razinfo(void)
{
  struct camelsrt_info_t *p_camelsrt_info ;

  /* Global buffer for packet extraction */
  camelsrt_global_current++;
  if(camelsrt_global_current==MAX_CAMEL_INSTANCE){
    camelsrt_global_current=0;
  }

  p_camelsrt_info=&camelsrt_global_info[camelsrt_global_current];
  memset(p_camelsrt_info,0,sizeof(struct camelsrt_info_t));

  p_camelsrt_info->opcode=255;

  return p_camelsrt_info;
}


static uint8_t camel_pdu_type;
static uint8_t camel_pdu_size;


static int
dissect_camel_camelPDU(bool implicit_tag _U_, tvbuff_t *tvb, int offset, asn1_ctx_t *actx _U_,proto_tree *tree,
                        int hf_index, struct tcap_private_t * p_private_tcap) {

    opcode = 0;
    if (p_private_tcap != NULL){
        gp_camelsrt_info->tcap_context=p_private_tcap->context;
        if (p_private_tcap->context)
            gp_camelsrt_info->tcap_session_id = ( (struct tcaphash_context_t *) (p_private_tcap->context))->session_id;
    }

    camel_pdu_type = tvb_get_uint8(tvb, offset)&0x0f;
    /* Get the length and add 2 */
    camel_pdu_size = tvb_get_uint8(tvb, offset+1)+2;

    /* Populate the info column with PDU type*/
    col_add_str(actx->pinfo->cinfo, COL_INFO, val_to_str(camel_pdu_type, camel_Component_vals, "Unknown Camel (%u)"));
    col_append_str(actx->pinfo->cinfo, COL_INFO, " ");

    is_ExtensionField =false;
    offset = dissect_camel_ROS(true, tvb, offset, actx, tree, hf_index);

    return offset;
}

static int
dissect_camel_all(int version, const char* col_protocol, const char* suffix,
                  tvbuff_t *tvb, packet_info *pinfo, proto_tree *parent_tree, void* data)
{
  proto_item  *item;
  proto_tree  *tree = NULL, *stat_tree = NULL;
  struct tcap_private_t * p_private_tcap = (struct tcap_private_t*)data;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, true, pinfo);

  col_set_str(pinfo->cinfo, COL_PROTOCOL, col_protocol);

  camel_ver = version;

  /* create display subtree for the protocol */
  if(parent_tree){
     item = proto_tree_add_item(parent_tree, proto_camel, tvb, 0, -1, ENC_NA);
     tree = proto_item_add_subtree(item, ett_camel);
     proto_item_append_text(item, "%s", suffix);
  }
  /* camelsrt reset counter, and initialise global pointer
     to store service response time related data */
  gp_camelsrt_info=camelsrt_razinfo();

  dissect_camel_camelPDU(false, tvb, 0, &asn1_ctx , tree, -1, p_private_tcap);

  /* If a Tcap context is associated to this transaction */
  if (gp_camelsrt_info->tcap_context ) {
    if (gcamel_DisplaySRT && tree) {
      stat_tree = proto_tree_add_subtree(tree, tvb, 0, 0, ett_camel_stat, NULL, "Stat");
    }
    camelsrt_call_matching(tvb, pinfo, stat_tree, gp_camelsrt_info);
    tap_queue_packet(camel_tap, pinfo, gp_camelsrt_info);
  }

  return tvb_captured_length(tvb);
}

static int
dissect_camel_v1(tvbuff_t *tvb, packet_info *pinfo, proto_tree *parent_tree, void* data)
{
  return dissect_camel_all(1, "Camel-v1", "-V1", tvb, pinfo, parent_tree, data);
}

static int
dissect_camel_v2(tvbuff_t *tvb, packet_info *pinfo, proto_tree *parent_tree, void* data)
{
  return dissect_camel_all(2, "Camel-v2", "-V2", tvb, pinfo, parent_tree, data);
}

static int
dissect_camel_v3(tvbuff_t *tvb, packet_info *pinfo, proto_tree *parent_tree, void* data)
{
  return dissect_camel_all(3, "Camel-v3", "-V3", tvb, pinfo, parent_tree, data);
}

static int
dissect_camel_v4(tvbuff_t *tvb, packet_info *pinfo, proto_tree *parent_tree, void* data)
{
  return dissect_camel_all(4, "Camel-v4", "-V4", tvb, pinfo, parent_tree, data);
}

static int
dissect_camel(tvbuff_t *tvb, packet_info *pinfo, proto_tree *parent_tree, void* data)
{
  return dissect_camel_all(4, "Camel", "", tvb, pinfo, parent_tree, data);
}

/* TAP STAT INFO */
typedef enum
{
  MESSAGE_TYPE_COLUMN = 0,
  COUNT_COLUMN
} camel_stat_columns;

static stat_tap_table_item camel_stat_fields[] = {{TABLE_ITEM_STRING, TAP_ALIGN_LEFT, "Message Type or Reason", "%-25s"}, {TABLE_ITEM_UINT, TAP_ALIGN_RIGHT, "Count", "%d"}};

static void camel_stat_init(stat_tap_table_ui* new_stat)
{
  const char *table_name = "CAMEL Message Counters";
  int num_fields = array_length(camel_stat_fields);
  stat_tap_table *table;
  int i;
  stat_tap_table_item_type items[array_length(camel_stat_fields)];

  table = stat_tap_find_table(new_stat, table_name);
  if (table) {
    if (new_stat->stat_tap_reset_table_cb) {
      new_stat->stat_tap_reset_table_cb(table);
    }
    return;
  }

  table = stat_tap_init_table(table_name, num_fields, 0, NULL);
  stat_tap_add_table(new_stat, table);

  memset(items, 0x0, sizeof(items));
  items[MESSAGE_TYPE_COLUMN].type = TABLE_ITEM_STRING;
  items[COUNT_COLUMN].type = TABLE_ITEM_UINT;
  items[COUNT_COLUMN].value.uint_value = 0;

  /* Add a row for each value type */
  for (i = 0; i < camel_MAX_NUM_OPR_CODES; i++)
  {
    const char *ocs = try_val_to_str(i, camel_opr_code_strings);
    char *col_str;
    if (ocs) {
      col_str = ws_strdup_printf("Request %s", ocs);
    } else {
      col_str = ws_strdup_printf("Unknown op code %d", i);
    }

    items[MESSAGE_TYPE_COLUMN].value.string_value = col_str;
    stat_tap_init_table_row(table, i, num_fields, items);
  }
}

static tap_packet_status
camel_stat_packet(void *tapdata, packet_info *pinfo _U_, epan_dissect_t *edt _U_, const void *csi_ptr, tap_flags_t flags _U_)
{
  stat_data_t* stat_data = (stat_data_t*)tapdata;
  const struct camelsrt_info_t *csi = (const struct camelsrt_info_t *) csi_ptr;
  stat_tap_table* table;
  stat_tap_table_item_type* msg_data;

  table = g_array_index(stat_data->stat_tap_data->tables, stat_tap_table*, 0);
  if (csi->opcode >= table->num_elements)
    return TAP_PACKET_DONT_REDRAW;
  msg_data = stat_tap_get_field_data(table, csi->opcode, COUNT_COLUMN);
  msg_data->value.uint_value++;
  stat_tap_set_field_data(table, csi->opcode, COUNT_COLUMN, msg_data);

  return TAP_PACKET_REDRAW;
}

static void
camel_stat_reset(stat_tap_table* table)
{
  unsigned element;
  stat_tap_table_item_type* item_data;

  for (element = 0; element < table->num_elements; element++)
  {
    item_data = stat_tap_get_field_data(table, element, COUNT_COLUMN);
    item_data->value.uint_value = 0;
    stat_tap_set_field_data(table, element, COUNT_COLUMN, item_data);
  }
}

static void
camel_stat_free_table_item(stat_tap_table* table _U_, unsigned row _U_, unsigned column, stat_tap_table_item_type* field_data)
{
  if (column != MESSAGE_TYPE_COLUMN) return;
  g_free((char*)field_data->value.string_value);
}

/*--- proto_reg_handoff_camel ---------------------------------------*/
static void range_delete_callback(uint32_t ssn, void *ptr _U_)
{
  if (ssn) {
    delete_itu_tcap_subdissector(ssn, camel_handle);
  }
}

static void range_add_callback(uint32_t ssn, void *ptr _U_)
{
  if (ssn) {
    add_itu_tcap_subdissector(ssn, camel_handle);
  }
}

void proto_reg_handoff_camel(void) {
  static bool camel_prefs_initialized = false;
  static range_t *ssn_range;

  if (!camel_prefs_initialized) {

    camel_prefs_initialized = true;

    register_ber_oid_dissector_handle("0.4.0.0.1.0.50.0",camel_v1_handle, proto_camel, "CAP-v1-gsmSSF-to-gsmSCF-AC" );
    register_ber_oid_dissector_handle("0.4.0.0.1.0.50.1",camel_v2_handle, proto_camel, "CAP-v2-gsmSSF-to-gsmSCF-AC" );
    register_ber_oid_dissector_handle("0.4.0.0.1.0.51.1",camel_v2_handle, proto_camel, "CAP-v2-assist-gsmSSF-to-gsmSCF-AC" );
    register_ber_oid_dissector_handle("0.4.0.0.1.0.52.1",camel_v2_handle, proto_camel, "CAP-v2-gsmSRF-to-gsmSCF-AC" );

    /* CAMEL Phase 3 Application Context Names */
    register_ber_oid_dissector_handle("0.4.0.0.1.21.3.4", camel_v3_handle, proto_camel, "capssf-scfGenericAC");
    register_ber_oid_dissector_handle("0.4.0.0.1.21.3.6", camel_v3_handle, proto_camel, "capssf-scfAssistHandoffAC");
    register_ber_oid_dissector_handle("0.4.0.0.1.20.3.14", camel_v3_handle, proto_camel, "gsmSRF-gsmSCF-ac");
    register_ber_oid_dissector_handle("0.4.0.0.1.21.3.50", camel_v3_handle, proto_camel, "cap3-gprssf-scfAC");
    register_ber_oid_dissector_handle("0.4.0.0.1.21.3.51", camel_v3_handle, proto_camel, "cap3-gsmscf-gprsssfAC");
    register_ber_oid_dissector_handle("0.4.0.0.1.21.3.61", camel_v3_handle, proto_camel, "cap3-sms-AC");

    /* CAMEL Phase 4 Application Context Names */
    register_ber_oid_dissector_handle("0.4.0.0.1.23.3.4", camel_v4_handle, proto_camel, "capssf-scfGenericAC");
    register_ber_oid_dissector_handle("0.4.0.0.1.23.3.6", camel_v4_handle, proto_camel, "capssf-scfAssistHandoffAC");
    register_ber_oid_dissector_handle("0.4.0.0.1.23.3.8", camel_v4_handle, proto_camel, "capscf-ssfGenericAC");
    register_ber_oid_dissector_handle("0.4.0.0.1.22.3.14", camel_v4_handle, proto_camel, "gsmSRF-gsmSCF-ac");
    register_ber_oid_dissector_handle("0.4.0.0.1.23.3.61", camel_v4_handle, proto_camel, "cap4-sms-AC");


  register_ber_oid_dissector("0.4.0.0.1.1.5.2", dissect_CAP_GPRS_ReferenceNumber_PDU, proto_camel, "id-CAP-GPRS-ReferenceNumber");
  register_ber_oid_dissector("0.4.0.0.1.1.2.2", dissect_CAP_U_ABORT_REASON_PDU, proto_camel, "id-CAP-U-ABORT-Reason");

  } else {
    range_foreach(ssn_range, range_delete_callback, NULL);
    wmem_free(wmem_epan_scope(), ssn_range);
  }

  ssn_range = range_copy(wmem_epan_scope(), global_ssn_range);

  range_foreach(ssn_range, range_add_callback, NULL);

}

void proto_register_camel(void) {
  module_t *camel_module;
  /* List of fields */
  static hf_register_info hf[] = {
    { &hf_camel_extension_code_local,
      { "local", "camel.extension_code_local",
        FT_INT32, BASE_DEC, NULL, 0,
        "Extension local code", HFILL }},
        { &hf_camel_error_code_local,
      { "local", "camel.error_code_local",
        FT_INT32, BASE_DEC, VALS(camel_err_code_string_vals), 0,
        "ERROR code", HFILL }},
    { &hf_camel_cause_indicator, /* Currently not enabled */
      { "Cause indicator",  "camel.cause_indicator",
        FT_UINT8, BASE_DEC|BASE_EXT_STRING, &q850_cause_code_vals_ext, 0x7f,
        NULL, HFILL }},
    { &hf_digit,
      { "Digit Value",  "camel.digit_value",
        FT_UINT8, BASE_DEC, VALS(digit_value), 0, NULL, HFILL }},
    { &hf_camel_PDPTypeNumber_etsi,
      { "ETSI defined PDP Type Value",  "camel.PDPTypeNumber_etsi",
        FT_UINT8, BASE_HEX, VALS(gsm_map_etsi_defined_pdp_vals), 0,
        NULL, HFILL }},
    { &hf_camel_PDPTypeNumber_ietf,
      { "IETF defined PDP Type Value",  "camel.PDPTypeNumber_ietf",
        FT_UINT8, BASE_HEX, VALS(gsm_map_ietf_defined_pdp_vals), 0,
        NULL, HFILL }},
    { &hf_camel_PDPAddress_IPv4,
      { "PDPAddress IPv4",  "camel.PDPAddress_IPv4",
        FT_IPv4, BASE_NONE, NULL, 0,
        "IPAddress IPv4", HFILL }},
    { &hf_camel_PDPAddress_IPv6,
      { "PDPAddress IPv6",  "camel.PDPAddress_IPv6",
        FT_IPv6, BASE_NONE, NULL, 0,
        "IPAddress IPv6", HFILL }},
    { &hf_camel_cellGlobalIdOrServiceAreaIdFixedLength,
      { "CellGlobalIdOrServiceAreaIdFixedLength", "camel.CellGlobalIdOrServiceAreaIdFixedLength",
        FT_BYTES, BASE_NONE, NULL, 0,
        "LocationInformationGPRS/CellGlobalIdOrServiceAreaIdOrLAI", HFILL }},
    { &hf_camel_RP_Cause,
      { "RP Cause",  "camel.RP_Cause",
        FT_UINT8, BASE_DEC, VALS(camel_RP_Cause_values), 0x7F,
        "RP Cause Value", HFILL }},

    { &hf_camel_CAMEL_AChBillingChargingCharacteristics,
      { "CAMEL-AChBillingChargingCharacteristics", "camel.CAMEL_AChBillingChargingCharacteristics",
        FT_UINT32, BASE_DEC,  VALS(camel_CAMEL_AChBillingChargingCharacteristics_vals), 0,
        NULL, HFILL }},

    { &hf_camel_CAMEL_FCIBillingChargingCharacteristics,
      { "CAMEL-FCIBillingChargingCharacteristics", "camel.CAMEL_FCIBillingChargingCharacteristics",
        FT_UINT32, BASE_DEC, VALS(camel_CAMEL_FCIBillingChargingCharacteristics_vals), 0,
        NULL, HFILL }},

    { &hf_camel_CAMEL_FCIGPRSBillingChargingCharacteristics,
      { "CAMEL-FCIGPRSBillingChargingCharacteristics", "camel.CAMEL_FCIGPRSBillingChargingCharacteristics",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},

    { &hf_camel_CAMEL_FCISMSBillingChargingCharacteristics,
      { "CAMEL-FCISMSBillingChargingCharacteristics", "camel.CAMEL_FCISMSBillingChargingCharacteristics",
        FT_UINT32, BASE_DEC, VALS(camel_CAMEL_FCISMSBillingChargingCharacteristics_vals), 0,
        NULL, HFILL }},

    { &hf_camel_CAMEL_SCIBillingChargingCharacteristics,
      { "CAMEL-SCIBillingChargingCharacteristics", "camel.CAMEL_SCIBillingChargingCharacteristics",
        FT_UINT32, BASE_DEC, VALS(camel_CAMEL_SCIBillingChargingCharacteristics_vals), 0,
        NULL, HFILL }},

    { &hf_camel_CAMEL_SCIGPRSBillingChargingCharacteristics,
      { "CAMEL-SCIGPRSBillingChargingCharacteristics", "camel.CAMEL_SCIGPRSBillingChargingCharacteristics",
        FT_UINT32, BASE_DEC, NULL, 0,
        "CAMEL-FSCIGPRSBillingChargingCharacteristics", HFILL }},

    { &hf_camel_CAMEL_CallResult,
      { "CAMEL-CAMEL_CallResult", "camel.CAMEL_CallResult",
        FT_UINT32, BASE_DEC, VALS(camel_CAMEL_CallResult_vals), 0,
        "CAMEL-CallResult", HFILL }},

  /* Camel Service Response Time */
    { &hf_camelsrt_SessionId,
      { "Session Id",
        "camel.srt.session_id",
        FT_UINT32, BASE_DEC, NULL, 0x0,
        NULL, HFILL }
    },
    //{ &hf_camelsrt_RequestNumber,
    //  { "Request Number",
    //    "camel.srt.request_number",
    //    FT_UINT64, BASE_DEC, NULL, 0x0,
    //    NULL, HFILL }
    //},
    { &hf_camelsrt_Duplicate,
      { "Request Duplicate",
        "camel.srt.duplicate",
        FT_UINT32, BASE_DEC, NULL, 0x0,
        NULL, HFILL }
    },
    { &hf_camelsrt_RequestFrame,
      { "Requested Frame",
        "camel.srt.reqframe",
        FT_FRAMENUM, BASE_NONE, FRAMENUM_TYPE(FT_FRAMENUM_REQUEST), 0x0,
        "SRT Request Frame", HFILL }
    },
    { &hf_camelsrt_ResponseFrame,
      { "Response Frame",
        "camel.srt.rspframe",
        FT_FRAMENUM, BASE_NONE, FRAMENUM_TYPE(FT_FRAMENUM_RESPONSE), 0x0,
        "SRT Response Frame", HFILL }
    },
    //{ &hf_camelsrt_DeltaTime,
    //  { "Service Response Time",
    //    "camel.srt.deltatime",
    //    FT_RELATIVE_TIME, BASE_NONE, NULL, 0x0,
    //    "DeltaTime between Request and Response", HFILL }
    //},
    //{ &hf_camelsrt_SessionTime,
    //  { "Session duration",
    //    "camel.srt.sessiontime",
    //    FT_RELATIVE_TIME, BASE_NONE, NULL, 0x0,
    //    "Duration of the TCAP session", HFILL }
    //},
    { &hf_camelsrt_DeltaTime31,
      { "Service Response Time",
        "camel.srt.deltatime31",
        FT_RELATIVE_TIME, BASE_NONE, NULL, 0x0,
        "DeltaTime between InitialDP and Continue", HFILL }
    },
    { &hf_camelsrt_DeltaTime65,
      { "Service Response Time",
        "camel.srt.deltatime65",
        FT_RELATIVE_TIME, BASE_NONE, NULL, 0x0,
        "DeltaTime between InitialDPSMS and ContinueSMS", HFILL }
    },
    { &hf_camelsrt_DeltaTime75,
      { "Service Response Time",
        "camel.srt.deltatime75",
        FT_RELATIVE_TIME, BASE_NONE, NULL, 0x0,
        "DeltaTime between InitialDPGPRS and ContinueGPRS", HFILL }
    },
    { &hf_camelsrt_DeltaTime35,
      { "Service Response Time",
        "camel.srt.deltatime35",
        FT_RELATIVE_TIME, BASE_NONE, NULL, 0x0,
        "DeltaTime between ApplyChargingReport and ApplyCharging", HFILL }
    },
    { &hf_camelsrt_DeltaTime22,
      { "Service Response Time",
        "camel.srt.deltatime22",
        FT_RELATIVE_TIME, BASE_NONE, NULL, 0x0,
        "DeltaTime between EventReport(Disconnect) and Release Call", HFILL }
    },
    { &hf_camelsrt_DeltaTime80,
      { "Service Response Time",
        "camel.srt.deltatime80",
        FT_RELATIVE_TIME, BASE_NONE, NULL, 0x0,
        "DeltaTime between EventReportGPRS and ContinueGPRS", HFILL }
    },
    { &hf_camel_timeandtimezone_time,
      { "Time",
        "camel.timeandtimezone.time",
        FT_STRING, BASE_NONE, NULL, 0x0,
        NULL, HFILL }
    },
    { &hf_camel_timeandtimezone_tz,
      { "Time Zone",
        "camel.timeandtimezone.timezone",
        FT_INT8, BASE_DEC, NULL, 0x0,
        "Difference, expressed in quarters of an hour, between local time and GMT", HFILL }
    },
#ifdef REMOVED
#endif
    { &hf_camel_PAR_cancelFailed_PDU,
      { "PAR-cancelFailed", "camel.PAR_cancelFailed_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_PAR_requestedInfoError_PDU,
      { "PAR-requestedInfoError", "camel.PAR_requestedInfoError",
        FT_UINT32, BASE_DEC, VALS(camel_PAR_requestedInfoError_vals), 0,
        NULL, HFILL }},
    { &hf_camel_UnavailableNetworkResource_PDU,
      { "UnavailableNetworkResource", "camel.UnavailableNetworkResource",
        FT_UINT32, BASE_DEC, VALS(camel_UnavailableNetworkResource_vals), 0,
        NULL, HFILL }},
    { &hf_camel_PAR_taskRefused_PDU,
      { "PAR-taskRefused", "camel.PAR_taskRefused",
        FT_UINT32, BASE_DEC, VALS(camel_PAR_taskRefused_vals), 0,
        NULL, HFILL }},
    { &hf_camel_CAP_GPRS_ReferenceNumber_PDU,
      { "CAP-GPRS-ReferenceNumber", "camel.CAP_GPRS_ReferenceNumber_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_PlayAnnouncementArg_PDU,
      { "PlayAnnouncementArg", "camel.PlayAnnouncementArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_PromptAndCollectUserInformationArg_PDU,
      { "PromptAndCollectUserInformationArg", "camel.PromptAndCollectUserInformationArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ReceivedInformationArg_PDU,
      { "ReceivedInformationArg", "camel.ReceivedInformationArg",
        FT_UINT32, BASE_DEC, VALS(camel_ReceivedInformationArg_vals), 0,
        NULL, HFILL }},
    { &hf_camel_SpecializedResourceReportArg_PDU,
      { "SpecializedResourceReportArg", "camel.SpecializedResourceReportArg",
        FT_UINT32, BASE_DEC, VALS(camel_SpecializedResourceReportArg_vals), 0,
        NULL, HFILL }},
    { &hf_camel_ApplyChargingArg_PDU,
      { "ApplyChargingArg", "camel.ApplyChargingArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ApplyChargingReportArg_PDU,
      { "ApplyChargingReportArg", "camel.ApplyChargingReportArg",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_AssistRequestInstructionsArg_PDU,
      { "AssistRequestInstructionsArg", "camel.AssistRequestInstructionsArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_CallGapArg_PDU,
      { "CallGapArg", "camel.CallGapArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_CallInformationReportArg_PDU,
      { "CallInformationReportArg", "camel.CallInformationReportArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_CallInformationRequestArg_PDU,
      { "CallInformationRequestArg", "camel.CallInformationRequestArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_CancelArg_PDU,
      { "CancelArg", "camel.CancelArg",
        FT_UINT32, BASE_DEC, VALS(camel_CancelArg_vals), 0,
        NULL, HFILL }},
    { &hf_camel_CollectInformationArg_PDU,
      { "CollectInformationArg", "camel.CollectInformationArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ConnectArg_PDU,
      { "ConnectArg", "camel.ConnectArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ConnectToResourceArg_PDU,
      { "ConnectToResourceArg", "camel.ConnectToResourceArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ContinueWithArgumentArg_PDU,
      { "ContinueWithArgumentArg", "camel.ContinueWithArgumentArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_DisconnectForwardConnectionWithArgumentArg_PDU,
      { "DisconnectForwardConnectionWithArgumentArg", "camel.DisconnectForwardConnectionWithArgumentArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_DisconnectLegArg_PDU,
      { "DisconnectLegArg", "camel.DisconnectLegArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_EntityReleasedArg_PDU,
      { "EntityReleasedArg", "camel.EntityReleasedArg",
        FT_UINT32, BASE_DEC, VALS(camel_EntityReleasedArg_vals), 0,
        NULL, HFILL }},
    { &hf_camel_EstablishTemporaryConnectionArg_PDU,
      { "EstablishTemporaryConnectionArg", "camel.EstablishTemporaryConnectionArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_EventReportBCSMArg_PDU,
      { "EventReportBCSMArg", "camel.EventReportBCSMArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_FurnishChargingInformationArg_PDU,
      { "FurnishChargingInformationArg", "camel.FurnishChargingInformationArg",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_InitialDPArg_PDU,
      { "InitialDPArg", "camel.InitialDPArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_InitiateCallAttemptArg_PDU,
      { "InitiateCallAttemptArg", "camel.InitiateCallAttemptArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_InitiateCallAttemptRes_PDU,
      { "InitiateCallAttemptRes", "camel.InitiateCallAttemptRes_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_MoveLegArg_PDU,
      { "MoveLegArg", "camel.MoveLegArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_PlayToneArg_PDU,
      { "PlayToneArg", "camel.PlayToneArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ReleaseCallArg_PDU,
      { "ReleaseCallArg", "camel.ReleaseCallArg",
        FT_UINT32, BASE_DEC, VALS(camel_ReleaseCallArg_vals), 0,
        NULL, HFILL }},
    { &hf_camel_RequestReportBCSMEventArg_PDU,
      { "RequestReportBCSMEventArg", "camel.RequestReportBCSMEventArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ResetTimerArg_PDU,
      { "ResetTimerArg", "camel.ResetTimerArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_SendChargingInformationArg_PDU,
      { "SendChargingInformationArg", "camel.SendChargingInformationArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_SplitLegArg_PDU,
      { "SplitLegArg", "camel.SplitLegArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ApplyChargingGPRSArg_PDU,
      { "ApplyChargingGPRSArg", "camel.ApplyChargingGPRSArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ApplyChargingReportGPRSArg_PDU,
      { "ApplyChargingReportGPRSArg", "camel.ApplyChargingReportGPRSArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_CancelGPRSArg_PDU,
      { "CancelGPRSArg", "camel.CancelGPRSArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ConnectGPRSArg_PDU,
      { "ConnectGPRSArg", "camel.ConnectGPRSArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ContinueGPRSArg_PDU,
      { "ContinueGPRSArg", "camel.ContinueGPRSArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_EntityReleasedGPRSArg_PDU,
      { "EntityReleasedGPRSArg", "camel.EntityReleasedGPRSArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_EventReportGPRSArg_PDU,
      { "EventReportGPRSArg", "camel.EventReportGPRSArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_FurnishChargingInformationGPRSArg_PDU,
      { "FurnishChargingInformationGPRSArg", "camel.FurnishChargingInformationGPRSArg",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_InitialDPGPRSArg_PDU,
      { "InitialDPGPRSArg", "camel.InitialDPGPRSArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ReleaseGPRSArg_PDU,
      { "ReleaseGPRSArg", "camel.ReleaseGPRSArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_RequestReportGPRSEventArg_PDU,
      { "RequestReportGPRSEventArg", "camel.RequestReportGPRSEventArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ResetTimerGPRSArg_PDU,
      { "ResetTimerGPRSArg", "camel.ResetTimerGPRSArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_SendChargingInformationGPRSArg_PDU,
      { "SendChargingInformationGPRSArg", "camel.SendChargingInformationGPRSArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ConnectSMSArg_PDU,
      { "ConnectSMSArg", "camel.ConnectSMSArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_EventReportSMSArg_PDU,
      { "EventReportSMSArg", "camel.EventReportSMSArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_FurnishChargingInformationSMSArg_PDU,
      { "FurnishChargingInformationSMSArg", "camel.FurnishChargingInformationSMSArg",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_InitialDPSMSArg_PDU,
      { "InitialDPSMSArg", "camel.InitialDPSMSArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ReleaseSMSArg_PDU,
      { "ReleaseSMSArg", "camel.ReleaseSMSArg",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_RequestReportSMSEventArg_PDU,
      { "RequestReportSMSEventArg", "camel.RequestReportSMSEventArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ResetTimerSMSArg_PDU,
      { "ResetTimerSMSArg", "camel.ResetTimerSMSArg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_CAP_U_ABORT_REASON_PDU,
      { "CAP-U-ABORT-REASON", "camel.CAP_U_ABORT_REASON",
        FT_UINT32, BASE_DEC, VALS(camel_CAP_U_ABORT_REASON_vals), 0,
        NULL, HFILL }},
    { &hf_camel_legID,
      { "legID", "camel.legID",
        FT_UINT32, BASE_DEC, VALS(inap_LegID_vals), 0,
        NULL, HFILL }},
    { &hf_camel_srfConnection,
      { "srfConnection", "camel.srfConnection",
        FT_UINT32, BASE_DEC, NULL, 0,
        "CallSegmentID", HFILL }},
    { &hf_camel_aOCInitial,
      { "aOCInitial", "camel.aOCInitial_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "CAI_GSM0224", HFILL }},
    { &hf_camel_aOCSubsequent,
      { "aOCSubsequent", "camel.aOCSubsequent_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_cAI_GSM0224,
      { "cAI-GSM0224", "camel.cAI_GSM0224_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_aocSubsequent_tariffSwitchInterval,
      { "tariffSwitchInterval", "camel.aocSubsequent_tariffSwitchInterval",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_86400", HFILL }},
    { &hf_camel_audibleIndicatorTone,
      { "tone", "camel.audibleIndicatorTone",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_camel_burstList,
      { "burstList", "camel.burstList_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_conferenceTreatmentIndicator,
      { "conferenceTreatmentIndicator", "camel.conferenceTreatmentIndicator",
        FT_BYTES, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_1", HFILL }},
    { &hf_camel_callCompletionTreatmentIndicator,
      { "callCompletionTreatmentIndicator", "camel.callCompletionTreatmentIndicator",
        FT_BYTES, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_1", HFILL }},
    { &hf_camel_calledAddressValue,
      { "calledAddressValue", "camel.calledAddressValue",
        FT_BYTES, BASE_NONE, NULL, 0,
        "Digits", HFILL }},
    { &hf_camel_gapOnService,
      { "gapOnService", "camel.gapOnService_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_calledAddressAndService,
      { "calledAddressAndService", "camel.calledAddressAndService_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_serviceKey,
      { "serviceKey", "camel.serviceKey",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_callingAddressAndService,
      { "callingAddressAndService", "camel.callingAddressAndService_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_callingAddressValue,
      { "callingAddressValue", "camel.callingAddressValue",
        FT_BYTES, BASE_NONE, NULL, 0,
        "Digits", HFILL }},
    { &hf_camel_eventTypeBCSM,
      { "eventTypeBCSM", "camel.eventTypeBCSM",
        FT_UINT32, BASE_DEC, VALS(camel_EventTypeBCSM_vals), 0,
        NULL, HFILL }},
    { &hf_camel_monitorMode,
      { "monitorMode", "camel.monitorMode",
        FT_UINT32, BASE_DEC, VALS(camel_MonitorMode_vals), 0,
        NULL, HFILL }},
    { &hf_camel_dpSpecificCriteria,
      { "dpSpecificCriteria", "camel.dpSpecificCriteria",
        FT_UINT32, BASE_DEC, VALS(camel_DpSpecificCriteria_vals), 0,
        NULL, HFILL }},
    { &hf_camel_automaticRearm,
      { "automaticRearm", "camel.automaticRearm_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_cause,
      { "cause", "camel.cause",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_bearerCap,
      { "bearerCap", "camel.bearerCap",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_numberOfBursts,
      { "numberOfBursts", "camel.numberOfBursts",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_3", HFILL }},
    { &hf_camel_burstInterval,
      { "burstInterval", "camel.burstInterval",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_1200", HFILL }},
    { &hf_camel_numberOfTonesInBurst,
      { "numberOfTonesInBurst", "camel.numberOfTonesInBurst",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_3", HFILL }},
    { &hf_camel_burstToneDuration,
      { "toneDuration", "camel.burstToneDuration",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_20", HFILL }},
    { &hf_camel_toneInterval,
      { "toneInterval", "camel.toneInterval",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_20", HFILL }},
    { &hf_camel_warningPeriod,
      { "warningPeriod", "camel.warningPeriod",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_1200", HFILL }},
    { &hf_camel_bursts,
      { "bursts", "camel.bursts_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "Burst", HFILL }},
    { &hf_camel_e1,
      { "e1", "camel.e1",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_8191", HFILL }},
    { &hf_camel_e2,
      { "e2", "camel.e2",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_8191", HFILL }},
    { &hf_camel_e3,
      { "e3", "camel.e3",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_8191", HFILL }},
    { &hf_camel_e4,
      { "e4", "camel.e4",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_8191", HFILL }},
    { &hf_camel_e5,
      { "e5", "camel.e5",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_8191", HFILL }},
    { &hf_camel_e6,
      { "e6", "camel.e6",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_8191", HFILL }},
    { &hf_camel_e7,
      { "e7", "camel.e7",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_8191", HFILL }},
    { &hf_camel_callSegmentID,
      { "callSegmentID", "camel.callSegmentID",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_invokeID,
      { "invokeID", "camel.invokeID",
        FT_INT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_timeDurationCharging,
      { "timeDurationCharging", "camel.timeDurationCharging_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_maxCallPeriodDuration,
      { "maxCallPeriodDuration", "camel.maxCallPeriodDuration",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_864000", HFILL }},
    { &hf_camel_releaseIfdurationExceeded,
      { "releaseIfdurationExceeded", "camel.releaseIfdurationExceeded",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_camel_timeDurationCharging_tariffSwitchInterval,
      { "tariffSwitchInterval", "camel.timeDurationCharging_tariffSwitchInterval",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_86400", HFILL }},
    { &hf_camel_audibleIndicator,
      { "audibleIndicator", "camel.audibleIndicator",
        FT_UINT32, BASE_DEC, VALS(camel_AudibleIndicator_vals), 0,
        NULL, HFILL }},
    { &hf_camel_extensions,
      { "extensions", "camel.extensions",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_timeDurationChargingResult,
      { "timeDurationChargingResult", "camel.timeDurationChargingResult_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_timeDurationChargingResultpartyToCharge,
      { "partyToCharge", "camel.timeDurationChargingResultpartyToCharge",
        FT_UINT32, BASE_DEC, VALS(camel_ReceivingSideID_vals), 0,
        "ReceivingSideID", HFILL }},
    { &hf_camel_timeInformation,
      { "timeInformation", "camel.timeInformation",
        FT_UINT32, BASE_DEC, VALS(camel_TimeInformation_vals), 0,
        NULL, HFILL }},
    { &hf_camel_legActive,
      { "legActive", "camel.legActive",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_camel_callLegReleasedAtTcpExpiry,
      { "callLegReleasedAtTcpExpiry", "camel.callLegReleasedAtTcpExpiry_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_aChChargingAddress,
      { "aChChargingAddress", "camel.aChChargingAddress",
        FT_UINT32, BASE_DEC, VALS(camel_AChChargingAddress_vals), 0,
        NULL, HFILL }},
    { &hf_camel_fci_fCIBCCCAMELsequence1,
      { "fCIBCCCAMELsequence1", "camel.fci_fCIBCCCAMELsequence1_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "T_fci_fCIBCCCAMELsequence1", HFILL }},
    { &hf_camel_freeFormatData,
      { "freeFormatData", "camel.freeFormatData",
        FT_BYTES, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_bound__minFCIBillingChargingDataLength_bound__maxFCIBillingChargingDataLength", HFILL }},
    { &hf_camel_fCIBCCCAMELsequence1partyToCharge,
      { "partyToCharge", "camel.fCIBCCCAMELsequence1partyToCharge",
        FT_UINT32, BASE_DEC, VALS(camel_SendingSideID_vals), 0,
        "SendingSideID", HFILL }},
    { &hf_camel_appendFreeFormatData,
      { "appendFreeFormatData", "camel.appendFreeFormatData",
        FT_UINT32, BASE_DEC, VALS(camel_AppendFreeFormatData_vals), 0,
        NULL, HFILL }},
    { &hf_camel_fciGPRS_fCIBCCCAMELsequence1,
      { "fCIBCCCAMELsequence1", "camel.fciGPRS_fCIBCCCAMELsequence1_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "T_fciGPRS_fCIBCCCAMELsequence1", HFILL }},
    { &hf_camel_pDPID,
      { "pDPID", "camel.pDPID",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_fciSMS_fCIBCCCAMELsequence1,
      { "fCIBCCCAMELsequence1", "camel.fciSMS_fCIBCCCAMELsequence1_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "T_fciSMS_fCIBCCCAMELsequence1", HFILL }},
    { &hf_camel_aOCBeforeAnswer,
      { "aOCBeforeAnswer", "camel.aOCBeforeAnswer_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_aOCAfterAnswer,
      { "aOCAfterAnswer", "camel.aOCAfterAnswer_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "AOCSubsequent", HFILL }},
    { &hf_camel_aOC_extension,
      { "aOC-extension", "camel.aOC_extension_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "CAMEL_SCIBillingChargingCharacteristicsAlt", HFILL }},
    { &hf_camel_aOCGPRS,
      { "aOCGPRS", "camel.aOCGPRS_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ChangeOfPositionControlInfo_item,
      { "ChangeOfLocation", "camel.ChangeOfLocation",
        FT_UINT32, BASE_DEC, VALS(camel_ChangeOfLocation_vals), 0,
        NULL, HFILL }},
    { &hf_camel_cellGlobalId,
      { "cellGlobalId", "camel.cellGlobalId",
        FT_BYTES, BASE_NONE, NULL, 0,
        "CellGlobalIdOrServiceAreaIdFixedLength", HFILL }},
    { &hf_camel_serviceAreaId,
      { "serviceAreaId", "camel.serviceAreaId",
        FT_BYTES, BASE_NONE, NULL, 0,
        "CellGlobalIdOrServiceAreaIdFixedLength", HFILL }},
    { &hf_camel_locationAreaId,
      { "locationAreaId", "camel.locationAreaId",
        FT_BYTES, BASE_NONE, NULL, 0,
        "LAIFixedLength", HFILL }},
    { &hf_camel_inter_SystemHandOver,
      { "inter-SystemHandOver", "camel.inter_SystemHandOver_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_inter_PLMNHandOver,
      { "inter-PLMNHandOver", "camel.inter_PLMNHandOver_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_inter_MSCHandOver,
      { "inter-MSCHandOver", "camel.inter_MSCHandOver_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_changeOfLocationAlt,
      { "changeOfLocationAlt", "camel.changeOfLocationAlt_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_maxTransferredVolume,
      { "maxTransferredVolume", "camel.maxTransferredVolume",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_4294967295", HFILL }},
    { &hf_camel_maxElapsedTime,
      { "maxElapsedTime", "camel.maxElapsedTime",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_86400", HFILL }},
    { &hf_camel_transferredVolume,
      { "transferredVolume", "camel.transferredVolume",
        FT_UINT32, BASE_DEC, VALS(camel_TransferredVolume_vals), 0,
        NULL, HFILL }},
    { &hf_camel_elapsedTime,
      { "elapsedTime", "camel.elapsedTime",
        FT_UINT32, BASE_DEC, VALS(camel_ElapsedTime_vals), 0,
        NULL, HFILL }},
    { &hf_camel_transferredVolumeRollOver,
      { "transferredVolumeRollOver", "camel.transferredVolumeRollOver",
        FT_UINT32, BASE_DEC, VALS(camel_TransferredVolumeRollOver_vals), 0,
        NULL, HFILL }},
    { &hf_camel_elapsedTimeRollOver,
      { "elapsedTimeRollOver", "camel.elapsedTimeRollOver",
        FT_UINT32, BASE_DEC, VALS(camel_ElapsedTimeRollOver_vals), 0,
        NULL, HFILL }},
    { &hf_camel_minimumNbOfDigits,
      { "minimumNbOfDigits", "camel.minimumNbOfDigits",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_30", HFILL }},
    { &hf_camel_maximumNbOfDigits,
      { "maximumNbOfDigits", "camel.maximumNbOfDigits",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_30", HFILL }},
    { &hf_camel_endOfReplyDigit,
      { "endOfReplyDigit", "camel.endOfReplyDigit",
        FT_BYTES, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_1_2", HFILL }},
    { &hf_camel_cancelDigit,
      { "cancelDigit", "camel.cancelDigit",
        FT_BYTES, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_1_2", HFILL }},
    { &hf_camel_startDigit,
      { "startDigit", "camel.startDigit",
        FT_BYTES, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_1_2", HFILL }},
    { &hf_camel_firstDigitTimeOut,
      { "firstDigitTimeOut", "camel.firstDigitTimeOut",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_127", HFILL }},
    { &hf_camel_interDigitTimeOut,
      { "interDigitTimeOut", "camel.interDigitTimeOut",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_127", HFILL }},
    { &hf_camel_errorTreatment,
      { "errorTreatment", "camel.errorTreatment",
        FT_UINT32, BASE_DEC, VALS(camel_ErrorTreatment_vals), 0,
        NULL, HFILL }},
    { &hf_camel_interruptableAnnInd,
      { "interruptableAnnInd", "camel.interruptableAnnInd",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_camel_voiceInformation,
      { "voiceInformation", "camel.voiceInformation",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_camel_voiceBack,
      { "voiceBack", "camel.voiceBack",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_camel_collectedDigits,
      { "collectedDigits", "camel.collectedDigits_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_basicGapCriteria,
      { "basicGapCriteria", "camel.basicGapCriteria",
        FT_UINT32, BASE_DEC, VALS(camel_BasicGapCriteria_vals), 0,
        NULL, HFILL }},
    { &hf_camel_scfID,
      { "scfID", "camel.scfID",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_DestinationRoutingAddress_item,
      { "CalledPartyNumber", "camel.CalledPartyNumber",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_applicationTimer,
      { "applicationTimer", "camel.applicationTimer",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_midCallControlInfo,
      { "midCallControlInfo", "camel.midCallControlInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_dpSpecificCriteriaAlt,
      { "dpSpecificCriteriaAlt", "camel.dpSpecificCriteriaAlt_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_changeOfPositionControlInfo,
      { "changeOfPositionControlInfo", "camel.changeOfPositionControlInfo",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_numberOfDigits,
      { "numberOfDigits", "camel.numberOfDigits",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_interDigitTimeout,
      { "interDigitTimeout", "camel.interDigitTimeout",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_127", HFILL }},
    { &hf_camel_oServiceChangeSpecificInfo,
      { "oServiceChangeSpecificInfo", "camel.oServiceChangeSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ext_basicServiceCode,
      { "ext-basicServiceCode", "camel.ext_basicServiceCode",
        FT_UINT32, BASE_DEC, VALS(gsm_map_Ext_BasicServiceCode_vals), 0,
        NULL, HFILL }},
    { &hf_camel_initiatorOfServiceChange,
      { "initiatorOfServiceChange", "camel.initiatorOfServiceChange",
        FT_UINT32, BASE_DEC, VALS(camel_InitiatorOfServiceChange_vals), 0,
        NULL, HFILL }},
    { &hf_camel_natureOfServiceChange,
      { "natureOfServiceChange", "camel.natureOfServiceChange",
        FT_UINT32, BASE_DEC, VALS(camel_NatureOfServiceChange_vals), 0,
        NULL, HFILL }},
    { &hf_camel_tServiceChangeSpecificInfo,
      { "tServiceChangeSpecificInfo", "camel.tServiceChangeSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_collectedInfoSpecificInfo,
      { "collectedInfoSpecificInfo", "camel.collectedInfoSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_calledPartyNumber,
      { "calledPartyNumber", "camel.calledPartyNumber",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_timeGPRSIfNoTariffSwitch,
      { "timeGPRSIfNoTariffSwitch", "camel.timeGPRSIfNoTariffSwitch",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_86400", HFILL }},
    { &hf_camel_timeGPRSIfTariffSwitch,
      { "timeGPRSIfTariffSwitch", "camel.timeGPRSIfTariffSwitch_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_timeGPRSSinceLastTariffSwitch,
      { "timeGPRSSinceLastTariffSwitch", "camel.timeGPRSSinceLastTariffSwitch",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_86400", HFILL }},
    { &hf_camel_timeGPRSTariffSwitchInterval,
      { "timeGPRSTariffSwitchInterval", "camel.timeGPRSTariffSwitchInterval",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_86400", HFILL }},
    { &hf_camel_rO_TimeGPRSIfNoTariffSwitch,
      { "rO-TimeGPRSIfNoTariffSwitch", "camel.rO_TimeGPRSIfNoTariffSwitch",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_camel_rO_TimeGPRSIfTariffSwitch,
      { "rO-TimeGPRSIfTariffSwitch", "camel.rO_TimeGPRSIfTariffSwitch_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_rO_TimeGPRSSinceLastTariffSwitch,
      { "rO-TimeGPRSSinceLastTariffSwitch", "camel.rO_TimeGPRSSinceLastTariffSwitch",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_camel_rO_TimeGPRSTariffSwitchInterval,
      { "rO-TimeGPRSTariffSwitchInterval", "camel.rO_TimeGPRSTariffSwitchInterval",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_camel_pDPTypeOrganization,
      { "pDPTypeOrganization", "camel.pDPTypeOrganization",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_pDPTypeNumber,
      { "pDPTypeNumber", "camel.pDPTypeNumber",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_pDPAddress,
      { "pDPAddress", "camel.pDPAddress",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_routeSelectFailureSpecificInfo,
      { "routeSelectFailureSpecificInfo", "camel.routeSelectFailureSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_routeSelectfailureCause,
      { "failureCause", "camel.routeSelectfailureCause",
        FT_BYTES, BASE_NONE, NULL, 0,
        "Cause", HFILL }},
    { &hf_camel_oCalledPartyBusySpecificInfo,
      { "oCalledPartyBusySpecificInfo", "camel.oCalledPartyBusySpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_busyCause,
      { "busyCause", "camel.busyCause",
        FT_BYTES, BASE_NONE, NULL, 0,
        "Cause", HFILL }},
    { &hf_camel_oNoAnswerSpecificInfo,
      { "oNoAnswerSpecificInfo", "camel.oNoAnswerSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_oAnswerSpecificInfo,
      { "oAnswerSpecificInfo", "camel.oAnswerSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_destinationAddress,
      { "destinationAddress", "camel.destinationAddress",
        FT_BYTES, BASE_NONE, NULL, 0,
        "CalledPartyNumber", HFILL }},
    { &hf_camel_or_Call,
      { "or-Call", "camel.or_Call_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_forwardedCall,
      { "forwardedCall", "camel.forwardedCall_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_chargeIndicator,
      { "chargeIndicator", "camel.chargeIndicator",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_ext_basicServiceCode2,
      { "ext-basicServiceCode2", "camel.ext_basicServiceCode2",
        FT_UINT32, BASE_DEC, VALS(gsm_map_Ext_BasicServiceCode_vals), 0,
        "Ext_BasicServiceCode", HFILL }},
    { &hf_camel_oMidCallSpecificInfo,
      { "oMidCallSpecificInfo", "camel.oMidCallSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_omidCallEvents,
      { "midCallEvents", "camel.omidCallEvents",
        FT_UINT32, BASE_DEC, VALS(camel_T_omidCallEvents_vals), 0,
        "T_omidCallEvents", HFILL }},
    { &hf_camel_dTMFDigitsCompleted,
      { "dTMFDigitsCompleted", "camel.dTMFDigitsCompleted",
        FT_BYTES, BASE_NONE, NULL, 0,
        "Digits", HFILL }},
    { &hf_camel_dTMFDigitsTimeOut,
      { "dTMFDigitsTimeOut", "camel.dTMFDigitsTimeOut",
        FT_BYTES, BASE_NONE, NULL, 0,
        "Digits", HFILL }},
    { &hf_camel_oDisconnectSpecificInfo,
      { "oDisconnectSpecificInfo", "camel.oDisconnectSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_releaseCause,
      { "releaseCause", "camel.releaseCause",
        FT_BYTES, BASE_NONE, NULL, 0,
        "Cause", HFILL }},
    { &hf_camel_tBusySpecificInfo,
      { "tBusySpecificInfo", "camel.tBusySpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_callForwarded,
      { "callForwarded", "camel.callForwarded_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_routeNotPermitted,
      { "routeNotPermitted", "camel.routeNotPermitted_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_forwardingDestinationNumber,
      { "forwardingDestinationNumber", "camel.forwardingDestinationNumber",
        FT_BYTES, BASE_NONE, NULL, 0,
        "CalledPartyNumber", HFILL }},
    { &hf_camel_tNoAnswerSpecificInfo,
      { "tNoAnswerSpecificInfo", "camel.tNoAnswerSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_tAnswerSpecificInfo,
      { "tAnswerSpecificInfo", "camel.tAnswerSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_tMidCallSpecificInfo,
      { "tMidCallSpecificInfo", "camel.tMidCallSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_tmidCallEvents,
      { "midCallEvents", "camel.tmidCallEvents",
        FT_UINT32, BASE_DEC, VALS(camel_T_tmidCallEvents_vals), 0,
        "T_tmidCallEvents", HFILL }},
    { &hf_camel_tDisconnectSpecificInfo,
      { "tDisconnectSpecificInfo", "camel.tDisconnectSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_oTermSeizedSpecificInfo,
      { "oTermSeizedSpecificInfo", "camel.oTermSeizedSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_locationInformation,
      { "locationInformation", "camel.locationInformation_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_callAcceptedSpecificInfo,
      { "callAcceptedSpecificInfo", "camel.callAcceptedSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_oAbandonSpecificInfo,
      { "oAbandonSpecificInfo", "camel.oAbandonSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_oChangeOfPositionSpecificInfo,
      { "oChangeOfPositionSpecificInfo", "camel.oChangeOfPositionSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_metDPCriteriaList,
      { "metDPCriteriaList", "camel.metDPCriteriaList",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_tChangeOfPositionSpecificInfo,
      { "tChangeOfPositionSpecificInfo", "camel.tChangeOfPositionSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_dpSpecificInfoAlt,
      { "dpSpecificInfoAlt", "camel.dpSpecificInfoAlt_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_o_smsFailureSpecificInfo,
      { "o-smsFailureSpecificInfo", "camel.o_smsFailureSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_mo_smsfailureCause,
      { "failureCause", "camel.mo-smsfailureCause",
        FT_UINT32, BASE_DEC, VALS(camel_MO_SMSCause_vals), 0,
        "MO_SMSCause", HFILL }},
    { &hf_camel_o_smsSubmissionSpecificInfo,
      { "o-smsSubmissionSpecificInfo", "camel.o_smsSubmissionSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_t_smsFailureSpecificInfo,
      { "t-smsFailureSpecificInfo", "camel.t_smsFailureSpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "T_t_smsFailureSpecificInfo", HFILL }},
    { &hf_camel_t_smsfailureCause,
      { "failureCause", "camel.t-smsfailureCause",
        FT_BYTES, BASE_NONE, NULL, 0,
        "MT_SMSCause", HFILL }},
    { &hf_camel_t_smsDeliverySpecificInfo,
      { "t-smsDeliverySpecificInfo", "camel.t_smsDeliverySpecificInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "T_t_smsDeliverySpecificInfo", HFILL }},
    { &hf_camel_Extensions_item,
      { "ExtensionField", "camel.ExtensionField_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_type,
      { "type", "camel.type",
        FT_UINT32, BASE_DEC, VALS(camel_Code_vals), 0,
        "Code", HFILL }},
    { &hf_camel_criticality,
      { "criticality", "camel.criticality",
        FT_UINT32, BASE_DEC, VALS(inap_CriticalityType_vals), 0,
        "CriticalityType", HFILL }},
    { &hf_camel_value,
      { "value", "camel.value_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_callDiversionTreatmentIndicator,
      { "callDiversionTreatmentIndicator", "camel.callDiversionTreatmentIndicator",
        FT_BYTES, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_1", HFILL }},
    { &hf_camel_callingPartyRestrictionIndicator,
      { "callingPartyRestrictionIndicator", "camel.callingPartyRestrictionIndicator",
        FT_BYTES, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_1", HFILL }},
    { &hf_camel_compoundGapCriteria,
      { "compoundGapCriteria", "camel.compoundGapCriteria_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "CompoundCriteria", HFILL }},
    { &hf_camel_gapIndicatorsDuration,
      { "duration", "camel.gapIndicatorsDuration",
        FT_INT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_gapInterval,
      { "gapInterval", "camel.gapInterval",
        FT_INT32, BASE_DEC, NULL, 0,
        "Interval", HFILL }},
    { &hf_camel_informationToSend,
      { "informationToSend", "camel.informationToSend",
        FT_UINT32, BASE_DEC, VALS(camel_InformationToSend_vals), 0,
        NULL, HFILL }},
    { &hf_camel_GenericNumbers_item,
      { "GenericNumber", "camel.GenericNumber",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_short_QoS_format,
      { "short-QoS-format", "camel.short_QoS_format",
        FT_BYTES, BASE_NONE, NULL, 0,
        "QoS_Subscribed", HFILL }},
    { &hf_camel_long_QoS_format,
      { "long-QoS-format", "camel.long_QoS_format",
        FT_BYTES, BASE_NONE, NULL, 0,
        "Ext_QoS_Subscribed", HFILL }},
    { &hf_camel_supplement_to_long_QoS_format,
      { "supplement-to-long-QoS-format", "camel.supplement_to_long_QoS_format",
        FT_BYTES, BASE_NONE, NULL, 0,
        "Ext2_QoS_Subscribed", HFILL }},
    { &hf_camel_additionalSupplement,
      { "additionalSupplement", "camel.additionalSupplement",
        FT_BYTES, BASE_NONE, NULL, 0,
        "Ext3_QoS_Subscribed", HFILL }},
    { &hf_camel_gPRSEventType,
      { "gPRSEventType", "camel.gPRSEventType",
        FT_UINT32, BASE_DEC, VALS(camel_GPRSEventType_vals), 0,
        NULL, HFILL }},
    { &hf_camel_attachChangeOfPositionSpecificInformation,
      { "attachChangeOfPositionSpecificInformation", "camel.attachChangeOfPositionSpecificInformation_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_locationInformationGPRS,
      { "locationInformationGPRS", "camel.locationInformationGPRS_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_pdp_ContextchangeOfPositionSpecificInformation,
      { "pdp-ContextchangeOfPositionSpecificInformation", "camel.pdp_ContextchangeOfPositionSpecificInformation_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_accessPointName,
      { "accessPointName", "camel.accessPointName",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_chargingID,
      { "chargingID", "camel.chargingID",
        FT_BYTES, BASE_NONE, NULL, 0,
        "GPRSChargingID", HFILL }},
    { &hf_camel_endUserAddress,
      { "endUserAddress", "camel.endUserAddress_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_qualityOfService,
      { "qualityOfService", "camel.qualityOfService_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_timeAndTimeZone,
      { "timeAndTimeZone", "camel.timeAndTimeZone",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_gGSNAddress,
      { "gGSNAddress", "camel.gGSNAddress",
        FT_BYTES, BASE_NONE, NULL, 0,
        "GSN_Address", HFILL }},
    { &hf_camel_detachSpecificInformation,
      { "detachSpecificInformation", "camel.detachSpecificInformation_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_initiatingEntity,
      { "initiatingEntity", "camel.initiatingEntity",
        FT_UINT32, BASE_DEC, VALS(camel_InitiatingEntity_vals), 0,
        NULL, HFILL }},
    { &hf_camel_routeingAreaUpdate,
      { "routeingAreaUpdate", "camel.routeingAreaUpdate_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_disconnectSpecificInformation,
      { "disconnectSpecificInformation", "camel.disconnectSpecificInformation_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_pDPContextEstablishmentSpecificInformation,
      { "pDPContextEstablishmentSpecificInformation", "camel.pDPContextEstablishmentSpecificInformation_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_pDPInitiationType,
      { "pDPInitiationType", "camel.pDPInitiationType",
        FT_UINT32, BASE_DEC, VALS(camel_PDPInitiationType_vals), 0,
        NULL, HFILL }},
    { &hf_camel_secondaryPDP_context,
      { "secondaryPDP-context", "camel.secondaryPDP_context_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_pDPContextEstablishmentAcknowledgementSpecificInformation,
      { "pDPContextEstablishmentAcknowledgementSpecificInformation", "camel.pDPContextEstablishmentAcknowledgementSpecificInformation_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_messageID,
      { "messageID", "camel.messageID",
        FT_UINT32, BASE_DEC, VALS(camel_MessageID_vals), 0,
        NULL, HFILL }},
    { &hf_camel_numberOfRepetitions,
      { "numberOfRepetitions", "camel.numberOfRepetitions",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_127", HFILL }},
    { &hf_camel_inbandInfoDuration,
      { "duration", "camel.inbandInfoDuration",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_32767", HFILL }},
    { &hf_camel_interval,
      { "interval", "camel.interval",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_32767", HFILL }},
    { &hf_camel_inbandInfo,
      { "inbandInfo", "camel.inbandInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_tone,
      { "tone", "camel.tone_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_cellGlobalIdOrServiceAreaIdOrLAI,
      { "cellGlobalIdOrServiceAreaIdOrLAI", "camel.cellGlobalIdOrServiceAreaIdOrLAI",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_routeingAreaIdentity,
      { "routeingAreaIdentity", "camel.routeingAreaIdentity",
        FT_BYTES, BASE_NONE, NULL, 0,
        "RAIdentity", HFILL }},
    { &hf_camel_geographicalInformation,
      { "geographicalInformation", "camel.geographicalInformation",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_sgsn_Number,
      { "sgsn-Number", "camel.sgsn_Number",
        FT_BYTES, BASE_NONE, NULL, 0,
        "ISDN_AddressString", HFILL }},
    { &hf_camel_selectedLSAIdentity,
      { "selectedLSAIdentity", "camel.selectedLSAIdentity",
        FT_BYTES, BASE_NONE, NULL, 0,
        "LSAIdentity", HFILL }},
    { &hf_camel_extensionContainer,
      { "extensionContainer", "camel.extensionContainer_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_sai_Present,
      { "sai-Present", "camel.sai_Present_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_userCSGInformation,
      { "userCSGInformation", "camel.userCSGInformation",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_elementaryMessageID,
      { "elementaryMessageID", "camel.elementaryMessageID",
        FT_UINT32, BASE_DEC, NULL, 0,
        "Integer4", HFILL }},
    { &hf_camel_text,
      { "text", "camel.text_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_messageContent,
      { "messageContent", "camel.messageContent",
        FT_STRING, BASE_NONE, NULL, 0,
        "IA5String_SIZE_bound__minMessageContentLength_bound__maxMessageContentLength", HFILL }},
    { &hf_camel_attributes,
      { "attributes", "camel.attributes",
        FT_BYTES, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_bound__minAttributesLength_bound__maxAttributesLength", HFILL }},
    { &hf_camel_elementaryMessageIDs,
      { "elementaryMessageIDs", "camel.elementaryMessageIDs",
        FT_UINT32, BASE_DEC, NULL, 0,
        "SEQUENCE_SIZE_1_bound__numOfMessageIDs_OF_Integer4", HFILL }},
    { &hf_camel_elementaryMessageIDs_item,
      { "Integer4", "camel.Integer4",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_variableMessage,
      { "variableMessage", "camel.variableMessage_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_variableParts,
      { "variableParts", "camel.variableParts",
        FT_UINT32, BASE_DEC, NULL, 0,
        "SEQUENCE_SIZE_1_5_OF_VariablePart", HFILL }},
    { &hf_camel_variableParts_item,
      { "VariablePart", "camel.VariablePart",
        FT_UINT32, BASE_DEC, VALS(camel_VariablePart_vals), 0,
        NULL, HFILL }},
    { &hf_camel_MetDPCriteriaList_item,
      { "MetDPCriterion", "camel.MetDPCriterion",
        FT_UINT32, BASE_DEC, VALS(camel_MetDPCriterion_vals), 0,
        NULL, HFILL }},
    { &hf_camel_enteringCellGlobalId,
      { "enteringCellGlobalId", "camel.enteringCellGlobalId",
        FT_BYTES, BASE_NONE, NULL, 0,
        "CellGlobalIdOrServiceAreaIdFixedLength", HFILL }},
    { &hf_camel_leavingCellGlobalId,
      { "leavingCellGlobalId", "camel.leavingCellGlobalId",
        FT_BYTES, BASE_NONE, NULL, 0,
        "CellGlobalIdOrServiceAreaIdFixedLength", HFILL }},
    { &hf_camel_enteringServiceAreaId,
      { "enteringServiceAreaId", "camel.enteringServiceAreaId",
        FT_BYTES, BASE_NONE, NULL, 0,
        "CellGlobalIdOrServiceAreaIdFixedLength", HFILL }},
    { &hf_camel_leavingServiceAreaId,
      { "leavingServiceAreaId", "camel.leavingServiceAreaId",
        FT_BYTES, BASE_NONE, NULL, 0,
        "CellGlobalIdOrServiceAreaIdFixedLength", HFILL }},
    { &hf_camel_enteringLocationAreaId,
      { "enteringLocationAreaId", "camel.enteringLocationAreaId",
        FT_BYTES, BASE_NONE, NULL, 0,
        "LAIFixedLength", HFILL }},
    { &hf_camel_leavingLocationAreaId,
      { "leavingLocationAreaId", "camel.leavingLocationAreaId",
        FT_BYTES, BASE_NONE, NULL, 0,
        "LAIFixedLength", HFILL }},
    { &hf_camel_inter_SystemHandOverToUMTS,
      { "inter-SystemHandOverToUMTS", "camel.inter_SystemHandOverToUMTS_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_inter_SystemHandOverToGSM,
      { "inter-SystemHandOverToGSM", "camel.inter_SystemHandOverToGSM_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_metDPCriterionAlt,
      { "metDPCriterionAlt", "camel.metDPCriterionAlt_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_minimumNumberOfDigits,
      { "minimumNumberOfDigits", "camel.minimumNumberOfDigits",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_30", HFILL }},
    { &hf_camel_maximumNumberOfDigits,
      { "maximumNumberOfDigits", "camel.maximumNumberOfDigits",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_30", HFILL }},
    { &hf_camel_requested_QoS,
      { "requested-QoS", "camel.requested_QoS",
        FT_UINT32, BASE_DEC, VALS(camel_GPRS_QoS_vals), 0,
        "GPRS_QoS", HFILL }},
    { &hf_camel_subscribed_QoS,
      { "subscribed-QoS", "camel.subscribed_QoS",
        FT_UINT32, BASE_DEC, VALS(camel_GPRS_QoS_vals), 0,
        "GPRS_QoS", HFILL }},
    { &hf_camel_negotiated_QoS,
      { "negotiated-QoS", "camel.negotiated_QoS",
        FT_UINT32, BASE_DEC, VALS(camel_GPRS_QoS_vals), 0,
        "GPRS_QoS", HFILL }},
    { &hf_camel_requested_QoS_Extension,
      { "requested-QoS-Extension", "camel.requested_QoS_Extension_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "GPRS_QoS_Extension", HFILL }},
    { &hf_camel_subscribed_QoS_Extension,
      { "subscribed-QoS-Extension", "camel.subscribed_QoS_Extension_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "GPRS_QoS_Extension", HFILL }},
    { &hf_camel_negotiated_QoS_Extension,
      { "negotiated-QoS-Extension", "camel.negotiated_QoS_Extension_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "GPRS_QoS_Extension", HFILL }},
    { &hf_camel_receivingSideID,
      { "receivingSideID", "camel.receivingSideID",
        FT_BYTES, BASE_NONE, NULL, 0,
        "LegType", HFILL }},
    { &hf_camel_RequestedInformationList_item,
      { "RequestedInformation", "camel.RequestedInformation_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_RequestedInformationTypeList_item,
      { "RequestedInformationType", "camel.RequestedInformationType",
        FT_UINT32, BASE_DEC, VALS(camel_RequestedInformationType_vals), 0,
        NULL, HFILL }},
    { &hf_camel_requestedInformationType,
      { "requestedInformationType", "camel.requestedInformationType",
        FT_UINT32, BASE_DEC, VALS(camel_RequestedInformationType_vals), 0,
        NULL, HFILL }},
    { &hf_camel_requestedInformationValue,
      { "requestedInformationValue", "camel.requestedInformationValue",
        FT_UINT32, BASE_DEC, VALS(camel_RequestedInformationValue_vals), 0,
        NULL, HFILL }},
    { &hf_camel_callAttemptElapsedTimeValue,
      { "callAttemptElapsedTimeValue", "camel.callAttemptElapsedTimeValue",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_camel_callStopTimeValue,
      { "callStopTimeValue", "camel.callStopTimeValue",
        FT_STRING, BASE_NONE, NULL, 0,
        "DateAndTime", HFILL }},
    { &hf_camel_callConnectedElapsedTimeValue,
      { "callConnectedElapsedTimeValue", "camel.callConnectedElapsedTimeValue",
        FT_UINT32, BASE_DEC, NULL, 0,
        "Integer4", HFILL }},
    { &hf_camel_releaseCauseValue,
      { "releaseCauseValue", "camel.releaseCauseValue",
        FT_BYTES, BASE_NONE, NULL, 0,
        "Cause", HFILL }},
    { &hf_camel_sendingSideID,
      { "sendingSideID", "camel.sendingSideID",
        FT_BYTES, BASE_NONE, NULL, 0,
        "LegType", HFILL }},
    { &hf_camel_forwardServiceInteractionInd,
      { "forwardServiceInteractionInd", "camel.forwardServiceInteractionInd_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_backwardServiceInteractionInd,
      { "backwardServiceInteractionInd", "camel.backwardServiceInteractionInd_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_bothwayThroughConnectionInd,
      { "bothwayThroughConnectionInd", "camel.bothwayThroughConnectionInd",
        FT_UINT32, BASE_DEC, VALS(inap_BothwayThroughConnectionInd_vals), 0,
        NULL, HFILL }},
    { &hf_camel_connectedNumberTreatmentInd,
      { "connectedNumberTreatmentInd", "camel.connectedNumberTreatmentInd",
        FT_UINT32, BASE_DEC, VALS(camel_ConnectedNumberTreatmentInd_vals), 0,
        NULL, HFILL }},
    { &hf_camel_nonCUGCall,
      { "nonCUGCall", "camel.nonCUGCall_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_holdTreatmentIndicator,
      { "holdTreatmentIndicator", "camel.holdTreatmentIndicator",
        FT_INT32, BASE_DEC, VALS(camel_holdTreatmentIndicator_values), 0,
        "OCTET_STRING_SIZE_1", HFILL }},
    { &hf_camel_cwTreatmentIndicator,
      { "cwTreatmentIndicator", "camel.cwTreatmentIndicator",
        FT_INT32, BASE_DEC, VALS(camel_cwTreatmentIndicator_values), 0,
        "OCTET_STRING_SIZE_1", HFILL }},
    { &hf_camel_ectTreatmentIndicator,
      { "ectTreatmentIndicator", "camel.ectTreatmentIndicator",
        FT_INT32, BASE_DEC, VALS(camel_ectTreatmentIndicator_values), 0,
        "OCTET_STRING_SIZE_1", HFILL }},
    { &hf_camel_eventTypeSMS,
      { "eventTypeSMS", "camel.eventTypeSMS",
        FT_UINT32, BASE_DEC, VALS(camel_EventTypeSMS_vals), 0,
        NULL, HFILL }},
    { &hf_camel_timeSinceTariffSwitch,
      { "timeSinceTariffSwitch", "camel.timeSinceTariffSwitch",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_864000", HFILL }},
    { &hf_camel_timeIfTariffSwitch_tariffSwitchInterval,
      { "tariffSwitchInterval", "camel.timeIfTariffSwitch_tariffSwitchInterval",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_864000", HFILL }},
    { &hf_camel_timeIfNoTariffSwitch,
      { "timeIfNoTariffSwitch", "camel.timeIfNoTariffSwitch",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_timeIfTariffSwitch,
      { "timeIfTariffSwitch", "camel.timeIfTariffSwitch_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_toneID,
      { "toneID", "camel.toneID",
        FT_UINT32, BASE_DEC, NULL, 0,
        "Integer4", HFILL }},
    { &hf_camel_toneDuration,
      { "duration", "camel.toneDuration",
        FT_UINT32, BASE_DEC, NULL, 0,
        "Integer4", HFILL }},
    { &hf_camel_volumeIfNoTariffSwitch,
      { "volumeIfNoTariffSwitch", "camel.volumeIfNoTariffSwitch",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_4294967295", HFILL }},
    { &hf_camel_volumeIfTariffSwitch,
      { "volumeIfTariffSwitch", "camel.volumeIfTariffSwitch_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_volumeSinceLastTariffSwitch,
      { "volumeSinceLastTariffSwitch", "camel.volumeSinceLastTariffSwitch",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_4294967295", HFILL }},
    { &hf_camel_volumeTariffSwitchInterval,
      { "volumeTariffSwitchInterval", "camel.volumeTariffSwitchInterval",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_4294967295", HFILL }},
    { &hf_camel_rO_VolumeIfNoTariffSwitch,
      { "rO-VolumeIfNoTariffSwitch", "camel.rO_VolumeIfNoTariffSwitch",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_camel_rO_VolumeIfTariffSwitch,
      { "rO-VolumeIfTariffSwitch", "camel.rO_VolumeIfTariffSwitch_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_rO_VolumeSinceLastTariffSwitch,
      { "rO-VolumeSinceLastTariffSwitch", "camel.rO_VolumeSinceLastTariffSwitch",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_camel_rO_VolumeTariffSwitchInterval,
      { "rO-VolumeTariffSwitchInterval", "camel.rO_VolumeTariffSwitchInterval",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_0_255", HFILL }},
    { &hf_camel_integer,
      { "integer", "camel.integer",
        FT_UINT32, BASE_DEC, NULL, 0,
        "Integer4", HFILL }},
    { &hf_camel_number,
      { "number", "camel.number",
        FT_BYTES, BASE_NONE, NULL, 0,
        "Digits", HFILL }},
    { &hf_camel_time,
      { "time", "camel.time",
        FT_BYTES, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_2", HFILL }},
    { &hf_camel_date,
      { "date", "camel.date",
        FT_BYTES, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_4", HFILL }},
    { &hf_camel_price,
      { "price", "camel.price",
        FT_BYTES, BASE_NONE, NULL, 0,
        "OCTET_STRING_SIZE_4", HFILL }},
    { &hf_camel_par_cancelFailedProblem,
      { "problem", "camel.par_cancelFailedProblem",
        FT_UINT32, BASE_DEC, VALS(camel_T_par_cancelFailedProblem_vals), 0,
        "T_par_cancelFailedProblem", HFILL }},
    { &hf_camel_operation,
      { "operation", "camel.operation",
        FT_INT32, BASE_DEC, NULL, 0,
        "InvokeID", HFILL }},
    { &hf_camel_destinationReference,
      { "destinationReference", "camel.destinationReference",
        FT_UINT32, BASE_DEC, NULL, 0,
        "Integer4", HFILL }},
    { &hf_camel_originationReference,
      { "originationReference", "camel.originationReference",
        FT_UINT32, BASE_DEC, NULL, 0,
        "Integer4", HFILL }},
    { &hf_camel_disconnectFromIPForbidden,
      { "disconnectFromIPForbidden", "camel.disconnectFromIPForbidden",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_camel_requestAnnouncementCompleteNotification,
      { "requestAnnouncementCompleteNotification", "camel.requestAnnouncementCompleteNotification",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_camel_requestAnnouncementStartedNotification,
      { "requestAnnouncementStartedNotification", "camel.requestAnnouncementStartedNotification",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_camel_collectedInfo,
      { "collectedInfo", "camel.collectedInfo",
        FT_UINT32, BASE_DEC, VALS(camel_CollectedInfo_vals), 0,
        NULL, HFILL }},
    { &hf_camel_digitsResponse,
      { "digitsResponse", "camel.digitsResponse",
        FT_BYTES, BASE_NONE, NULL, 0,
        "Digits", HFILL }},
    { &hf_camel_allAnnouncementsComplete,
      { "allAnnouncementsComplete", "camel.allAnnouncementsComplete_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_firstAnnouncementStarted,
      { "firstAnnouncementStarted", "camel.firstAnnouncementStarted_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_aChBillingChargingCharacteristics,
      { "aChBillingChargingCharacteristics", "camel.aChBillingChargingCharacteristics",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_partyToCharge,
      { "partyToCharge", "camel.partyToCharge",
        FT_UINT32, BASE_DEC, VALS(camel_SendingSideID_vals), 0,
        "SendingSideID", HFILL }},
    { &hf_camel_iTXcharging,
      { "iTXcharging", "camel.iTXcharging",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_camel_correlationID,
      { "correlationID", "camel.correlationID",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_iPSSPCapabilities,
      { "iPSSPCapabilities", "camel.iPSSPCapabilities",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_gapCriteria,
      { "gapCriteria", "camel.gapCriteria",
        FT_UINT32, BASE_DEC, VALS(camel_GapCriteria_vals), 0,
        NULL, HFILL }},
    { &hf_camel_gapIndicators,
      { "gapIndicators", "camel.gapIndicators_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_controlType,
      { "controlType", "camel.controlType",
        FT_UINT32, BASE_DEC, VALS(camel_ControlType_vals), 0,
        NULL, HFILL }},
    { &hf_camel_gapTreatment,
      { "gapTreatment", "camel.gapTreatment",
        FT_UINT32, BASE_DEC, VALS(camel_GapTreatment_vals), 0,
        NULL, HFILL }},
    { &hf_camel_requestedInformationList,
      { "requestedInformationList", "camel.requestedInformationList",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_legID_01,
      { "legID", "camel.legID",
        FT_UINT32, BASE_DEC, VALS(camel_ReceivingSideID_vals), 0,
        "ReceivingSideID", HFILL }},
    { &hf_camel_requestedInformationTypeList,
      { "requestedInformationTypeList", "camel.requestedInformationTypeList",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_legID_02,
      { "legID", "camel.legID",
        FT_UINT32, BASE_DEC, VALS(camel_SendingSideID_vals), 0,
        "SendingSideID", HFILL }},
    { &hf_camel_allRequests,
      { "allRequests", "camel.allRequests_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_callSegmentToCancel,
      { "callSegmentToCancel", "camel.callSegmentToCancel_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_destinationRoutingAddress,
      { "destinationRoutingAddress", "camel.destinationRoutingAddress",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_alertingPattern,
      { "alertingPattern", "camel.alertingPattern",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_originalCalledPartyID,
      { "originalCalledPartyID", "camel.originalCalledPartyID",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_carrier,
      { "carrier", "camel.carrier",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_callingPartysCategory,
      { "callingPartysCategory", "camel.callingPartysCategory",
        FT_UINT16, BASE_DEC|BASE_EXT_STRING, &isup_calling_partys_category_value_ext, 0,
        NULL, HFILL }},
    { &hf_camel_redirectingPartyID,
      { "redirectingPartyID", "camel.redirectingPartyID",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_redirectionInformation,
      { "redirectionInformation", "camel.redirectionInformation",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_genericNumbers,
      { "genericNumbers", "camel.genericNumbers",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_serviceInteractionIndicatorsTwo,
      { "serviceInteractionIndicatorsTwo", "camel.serviceInteractionIndicatorsTwo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_chargeNumber,
      { "chargeNumber", "camel.chargeNumber",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_legToBeConnected,
      { "legToBeConnected", "camel.legToBeConnected",
        FT_UINT32, BASE_DEC, VALS(inap_LegID_vals), 0,
        "LegID", HFILL }},
    { &hf_camel_cug_Interlock,
      { "cug-Interlock", "camel.cug_Interlock",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_cug_OutgoingAccess,
      { "cug-OutgoingAccess", "camel.cug_OutgoingAccess_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_suppressionOfAnnouncement,
      { "suppressionOfAnnouncement", "camel.suppressionOfAnnouncement_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_oCSIApplicable,
      { "oCSIApplicable", "camel.oCSIApplicable_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_naOliInfo,
      { "naOliInfo", "camel.naOliInfo",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_bor_InterrogationRequested,
      { "bor-InterrogationRequested", "camel.bor_InterrogationRequested_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_suppress_N_CSI,
      { "suppress-N-CSI", "camel.suppress_N_CSI_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_resourceAddress,
      { "resourceAddress", "camel.resourceAddress",
        FT_UINT32, BASE_DEC, VALS(camel_T_resourceAddress_vals), 0,
        NULL, HFILL }},
    { &hf_camel_ipRoutingAddress,
      { "ipRoutingAddress", "camel.ipRoutingAddress",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_none,
      { "none", "camel.none_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_suppress_O_CSI,
      { "suppress-O-CSI", "camel.suppress_O_CSI_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_continueWithArgumentArgExtension,
      { "continueWithArgumentArgExtension", "camel.continueWithArgumentArgExtension_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_suppress_D_CSI,
      { "suppress-D-CSI", "camel.suppress_D_CSI_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_suppressOutgoingCallBarring,
      { "suppressOutgoingCallBarring", "camel.suppressOutgoingCallBarring_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_legOrCallSegment,
      { "legOrCallSegment", "camel.legOrCallSegment",
        FT_UINT32, BASE_DEC, VALS(camel_LegOrCallSegment_vals), 0,
        NULL, HFILL }},
    { &hf_camel_legToBeReleased,
      { "legToBeReleased", "camel.legToBeReleased",
        FT_UINT32, BASE_DEC, VALS(inap_LegID_vals), 0,
        "LegID", HFILL }},
    { &hf_camel_callSegmentFailure,
      { "callSegmentFailure", "camel.callSegmentFailure_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_bCSM_Failure,
      { "bCSM-Failure", "camel.bCSM_Failure_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_assistingSSPIPRoutingAddress,
      { "assistingSSPIPRoutingAddress", "camel.assistingSSPIPRoutingAddress",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_callingPartyNumber,
      { "callingPartyNumber", "camel.callingPartyNumber",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_eventSpecificInformationBCSM,
      { "eventSpecificInformationBCSM", "camel.eventSpecificInformationBCSM",
        FT_UINT32, BASE_DEC, VALS(camel_EventSpecificInformationBCSM_vals), 0,
        NULL, HFILL }},
    { &hf_camel_miscCallInfo,
      { "miscCallInfo", "camel.miscCallInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_cGEncountered,
      { "cGEncountered", "camel.cGEncountered",
        FT_UINT32, BASE_DEC, VALS(camel_CGEncountered_vals), 0,
        NULL, HFILL }},
    { &hf_camel_locationNumber,
      { "locationNumber", "camel.locationNumber",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_highLayerCompatibility,
      { "highLayerCompatibility", "camel.highLayerCompatibility",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_additionalCallingPartyNumber,
      { "additionalCallingPartyNumber", "camel.additionalCallingPartyNumber",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_bearerCapability,
      { "bearerCapability", "camel.bearerCapability",
        FT_UINT32, BASE_DEC, VALS(camel_BearerCapability_vals), 0,
        NULL, HFILL }},
    { &hf_camel_cug_Index,
      { "cug-Index", "camel.cug_Index",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_iMSI,
      { "iMSI", "camel.iMSI",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_subscriberState,
      { "subscriberState", "camel.subscriberState",
        FT_UINT32, BASE_DEC, VALS(gsm_map_ms_SubscriberState_vals), 0,
        NULL, HFILL }},
    { &hf_camel_callReferenceNumber,
      { "callReferenceNumber", "camel.callReferenceNumber",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_mscAddress,
      { "mscAddress", "camel.mscAddress",
        FT_BYTES, BASE_NONE, NULL, 0,
        "ISDN_AddressString", HFILL }},
    { &hf_camel_calledPartyBCDNumber,
      { "calledPartyBCDNumber", "camel.calledPartyBCDNumber",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_timeAndTimezone,
      { "timeAndTimezone", "camel.timeAndTimezone",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_callForwardingSS_Pending,
      { "callForwardingSS-Pending", "camel.callForwardingSS_Pending_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_initialDPArgExtension,
      { "initialDPArgExtension", "camel.initialDPArgExtension_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_gmscAddress,
      { "gmscAddress", "camel.gmscAddress",
        FT_BYTES, BASE_NONE, NULL, 0,
        "ISDN_AddressString", HFILL }},
    { &hf_camel_ms_Classmark2,
      { "ms-Classmark2", "camel.ms_Classmark2",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_iMEI,
      { "iMEI", "camel.iMEI",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_supportedCamelPhases,
      { "supportedCamelPhases", "camel.supportedCamelPhases",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_offeredCamel4Functionalities,
      { "offeredCamel4Functionalities", "camel.offeredCamel4Functionalities",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_bearerCapability2,
      { "bearerCapability2", "camel.bearerCapability2",
        FT_UINT32, BASE_DEC, VALS(camel_BearerCapability_vals), 0,
        "BearerCapability", HFILL }},
    { &hf_camel_highLayerCompatibility2,
      { "highLayerCompatibility2", "camel.highLayerCompatibility2",
        FT_BYTES, BASE_NONE, NULL, 0,
        "HighLayerCompatibility", HFILL }},
    { &hf_camel_lowLayerCompatibility,
      { "lowLayerCompatibility", "camel.lowLayerCompatibility",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_lowLayerCompatibility2,
      { "lowLayerCompatibility2", "camel.lowLayerCompatibility2",
        FT_BYTES, BASE_NONE, NULL, 0,
        "LowLayerCompatibility", HFILL }},
    { &hf_camel_enhancedDialledServicesAllowed,
      { "enhancedDialledServicesAllowed", "camel.enhancedDialledServicesAllowed_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_uu_Data,
      { "uu-Data", "camel.uu_Data_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_collectInformationAllowed,
      { "collectInformationAllowed", "camel.collectInformationAllowed_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_releaseCallArgExtensionAllowed,
      { "releaseCallArgExtensionAllowed", "camel.releaseCallArgExtensionAllowed_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_legToBeCreated,
      { "legToBeCreated", "camel.legToBeCreated",
        FT_UINT32, BASE_DEC, VALS(inap_LegID_vals), 0,
        "LegID", HFILL }},
    { &hf_camel_newCallSegment,
      { "newCallSegment", "camel.newCallSegment",
        FT_UINT32, BASE_DEC, NULL, 0,
        "CallSegmentID", HFILL }},
    { &hf_camel_gsmSCFAddress,
      { "gsmSCFAddress", "camel.gsmSCFAddress",
        FT_BYTES, BASE_NONE, NULL, 0,
        "ISDN_AddressString", HFILL }},
    { &hf_camel_suppress_T_CSI,
      { "suppress-T-CSI", "camel.suppress_T_CSI_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_legIDToMove,
      { "legIDToMove", "camel.legIDToMove",
        FT_UINT32, BASE_DEC, VALS(inap_LegID_vals), 0,
        "LegID", HFILL }},
    { &hf_camel_allCallSegments,
      { "allCallSegments", "camel.allCallSegments",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_allCallSegmentsWithExtension,
      { "allCallSegmentsWithExtension", "camel.allCallSegmentsWithExtension_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_bcsmEvents,
      { "bcsmEvents", "camel.bcsmEvents",
        FT_UINT32, BASE_DEC, NULL, 0,
        "SEQUENCE_SIZE_1_bound__numOfBCSMEvents_OF_BCSMEvent", HFILL }},
    { &hf_camel_bcsmEvents_item,
      { "BCSMEvent", "camel.BCSMEvent_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_timerID,
      { "timerID", "camel.timerID",
        FT_UINT32, BASE_DEC, VALS(camel_TimerID_vals), 0,
        NULL, HFILL }},
    { &hf_camel_timervalue,
      { "timervalue", "camel.timervalue",
        FT_UINT32, BASE_DEC, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_sCIBillingChargingCharacteristics,
      { "sCIBillingChargingCharacteristics", "camel.sCIBillingChargingCharacteristics",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_legToBeSplit,
      { "legToBeSplit", "camel.legToBeSplit",
        FT_UINT32, BASE_DEC, VALS(inap_LegID_vals), 0,
        "LegID", HFILL }},
    { &hf_camel_chargingCharacteristics,
      { "chargingCharacteristics", "camel.chargingCharacteristics",
        FT_UINT32, BASE_DEC, VALS(camel_ChargingCharacteristics_vals), 0,
        NULL, HFILL }},
    { &hf_camel_applyChargingGPRS_tariffSwitchInterval,
      { "tariffSwitchInterval", "camel.applyChargingGPRS_tariffSwitchInterval",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_86400", HFILL }},
    { &hf_camel_chargingResult,
      { "chargingResult", "camel.chargingResult",
        FT_UINT32, BASE_DEC, VALS(camel_ChargingResult_vals), 0,
        NULL, HFILL }},
    { &hf_camel_active,
      { "active", "camel.active",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_camel_chargingRollOver,
      { "chargingRollOver", "camel.chargingRollOver",
        FT_UINT32, BASE_DEC, VALS(camel_ChargingRollOver_vals), 0,
        NULL, HFILL }},
    { &hf_camel_pdpID,
      { "pdpID", "camel.pdpID",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_gPRSCause,
      { "gPRSCause", "camel.gPRSCause",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_miscGPRSInfo,
      { "miscGPRSInfo", "camel.miscGPRSInfo_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "MiscCallInfo", HFILL }},
    { &hf_camel_gPRSEventSpecificInformation,
      { "gPRSEventSpecificInformation", "camel.gPRSEventSpecificInformation",
        FT_UINT32, BASE_DEC, VALS(camel_GPRSEventSpecificInformation_vals), 0,
        NULL, HFILL }},
    { &hf_camel_mSISDN,
      { "mSISDN", "camel.mSISDN",
        FT_BYTES, BASE_NONE, NULL, 0,
        "ISDN_AddressString", HFILL }},
    { &hf_camel_gPRSMSClass,
      { "gPRSMSClass", "camel.gPRSMSClass_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_sGSNCapabilities,
      { "sGSNCapabilities", "camel.sGSNCapabilities",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_gprsCause,
      { "gprsCause", "camel.gprsCause",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_gPRSEvent,
      { "gPRSEvent", "camel.gPRSEvent",
        FT_UINT32, BASE_DEC, NULL, 0,
        "SEQUENCE_SIZE_1_bound__numOfGPRSEvents_OF_GPRSEvent", HFILL }},
    { &hf_camel_gPRSEvent_item,
      { "GPRSEvent", "camel.GPRSEvent_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_sCIGPRSBillingChargingCharacteristics,
      { "sCIGPRSBillingChargingCharacteristics", "camel.sCIGPRSBillingChargingCharacteristics",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_callingPartysNumber,
      { "callingPartysNumber", "camel.callingPartysNumber",
        FT_BYTES, BASE_NONE, NULL, 0,
        "SMS_AddressString", HFILL }},
    { &hf_camel_destinationSubscriberNumber,
      { "destinationSubscriberNumber", "camel.destinationSubscriberNumber",
        FT_BYTES, BASE_NONE, NULL, 0,
        "CalledPartyBCDNumber", HFILL }},
    { &hf_camel_sMSCAddress,
      { "sMSCAddress", "camel.sMSCAddress",
        FT_BYTES, BASE_NONE, NULL, 0,
        "ISDN_AddressString", HFILL }},
    { &hf_camel_eventSpecificInformationSMS,
      { "eventSpecificInformationSMS", "camel.eventSpecificInformationSMS",
        FT_UINT32, BASE_DEC, VALS(camel_EventSpecificInformationSMS_vals), 0,
        NULL, HFILL }},
    { &hf_camel_callingPartyNumber_01,
      { "callingPartyNumber", "camel.callingPartyNumber",
        FT_BYTES, BASE_NONE, NULL, 0,
        "SMS_AddressString", HFILL }},
    { &hf_camel_locationInformationMSC,
      { "locationInformationMSC", "camel.locationInformationMSC_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "LocationInformation", HFILL }},
    { &hf_camel_tPShortMessageSpecificInfo,
      { "tPShortMessageSpecificInfo", "camel.tPShortMessageSpecificInfo",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_tPProtocolIdentifier,
      { "tPProtocolIdentifier", "camel.tPProtocolIdentifier",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_tPDataCodingScheme,
      { "tPDataCodingScheme", "camel.tPDataCodingScheme",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_tPValidityPeriod,
      { "tPValidityPeriod", "camel.tPValidityPeriod",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_smsReferenceNumber,
      { "smsReferenceNumber", "camel.smsReferenceNumber",
        FT_BYTES, BASE_NONE, NULL, 0,
        "CallReferenceNumber", HFILL }},
    { &hf_camel_calledPartyNumber_01,
      { "calledPartyNumber", "camel.calledPartyNumber",
        FT_BYTES, BASE_NONE, NULL, 0,
        "ISDN_AddressString", HFILL }},
    { &hf_camel_sMSEvents,
      { "sMSEvents", "camel.sMSEvents",
        FT_UINT32, BASE_DEC, NULL, 0,
        "SEQUENCE_SIZE_1_bound__numOfSMSEvents_OF_SMSEvent", HFILL }},
    { &hf_camel_sMSEvents_item,
      { "SMSEvent", "camel.SMSEvent_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_extensions_01,
      { "extensions", "camel.extensions",
        FT_UINT32, BASE_DEC, NULL, 0,
        "SEQUENCE_SIZE_1_numOfExtensions_OF_ExtensionField", HFILL }},
    { &hf_camel_extensions_item,
      { "ExtensionField", "camel.ExtensionField_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_na_info,
      { "na-info", "camel.na_info_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_naCarrierInformation,
      { "naCarrierInformation", "camel.naCarrierInformation_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_naCarrierId,
      { "naCarrierId", "camel.naCarrierId",
        FT_BYTES, BASE_NONE, NULL, 0,
        "NAEA_CIC", HFILL }},
    { &hf_camel_naCICSelectionType,
      { "naCICSelectionType", "camel.naCICSelectionType",
        FT_BYTES, BASE_NONE, NULL, 0,
        "NACarrierSelectionInfo", HFILL }},
    { &hf_camel_naChargeNumber,
      { "naChargeNumber", "camel.naChargeNumber",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_timeDurationCharging_01,
      { "timeDurationCharging", "camel.timeDurationCharging_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "T_timeDurationCharging_01", HFILL }},
    { &hf_camel_releaseIfdurationExceeded_01,
      { "releaseIfdurationExceeded", "camel.releaseIfdurationExceeded_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_tariffSwitchInterval,
      { "tariffSwitchInterval", "camel.tariffSwitchInterval",
        FT_UINT32, BASE_DEC, NULL, 0,
        "INTEGER_1_86400", HFILL }},
    { &hf_camel_tone_01,
      { "tone", "camel.tone",
        FT_BOOLEAN, BASE_NONE, NULL, 0,
        "BOOLEAN", HFILL }},
    { &hf_camel_local,
      { "local", "camel.local",
        FT_INT32, BASE_DEC, VALS(camel_opr_code_strings), 0,
        NULL, HFILL }},
    { &hf_camel_global,
      { "global", "camel.global",
        FT_OID, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_invoke,
      { "invoke", "camel.invoke_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_returnResult,
      { "returnResult", "camel.returnResult_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_returnError,
      { "returnError", "camel.returnError_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_reject,
      { "reject", "camel.reject_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_invokeId,
      { "invokeId", "camel.invokeId",
        FT_UINT32, BASE_DEC, VALS(camel_InvokeId_vals), 0,
        NULL, HFILL }},
    { &hf_camel_linkedId,
      { "linkedId", "camel.linkedId",
        FT_UINT32, BASE_DEC, VALS(camel_T_linkedId_vals), 0,
        NULL, HFILL }},
    { &hf_camel_linkedIdPresent,
      { "present", "camel.linkedIdPresent",
        FT_INT32, BASE_DEC, NULL, 0,
        "T_linkedIdPresent", HFILL }},
    { &hf_camel_absent,
      { "absent", "camel.absent_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_opcode,
      { "opcode", "camel.opcode",
        FT_UINT32, BASE_DEC, VALS(camel_Code_vals), 0,
        "Code", HFILL }},
    { &hf_camel_argument,
      { "argument", "camel.argument_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_result,
      { "result", "camel.result_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_resultArgument,
      { "result", "camel.resultArgument_element",
        FT_NONE, BASE_NONE, NULL, 0,
        "ResultArgument", HFILL }},
    { &hf_camel_errcode,
      { "errcode", "camel.errcode",
        FT_UINT32, BASE_DEC, VALS(camel_Code_vals), 0,
        "Code", HFILL }},
    { &hf_camel_parameter,
      { "parameter", "camel.parameter_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_camel_problem,
      { "problem", "camel.problem",
        FT_UINT32, BASE_DEC, VALS(camel_T_problem_vals), 0,
        NULL, HFILL }},
    { &hf_camel_general,
      { "general", "camel.general",
        FT_INT32, BASE_DEC, VALS(camel_GeneralProblem_vals), 0,
        "GeneralProblem", HFILL }},
    { &hf_camel_invokeProblem,
      { "invoke", "camel.invokeProblem",
        FT_INT32, BASE_DEC, VALS(camel_InvokeProblem_vals), 0,
        "InvokeProblem", HFILL }},
    { &hf_camel_problemReturnResult,
      { "returnResult", "camel.problemReturnResult",
        FT_INT32, BASE_DEC, VALS(camel_ReturnResultProblem_vals), 0,
        "ReturnResultProblem", HFILL }},
    { &hf_camel_returnErrorProblem,
      { "returnError", "camel.returnErrorProblem",
        FT_INT32, BASE_DEC, VALS(camel_ReturnErrorProblem_vals), 0,
        "ReturnErrorProblem", HFILL }},
    { &hf_camel_present,
      { "present", "camel.present",
        FT_INT32, BASE_DEC, NULL, 0,
        "INTEGER", HFILL }},
    { &hf_camel_InvokeId_present,
      { "InvokeId.present", "camel.InvokeId_present",
        FT_INT32, BASE_DEC, NULL, 0,
        "InvokeId_present", HFILL }},
  };

  /* List of subtrees */
  static int *ett[] = {
    &ett_camel,
    &ett_camelisup_parameter,
    &ett_camel_AccessPointName,
    &ett_camel_pdptypenumber,
    &ett_camel_cause,
    &ett_camel_RPcause,
    &ett_camel_stat,
    &ett_camel_calledpartybcdnumber,
    &ett_camel_callingpartynumber,
    &ett_camel_originalcalledpartyid,
    &ett_camel_redirectingpartyid,
    &ett_camel_locationnumber,
    &ett_camel_additionalcallingpartynumber,
    &ett_camel_calledAddressValue,
    &ett_camel_callingAddressValue,
    &ett_camel_assistingSSPIPRoutingAddress,
    &ett_camel_correlationID,
    &ett_camel_dTMFDigitsCompleted,
    &ett_camel_dTMFDigitsTimeOut,
    &ett_camel_number,
    &ett_camel_digitsResponse,
    &ett_camel_timeandtimezone,

    &ett_camel_AChChargingAddress,
    &ett_camel_AOCBeforeAnswer,
    &ett_camel_AOCGPRS,
    &ett_camel_AOCSubsequent,
    &ett_camel_AudibleIndicator,
    &ett_camel_BackwardServiceInteractionInd,
    &ett_camel_BasicGapCriteria,
    &ett_camel_T_calledAddressAndService,
    &ett_camel_T_callingAddressAndService,
    &ett_camel_BCSMEvent,
    &ett_camel_BCSM_Failure,
    &ett_camel_BearerCapability,
    &ett_camel_Burst,
    &ett_camel_BurstList,
    &ett_camel_CAI_GSM0224,
    &ett_camel_CallSegmentFailure,
    &ett_camel_CallSegmentToCancel,
    &ett_camel_CAMEL_AChBillingChargingCharacteristics,
    &ett_camel_T_timeDurationCharging,
    &ett_camel_CAMEL_CallResult,
    &ett_camel_T_timeDurationChargingResult,
    &ett_camel_CAMEL_FCIBillingChargingCharacteristics,
    &ett_camel_T_fci_fCIBCCCAMELsequence1,
    &ett_camel_CAMEL_FCIGPRSBillingChargingCharacteristics,
    &ett_camel_T_fciGPRS_fCIBCCCAMELsequence1,
    &ett_camel_CAMEL_FCISMSBillingChargingCharacteristics,
    &ett_camel_T_fciSMS_fCIBCCCAMELsequence1,
    &ett_camel_CAMEL_SCIBillingChargingCharacteristics,
    &ett_camel_CAMEL_SCIBillingChargingCharacteristicsAlt,
    &ett_camel_CAMEL_SCIGPRSBillingChargingCharacteristics,
    &ett_camel_ChangeOfPositionControlInfo,
    &ett_camel_ChangeOfLocation,
    &ett_camel_ChangeOfLocationAlt,
    &ett_camel_ChargingCharacteristics,
    &ett_camel_ChargingResult,
    &ett_camel_ChargingRollOver,
    &ett_camel_CollectedDigits,
    &ett_camel_CollectedInfo,
    &ett_camel_CompoundCriteria,
    &ett_camel_DestinationRoutingAddress,
    &ett_camel_DpSpecificCriteria,
    &ett_camel_DpSpecificCriteriaAlt,
    &ett_camel_DpSpecificInfoAlt,
    &ett_camel_T_oServiceChangeSpecificInfo,
    &ett_camel_T_tServiceChangeSpecificInfo,
    &ett_camel_T_collectedInfoSpecificInfo,
    &ett_camel_ElapsedTime,
    &ett_camel_T_timeGPRSIfTariffSwitch,
    &ett_camel_ElapsedTimeRollOver,
    &ett_camel_T_rO_TimeGPRSIfTariffSwitch,
    &ett_camel_EndUserAddress,
    &ett_camel_EventSpecificInformationBCSM,
    &ett_camel_T_routeSelectFailureSpecificInfo,
    &ett_camel_T_oCalledPartyBusySpecificInfo,
    &ett_camel_T_oNoAnswerSpecificInfo,
    &ett_camel_T_oAnswerSpecificInfo,
    &ett_camel_T_oMidCallSpecificInfo,
    &ett_camel_T_omidCallEvents,
    &ett_camel_T_oDisconnectSpecificInfo,
    &ett_camel_T_tBusySpecificInfo,
    &ett_camel_T_tNoAnswerSpecificInfo,
    &ett_camel_T_tAnswerSpecificInfo,
    &ett_camel_T_tMidCallSpecificInfo,
    &ett_camel_T_tmidCallEvents,
    &ett_camel_T_tDisconnectSpecificInfo,
    &ett_camel_T_oTermSeizedSpecificInfo,
    &ett_camel_T_callAcceptedSpecificInfo,
    &ett_camel_T_oAbandonSpecificInfo,
    &ett_camel_T_oChangeOfPositionSpecificInfo,
    &ett_camel_T_tChangeOfPositionSpecificInfo,
    &ett_camel_EventSpecificInformationSMS,
    &ett_camel_T_o_smsFailureSpecificInfo,
    &ett_camel_T_o_smsSubmissionSpecificInfo,
    &ett_camel_T_t_smsFailureSpecificInfo,
    &ett_camel_T_t_smsDeliverySpecificInfo,
    &ett_camel_Extensions,
    &ett_camel_ExtensionField,
    &ett_camel_ForwardServiceInteractionInd,
    &ett_camel_GapCriteria,
    &ett_camel_GapIndicators,
    &ett_camel_GapOnService,
    &ett_camel_GapTreatment,
    &ett_camel_GenericNumbers,
    &ett_camel_GPRS_QoS,
    &ett_camel_GPRS_QoS_Extension,
    &ett_camel_GPRSEvent,
    &ett_camel_GPRSEventSpecificInformation,
    &ett_camel_T_attachChangeOfPositionSpecificInformation,
    &ett_camel_T_pdp_ContextchangeOfPositionSpecificInformation,
    &ett_camel_T_detachSpecificInformation,
    &ett_camel_T_disconnectSpecificInformation,
    &ett_camel_T_pDPContextEstablishmentSpecificInformation,
    &ett_camel_T_pDPContextEstablishmentAcknowledgementSpecificInformation,
    &ett_camel_InbandInfo,
    &ett_camel_InformationToSend,
    &ett_camel_LegOrCallSegment,
    &ett_camel_LocationInformationGPRS,
    &ett_camel_MessageID,
    &ett_camel_T_text,
    &ett_camel_SEQUENCE_SIZE_1_bound__numOfMessageIDs_OF_Integer4,
    &ett_camel_T_variableMessage,
    &ett_camel_SEQUENCE_SIZE_1_5_OF_VariablePart,
    &ett_camel_MetDPCriteriaList,
    &ett_camel_MetDPCriterion,
    &ett_camel_MetDPCriterionAlt,
    &ett_camel_MidCallControlInfo,
    &ett_camel_QualityOfService,
    &ett_camel_ReceivingSideID,
    &ett_camel_RequestedInformationList,
    &ett_camel_RequestedInformationTypeList,
    &ett_camel_RequestedInformation,
    &ett_camel_RequestedInformationValue,
    &ett_camel_SendingSideID,
    &ett_camel_ServiceInteractionIndicatorsTwo,
    &ett_camel_SMSEvent,
    &ett_camel_TimeIfTariffSwitch,
    &ett_camel_TimeInformation,
    &ett_camel_Tone,
    &ett_camel_TransferredVolume,
    &ett_camel_T_volumeIfTariffSwitch,
    &ett_camel_TransferredVolumeRollOver,
    &ett_camel_T_rO_VolumeIfTariffSwitch,
    &ett_camel_VariablePart,
    &ett_camel_PAR_cancelFailed,
    &ett_camel_CAP_GPRS_ReferenceNumber,
    &ett_camel_PlayAnnouncementArg,
    &ett_camel_PromptAndCollectUserInformationArg,
    &ett_camel_ReceivedInformationArg,
    &ett_camel_SpecializedResourceReportArg,
    &ett_camel_ApplyChargingArg,
    &ett_camel_AssistRequestInstructionsArg,
    &ett_camel_CallGapArg,
    &ett_camel_CallInformationReportArg,
    &ett_camel_CallInformationRequestArg,
    &ett_camel_CancelArg,
    &ett_camel_CollectInformationArg,
    &ett_camel_ConnectArg,
    &ett_camel_ConnectToResourceArg,
    &ett_camel_T_resourceAddress,
    &ett_camel_ContinueWithArgumentArg,
    &ett_camel_ContinueWithArgumentArgExtension,
    &ett_camel_DisconnectForwardConnectionWithArgumentArg,
    &ett_camel_DisconnectLegArg,
    &ett_camel_EntityReleasedArg,
    &ett_camel_EstablishTemporaryConnectionArg,
    &ett_camel_EventReportBCSMArg,
    &ett_camel_InitialDPArg,
    &ett_camel_InitialDPArgExtension,
    &ett_camel_InitiateCallAttemptArg,
    &ett_camel_InitiateCallAttemptRes,
    &ett_camel_MoveLegArg,
    &ett_camel_PlayToneArg,
    &ett_camel_ReleaseCallArg,
    &ett_camel_AllCallSegmentsWithExtension,
    &ett_camel_RequestReportBCSMEventArg,
    &ett_camel_SEQUENCE_SIZE_1_bound__numOfBCSMEvents_OF_BCSMEvent,
    &ett_camel_ResetTimerArg,
    &ett_camel_SendChargingInformationArg,
    &ett_camel_SplitLegArg,
    &ett_camel_ApplyChargingGPRSArg,
    &ett_camel_ApplyChargingReportGPRSArg,
    &ett_camel_CancelGPRSArg,
    &ett_camel_ConnectGPRSArg,
    &ett_camel_ContinueGPRSArg,
    &ett_camel_EntityReleasedGPRSArg,
    &ett_camel_EventReportGPRSArg,
    &ett_camel_InitialDPGPRSArg,
    &ett_camel_ReleaseGPRSArg,
    &ett_camel_RequestReportGPRSEventArg,
    &ett_camel_SEQUENCE_SIZE_1_bound__numOfGPRSEvents_OF_GPRSEvent,
    &ett_camel_ResetTimerGPRSArg,
    &ett_camel_SendChargingInformationGPRSArg,
    &ett_camel_ConnectSMSArg,
    &ett_camel_EventReportSMSArg,
    &ett_camel_InitialDPSMSArg,
    &ett_camel_RequestReportSMSEventArg,
    &ett_camel_SEQUENCE_SIZE_1_bound__numOfSMSEvents_OF_SMSEvent,
    &ett_camel_ResetTimerSMSArg,
    &ett_camel_EstablishTemporaryConnectionArgV2,
    &ett_camel_SEQUENCE_SIZE_1_numOfExtensions_OF_ExtensionField,
    &ett_camel_InitialDPArgExtensionV2,
    &ett_camel_NACarrierInformation,
    &ett_camel_NA_Info,
    &ett_camel_CAMEL_AChBillingChargingCharacteristicsV2,
    &ett_camel_T_timeDurationCharging_01,
    &ett_camel_ReleaseIfDurationExceeded,
    &ett_camel_Code,
    &ett_camel_ROS,
    &ett_camel_Invoke,
    &ett_camel_T_linkedId,
    &ett_camel_ReturnResult,
    &ett_camel_T_result,
    &ett_camel_ReturnError,
    &ett_camel_Reject,
    &ett_camel_T_problem,
    &ett_camel_InvokeId,
  };

  static ei_register_info ei[] = {
     { &ei_camel_unknown_invokeData, { "camel.unknown.invokeData", PI_MALFORMED, PI_WARN, "Unknown invokeData", EXPFILL }},
     { &ei_camel_unknown_returnResultData, { "camel.unknown.returnResultData", PI_MALFORMED, PI_WARN, "Unknown returnResultData", EXPFILL }},
     { &ei_camel_unknown_returnErrorData, { "camel.unknown.returnErrorData", PI_MALFORMED, PI_WARN, "Unknown returnErrorData", EXPFILL }},
     { &ei_camel_par_wrong_length, { "camel.par_wrong_length", PI_PROTOCOL, PI_ERROR, "Wrong length of parameter", EXPFILL }},
     { &ei_camel_bcd_not_digit, { "camel.bcd_not_digit", PI_MALFORMED, PI_WARN, "BCD number contains a value that is not a digit", EXPFILL }},
  };

  expert_module_t* expert_camel;

  static tap_param camel_stat_params[] = {
    { PARAM_FILTER, "filter", "Filter", NULL, true }
  };

  static stat_tap_table_ui camel_stat_table = {
    REGISTER_TELEPHONY_GROUP_GSM,
    "CAMEL Messages and Response Status",
    PSNAME,
    "camel,counter",
    camel_stat_init,
    camel_stat_packet,
    camel_stat_reset,
    camel_stat_free_table_item,
    NULL,
    array_length(camel_stat_fields), camel_stat_fields,
    array_length(camel_stat_params), camel_stat_params,
    NULL,
    0
  };

  /* Register protocol */
  proto_camel = proto_register_protocol(PNAME, PSNAME, PFNAME);

  camel_handle = register_dissector("camel", dissect_camel, proto_camel);
  camel_v1_handle = register_dissector("camel-v1", dissect_camel_v1, proto_camel);
  camel_v2_handle = register_dissector("camel-v2", dissect_camel_v2, proto_camel);
  camel_v3_handle = register_dissector("camel-v3", dissect_camel_v3, proto_camel);
  camel_v4_handle = register_dissector("camel-v4", dissect_camel_v4, proto_camel);

  proto_register_field_array(proto_camel, hf, array_length(hf));
  proto_register_subtree_array(ett, array_length(ett));
  expert_camel = expert_register_protocol(proto_camel);
  expert_register_field_array(expert_camel, ei, array_length(ei));

  rose_ctx_init(&camel_rose_ctx);

  /* Register dissector tables */
  camel_rose_ctx.arg_local_dissector_table = register_dissector_table("camel.ros.local.arg",
                                                                      "CAMEL Operation Argument (local opcode)", proto_camel,
                                                                      FT_UINT32, BASE_HEX);
  camel_rose_ctx.res_local_dissector_table = register_dissector_table("camel.ros.local.res",
                                                                      "CAMEL Operation Result (local opcode)", proto_camel,
                                                                      FT_UINT32, BASE_HEX);
  camel_rose_ctx.err_local_dissector_table = register_dissector_table("camel.ros.local.err",
                                                                      "CAMEL Error (local opcode)", proto_camel,
                                                                      FT_UINT32, BASE_HEX);

  /* Register our configuration options, particularly our SSNs */
  /* Set default SSNs */
  range_convert_str(wmem_epan_scope(), &global_ssn_range, "146", MAX_SSN);

  camel_module = prefs_register_protocol(proto_camel, proto_reg_handoff_camel);

  prefs_register_enum_preference(camel_module, "date.format", "Date Format",
                                  "The date format: (DD/MM) or (MM/DD)",
                                  &date_format, date_options, false);


  prefs_register_range_preference(camel_module, "tcap.ssn",
    "TCAP SSNs",
    "TCAP Subsystem numbers used for Camel",
    &global_ssn_range, MAX_SSN);

  prefs_register_obsolete_preference(camel_module, "srt");

  prefs_register_bool_preference(camel_module, "persistentsrt",
                                 "Persistent stats for SRT",
                                 "Statistics for Response Time",
                                 &gcamel_PersistentSRT);

  /* Routine for statistic */
  register_init_routine(&camelsrt_init_routine);

  /* create new hash-table for SRT */
  srt_calls = wmem_map_new_autoreset(wmem_epan_scope(), wmem_file_scope(), camelsrt_call_hash, camelsrt_call_equal);

  camel_tap=register_tap(PSNAME);

  register_srt_table(proto_camel, PSNAME, 1, camelstat_packet, camelstat_init, NULL);
  register_stat_tap_table_ui(&camel_stat_table);
}

/*
 * Editor modelines
 *
 * Local Variables:
 * c-basic-offset: 2
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * ex: set shiftwidth=2 tabstop=8 expandtab:
 * :indentSize=2:tabSize=8:noTabs=true:
 */

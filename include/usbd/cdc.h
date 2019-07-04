/***********************************************************************
* $Id:: mw_usbd_audio.h 165 2011-04-14 17:41:11Z usb10131                     $
*
* Project: USB device ROM Stack
*
* Description:
*     USB Audio Device Class Definitions.
*
***********************************************************************
*   Copyright(C) 2011, NXP Semiconductor
*   All rights reserved.
*
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* products. This software is supplied "AS IS" without any warranties.
* NXP Semiconductors assumes no responsibility or liability for the
* use of the software, conveys no license or title under any patent,
* copyright, or mask work right to the product. NXP Semiconductors
* reserves the right to make changes in the software without
* notification. NXP Semiconductors also make no representation or
* warranty that such application will be suitable for the specified
* use without further testing or modification.
**********************************************************************/

/*! \addtogroup USBD_CDC Communications Device Class
 *
 *
 * @{
 * \ingroup USB_DEV
 *
 * \details This is not yet implemented.
 *
 */


/*! \file */

#ifndef USBD_CDC_H_
#define USBD_CDC_H_

#include "types.h"

//bInterfaceSubClass
#define USBD_CDC_INTERFACE_SUBCLASS_ACM 0x02
//bInterfaceProtocol
#define USBD_CDC_INTERFACE_PROTOCOL_V25TER 0x01

//bDescriptorSubType
#define USBD_CDC_DESCRIPTOR_SUBTYPE_HEADER 0x00
#define USBD_CDC_DESCRIPTOR_SUBTYPE_CALL_MANAGEMENT 0x01
#define USBD_CDC_DESCRIPTOR_SUBTYPE_CALL_MGT USBD_CDC_DESCRIPTOR_SUBTYPE_CALL_MANAGEMENT
#define USBD_CDC_DESCRIPTOR_SUBTYPE_ABSTRACT_CONTROL_MGT 0x02
#define USBD_CDC_DESCRIPTOR_SUBTYPE_DIRECT_LINE_MGT 0x03
#define USBD_CDC_DESCRIPTOR_SUBTYPE_TELEPHONE_RINGER 0x04
#define USBD_CDC_DESCRIPTOR_SUBTYPE_TELEPHONE_CALL_AND_LINE_STATE_REPORTING_CAPABILITIES 0x05
#define USBD_CDC_DESCRIPTOR_SUBTYPE_UNION 0x06
#define USBD_CDC_DESCRIPTOR_SUBTYPE_COUNTRY_SEL 0x07
#define USBD_CDC_DESCRIPTOR_SUBTYPE_TELEPHONE_OPERATION 0x08
#define USBD_CDC_DESCRIPTOR_SUBTYPE_USB_TERMINAL 0x09
#define USBD_CDC_DESCRIPTOR_SUBTYPE_NETWORK_CHANNEL 0x0A
#define USBD_CDC_DESCRIPTOR_SUBTYPE_PROTOCOL_UNIT 0x0B
#define USBD_CDC_DESCRIPTOR_SUBTYPE_EXTENSION_UNIT 0x0C
#define USBD_CDC_DESCRIPTOR_SUBTYPE_MULTI_CHANNEL 0x0D
#define USBD_CDC_DESCRIPTOR_SUBTYPE_CAPI_CONTROL 0x0E
#define USBD_CDC_DESCRIPTOR_SUBTYPE_ETHERNET_NETWORKING 0x0F
#define USBD_CDC_DESCRIPTOR_SUBTYPE_ATM_NETWORKING 0x10

//Interface Class specific requests
#define USBD_CDC_REQUEST_SEND_ENCAPSULATED_COMMAND 0x00
#define USBD_CDC_REQUEST_GET_ENCAPSULATED_RESPONSE 0x01
#define USBD_CDC_REQUEST_SET_COMM_FEATURE 0x02
#define USBD_CDC_REQUEST_GET_COMM_FEATURE 0x03
#define USBD_CDC_REQUEST_CLEAR_COMM_FEATURE 0x04
#define USBD_CDC_REQUEST_SET_AUX_LINE_STATE 0x10
#define USBD_CDC_REQUEST_SET_HOOK_STATE 0x11
#define USBD_CDC_REQUEST_PULSE_SETUP 0x12
#define USBD_CDC_REQUEST_SEND_PULSE 0x13
#define USBD_CDC_REQUEST_SET_PULSE_TIME 0x14
#define USBD_CDC_REQUEST_RING_AUX_JACK 0x15
#define USBD_CDC_REQUEST_SET_LINE_CODING 0x20
#define USBD_CDC_REQUEST_GET_LINE_CODING 0x21
#define USBD_CDC_REQUEST_SET_CONTROL_LINE_STATE 0x22
#define USBD_CDC_REQUEST_SEND_BREAK 0x23
#define USBD_CDC_REQUEST_SET_RINGER_PARMS 0x30
#define USBD_CDC_REQUEST_GET_RINGER_PARMS 0x31
#define USBD_CDC_REQUEST_SET_OPERATION_PARMS 0x32
#define USBD_CDC_REQUEST_GET_OPERATION_PARMS 0x33
#define USBD_CDC_REQUEST_SET_LINE_PARAMETERS 0x34
#define USBD_CDC_REQUEST_GET_LINE_PARAMETERS 0x35
#define USBD_CDC_REQUEST_DIAL_DIGITS 0x36
#define USBD_CDC_REQUEST_SET_UNIT_PARAMETER 0x37
#define USBD_CDC_REQUEST_GET_UNIT_PARAMETER 0x38
#define USBD_CDC_REQUEST_CLEAR_UNIT_PARAMETER 0x39
#define USBD_CDC_REQUEST_GET_PROFILE 0x3A
#define USBD_CDC_REQUEST_SET_ETHERNET_MULTICAST_FILTERS 0x40
#define USBD_CDC_REQUEST_SET_ETHERNET_POWER_MANAGEMENT_PATTERNFILTER 0x41
#define USBD_CDC_REQUEST_GET_ETHERNET_POWER_MANAGEMENT_PATTERNFILTER 0x42
#define USBD_CDC_REQUEST_SET_ETHERNET_PACKET_FILTER 0x43
#define USBD_CDC_REQUEST_GET_ETHERNET_STATISTIC 0x44
#define USBD_CDC_REQUEST_SET_ATM_DATA_FORMAT 0x50
#define USBD_CDC_REQUEST_GET_ATM_DEVICE_STATISTICS 0x51
#define USBD_CDC_REQUEST_SET_ATM_DEFAULT_VC 0x52
#define USBD_CDC_REQUEST_GET_ATM_VC_STATISTICS 0x53

#define USBD_CDC_STATE_RING_DETECT 0x09
#define USBD_CDC_STATE_SERIAL_STATE 0x20

#define USBD_CDC_SERIAL_STATE_OVERRUN (1<<6)
#define USBD_CDC_SERIAL_STATE_PARITY (1<<5)
#define USBD_CDC_SERIAL_STATE_FRAMING (1<<4)
#define USBD_CDC_SERIAL_STATE_RING (1<<3)
#define USBD_CDC_SERIAL_STATE_BREAK (1<<2)
#define USBD_CDC_SERIAL_STATE_TX_CARRIER (1<<1)
#define USBD_CDC_SERIAL_STATE_RX_CARRIER (1<<0)

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 functionSpecificData; //can be array
} usbd_cdc_functional_descriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u16 bcdCDC;
} usbd_cdc_header_functional_descriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bmCapabilities;
	u8 bDataInterface;
} usbd_cdc_call_management_functional_descriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bmCapabilities;
} usbd_cdc_abstract_control_model_functional_descriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bmCapabilities;
} usbd_cdc_direct_line_management_functional_descriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bRingerVolSteps;
	u8 bNumRingerPatterns;
} usbd_cdc_telephone_ringer_functional_descriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bmCapabilities;
} usbd_cdc_telephone_operational_modes_functional_descriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bmCapabilities;
} usbd_cdc_telephone_call_and_line_reporting_capabilities_functional_descriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bMasterInterface;
	u8 bSlaveInterface; //can be array
} usbd_cdc_union_functional_descriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 iCountryCodeRelDate;
	u16 wCountryCode; //can be array
} usbd_cdc_country_selection_functional_descriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bEntityId;
	u8 bInInterfaceNo;
	u8 bOutInterfaceNo;
	u8 bmOptions;
	u8 bChildId; //can be array
} usbd_cdc_usb_terminal_functional_descriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bEntityId;
	u8 iName;
	u8 bChannelIndex;
	u8 bPhysicalInterface;
} usbd_cdc_network_channel_terminal_functional_descriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bEntityId;
	u8 bProtocol;
	u8 bChildId; //can be array
} usbd_cdc_protocol_unit_functional_descriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bEntityId;
	u8 bExtensionCode;
	u8 iName;
	u8 bChildId; //can be array
} usbd_cdc_extension_unit_functional_descriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bmCapabilities;
} usbd_cdc_multi_channel_management_functional_descriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bmCapabilities;
} usbd_cdc_capi_control_management_functional_descriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 iMACAddress;
	u32 bmEthernetStatistics;
	u16 wMaxSegmentSize;
	u16 wNumberMCFilters;
	u8 bNumberPowerFilters;
} usbd_cdc_ethernet_netorking_functional_descriptor_t;

typedef struct MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 iEndSystemIdentifier;
	u8 bmDataCapabilities;
	u8 bmATMDeviceStatistics;
	u16 wType2MaxSegmentSize;
	u16 wType3MaxSegmentSize;
	u16 wMaxVC;
} usbd_cdc_atm_netorking_functional_descriptor_t;


typedef struct MCU_PACK MCU_PACK {
	u8 bFunctionLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bEntityId;
	u8 bExtensionCode;
	u8 iName;
	u8 bChildId; //can be array
} usbd_cdc_extensionunit_descriptor_t;

typedef struct MCU_PACK {
	usbd_cdc_header_functional_descriptor_t header;
	usbd_cdc_abstract_control_model_functional_descriptor_t acm;
	usbd_cdc_union_functional_descriptor_t union_descriptor;
	usbd_cdc_call_management_functional_descriptor_t call_management;
} usbd_cdc_abstract_control_model_interface_descriptor_t;

typedef struct MCU_PACK {
	usbd_cdc_header_functional_descriptor_t header;
	usbd_cdc_abstract_control_model_functional_descriptor_t acm;
	usbd_cdc_call_management_functional_descriptor_t call_management;
} usbd_cdc_abstract_control_model_interface_alt_descriptor_t;

typedef struct MCU_PACK {
	u8 bmRequestType;
	u8 bNotification;
	u16 wValue;
	u16 wIndex;
	u16 wLength;
} usbd_cdc_abstract_control_model_notification_t;

typedef struct MCU_PACK {
	u8 bmRequestType;
	u8 bRequest;
	u16 wValue;
	u16 wIndex;
	u16 wLength;
	u16 serial_state;
} usbd_cdc_abstract_control_model_notification_serialstate_t;

typedef struct MCU_PACK {
	usbd_interface_assocation_t interface_association;
	usbd_interface_descriptor_t interface_control /* The interface descriptor */;
	usbd_cdc_abstract_control_model_interface_descriptor_t acm /*! The CDC ACM Class descriptor */;
	usbd_endpoint_descriptor_t control /* Endpoint:  Interrupt out for control packets */;
	usbd_interface_descriptor_t interface_data /* The interface descriptor */;
	usbd_endpoint_descriptor_t data_out /* Endpoint:  Bulk out */;
	usbd_endpoint_descriptor_t data_in /* Endpoint:  Bulk in */;
} usbd_cdc_configuration_descriptor_t;

typedef struct MCU_PACK {
    u32 rate;
    u8 stop_bits;
    u8 parity;
    u8 width;
} usbd_cdc_line_coding_request_t;


#endif /* USBD_CDC_H_ */

/*! @} */

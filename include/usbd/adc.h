/* Copyright 2011-2018 Tyler Gilbert;
 * This file is part of Stratify OS.
 *
 * Stratify OS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stratify OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

/*! \addtogroup USBD_ADC Audio Device Class
 *
 *
 * @{
 * \ingroup USB_DEV
 *
 * \details This is not yet implemented.
 *
 */


/*! \file */


#ifndef USBD_ADC_H_
#define USBD_ADC_H_

#include "mcu/types.h"

#define USBD_ADC_INTERFACE_SUBCLASS_AUDIO_CONTROL 0x01
#define USBD_ADC_INTERFACE_SUBCLASS_AUDIOCONTROL USBD_ADC_INTERFACE_SUBCLASS_AUDIO_CONTROL
#define USBD_ADC_INTERFACE_SUBCLASS_AUDIO_STREAMING 0x02
#define USBD_ADC_INTERFACE_SUBCLASS_AUDIOSTREAMING USBD_ADC_INTERFACE_SUBCLASS_AUDIO_STREAMING
#define USBD_ADC_INTERFACE_SUBCLASS_MIDI_STREAMING 0x03
#define USBD_ADC_INTERFACE_SUBCLASS_MIDISTREAMING USBD_ADC_INTERFACE_SUBCLASS_MIDI_STREAMING

#define USBD_ADC_DESCRIPTOR_TYPE_DEVICE 0x21
#define USBD_ADC_DESCRIPTOR_TYPE_CONFIGURATION 0x22
#define USBD_ADC_DESCRIPTOR_TYPE_STRING 0x23
#define USBD_ADC_DESCRIPTOR_TYPE_INTERFACE 0x24
#define USBD_ADC_DESCRIPTOR_TYPE_ENDPOINT 0x25

#define USBD_ADC_CONTROL_DESCRIPTOR_SUBTYPE_UNDEFINED 0x00
#define USBD_ADC_CONTROL_DESCRIPTOR_SUBTYPE_HEADER 0x01
#define USBD_ADC_CONTROL_DESCRIPTOR_SUBTYPE_INPUT_TERMINAL 0x02
#define USBD_ADC_CONTROL_DESCRIPTOR_SUBTYPE_OUTPUT_TERMINAL 0x03
#define USBD_ADC_CONTROL_DESCRIPTOR_SUBTYPE_MIXER_UNIT 0x04
#define USBD_ADC_CONTROL_DESCRIPTOR_SUBTYPE_SELECTOR_UNIT 0x05
#define USBD_ADC_CONTROL_DESCRIPTOR_SUBTYPE_FEATURE_UNIT 0x06
#define USBD_ADC_CONTROL_DESCRIPTOR_SUBTYPE_PROCESSING_UNIT 0x07
#define USBD_ADC_CONTROL_DESCRIPTOR_SUBTYPE_EXTENSION_UNIT 0x08

#define USBD_ADC_STREAMING_DESCRIPTOR_SUBTYPE_UNDEFINED 0x00
#define USBD_ADC_STREAMING_DESCRIPTOR_SUBTYPE_GENERAL 0x01
#define USBD_ADC_STREAMING_DESCRIPTOR_SUBTYPE_FORMAT_TYPE 0x02
#define USBD_ADC_STREAMING_DESCRIPTOR_SUBTYPE_FORMAT_SPECIFIC 0x03

#define USBD_ADC_REQUEST_UNDEFINED 0x00
#define USBD_ADC_REQUEST_SET_CUR 0x01
#define USBD_ADC_REQUEST_GET_CUR 0x81
#define USBD_ADC_REQUEST_SET_MIN 0x02
#define USBD_ADC_REQUEST_GET_MIN 0x82
#define USBD_ADC_REQUEST_SET_MAX 0x03
#define USBD_ADC_REQUEST_GET_MAX 0x83
#define USBD_ADC_REQUEST_SET_RES 0x04
#define USBD_ADC_REQUEST_GET_RES 0x84
#define USBD_ADC_REQUEST_SET_MEM 0x05
#define USBD_ADC_REQUEST_GET_MEM 0x85
#define USBD_ADC_REQUEST_GET_STAT 0xFF

#define USBD_ADC_FORMAT_TYPE_I 0x01
#define USBD_ADC_FORMAT_TYPE_II 0x02
#define USBD_ADC_FORMAT_TYPE_III 0x03

#define USBD_ADC_TYPE_I_FORMAT_TYPE_PCM 0x0001
#define USBD_ADC_TYPE_I_FORMAT_TYPE_PCM8 0x0002
#define USBD_ADC_TYPE_I_FORMAT_TYPE_IEEE_FLOAT 0x0003
#define USBD_ADC_TYPE_I_FORMAT_TYPE_ALAW 0x0004
#define USBD_ADC_TYPE_I_FORMAT_TYPE_MULAW 0x0005

#define USBD_ADC_CHANNEL_CONFIG_M 0x0000 //Mono
#define USBD_ADC_CHANNEL_CONFIG_L 0x0001 //Left Front
#define USBD_ADC_CHANNEL_CONFIG_R 0x0002 //Right Front
#define USBD_ADC_CHANNEL_CONFIG_C 0x0004 //Center
#define USBD_ADC_CHANNEL_CONFIG_LFE 0x0008 //Low Frequency
#define USBD_ADC_CHANNEL_CONFIG_LS 0x0010 //Left Surround
#define USBD_ADC_CHANNEL_CONFIG_RS 0x0020 //Right Surround
#define USBD_ADC_CHANNEL_CONFIG_LC 0x0040 //Left Center
#define USBD_ADC_CHANNEL_CONFIG_RC 0x0080 //Right Center
#define USBD_ADC_CHANNEL_CONFIG_S 0x0100 //Surround
#define USBD_ADC_CHANNEL_CONFIG_SL 0x0200 //Side left
#define USBD_ADC_CHANNEL_CONFIG_SR 0x0400 //Side Right
#define USBD_ADC_CHANNEL_CONFIG_T 0x0800 //Top

#define USBD_ADC_TERMINAL_TYPE_USB 0x0100
#define USBD_ADC_TERMINAL_TYPE_USB_STREAMING 0x0101
#define USBD_ADC_TERMINAL_TYPE_USB_VENDOR_SPECIFIC 0x01FF

#define USBD_ADC_TERMINAL_TYPE_INPUT 0x0200
#define USBD_ADC_TERMINAL_TYPE_MICROPHONE 0x0201
#define USBD_ADC_TERMINAL_TYPE_DESKTOP_MICROPHONE 0x0202
#define USBD_ADC_TERMINAL_TYPE_PERSONAL_MICROPHONE 0x0203
#define USBD_ADC_TERMINAL_TYPE_OMNI_DIR_MICROPHONE 0x0204
#define USBD_ADC_TERMINAL_TYPE_MICROPHONE_ARRAY 0x0205
#define USBD_ADC_TERMINAL_TYPE_PROCESSING_MIC_ARRAY 0x0206

#define USBD_ADC_TERMINAL_TYPE_OUTPUT 0x0300
#define USBD_ADC_TERMINAL_TYPE_SPEAKER 0x0301
#define USBD_ADC_TERMINAL_TYPE_HEADPHONES 0x0302
#define USBD_ADC_TERMINAL_TYPE_HEAD_MOUNTED_DISPLAY_AUDIO 0x0303
#define USBD_ADC_TERMINAL_TYPE_DESKTOP_SPEAKER 0x0304
#define USBD_ADC_TERMINAL_TYPE_ROOM_SPEAKER 0x0305
#define USBD_ADC_TERMINAL_TYPE_COMMUNICATION_SPEAKER 0x0306

#define USBD_ADC_ENDPOINT_ATTRIBUTES_SAMPLING_FREQUENCY 0x01
#define USBD_ADC_ENDPOINT_ATTRIBUTES_PITCH 0x02
#define USBD_ADC_ENDPOINT_ATTRIBUTES_MAX_PACKETS_ONLY 0x80

#define USBD_ADC_ENDPOINT_REQUEST_SAMPLING_FREQUENCY 1
#define USBD_ADC_ENDPOINT_REQUEST_PITCH 2


typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bEndpointAddress;
	u8 bmAttributes;
	u16 wMaxPacketSize;
	u8 bInterval;
	u8 bRefresh;
	u8 bSynchAddress;
} usbd_adc_endpoint_descriptor_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u16 bcdADC;
	u16 wTotalLength;
	u8 bInCollection;
	u8 baInterfaceNr;
} usbd_adc_interface_descriptor_t;

typedef struct MCU_PACK {
	u8 bNrChannels;
	u32 bmChannelConfig;
	u8 iChannelNames;
} usbd_adc_channel_cluster_descriptor_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bTerminalID;
	u16 wTerminalType;
	u8 bAssocTerminal;
	u8 bNrChannels;
	u16 wChannelConfig;
	u8 iChannelNames;
	u8 iTerminal;
} usbd_adc_input_terminal_descriptor_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bTerminalID;
	u16 wTerminalType;
	u8 bAssocTerminal;
	u8 bSourceID;
	u8 iTerminal;
} usbd_adc_output_terminal_descriptor_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bUnitID;
	u8 bSourceID;
	u8 bControlSize;
	u8 bmaControls[1];
	u8 iFeature;
} usbd_adc_feature_unit_descriptor_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
	u8 bTerminalLink;
	u8 bDelay;
	u16 wFormatTag;
} usbd_adc_streaming_interface_descriptor_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bmAttributes;
	u8 bLockDelayUnits;
	u16 wLockDelay;
} usbd_adc_streaming_endpoint_descriptor_t;

typedef struct MCU_PACK
{
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bFormatType;
	u8 bNrChannels;
	u8 bSubFrameSize;
	u8 bBitResolution;
	u8 bSamFreqType;
	u8 tSamFreq[3];
} usbd_adc_type_I_format_descriptor_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u16 wFormatTag;
	u32 bmBSID;
	u8 bmAC3Features;
} usbd_adc_ac3_format_descriptor_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bFormatType;
	u16 wMaxBitRate;
	u16 wSamplesPerFrame;
	u8 bSamFreqType;
	u8 tLowerSamFreq[3];
	u8 tUpperSamFreq[3];
} usbd_adc_type_II_format_descriptor_t;


typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bFormatType;
	u16 wMaxBitRate;
	u16 wSamplesPerFrame;
	u8 bSamFreqType;
	u8 tSamFreq[3]; //just one frequency -- others are added manually after
} usbd_adc_type_II_format_discrete_descriptor_t;




#endif /* USBD_ADC_H_ */

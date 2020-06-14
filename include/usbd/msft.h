#ifndef SOS_USBD_MSFT_H
#define SOS_USBD_MSFT_H

#include "types.h"

#define USBD_MSFT_STRING_LENGTH 8
#define USBD_MSFT_VENDOR_CODE_BYTE 's'

typedef USBD_DECLARE_STRING(USBD_MSFT_STRING_LENGTH) msft_string_t;

#define USBD_MSFT_OS2_SET_HEADER_DESCRIPTOR 0x00
#define USBD_MSFT_OS2_SUBSET_HEADER_CONFIGURATION 0x01
#define USBD_MSFT_OS2_SUBSET_HEADER_FUNCTION 0x02
#define USBD_MSFT_OS2_FEATURE_COMPATIBLE_ID 0x03
#define USBD_MSFT_OS2_FEATURE_REG_PROPERTY 0x04

typedef struct MCU_PACK {
	u32 length;
	u16 bcd;
	u16 compatible_id_index;
	u8 section_count[8];
} usbd_msft_compatible_id_header_feature_descriptor_t;

//this is the 1.0 descriptor
typedef struct MCU_PACK {
	u8 interface_number;
	u8 resd0;
	u8 compatible_id[8];
	u8 sub_compatible_id[8];
	u8 resd1[6];
} usbd_msft_compatible_id_interface_feature_descriptor_t;


typedef struct MCU_PACK {
	usbd_msft_compatible_id_header_feature_descriptor_t header;
	usbd_msft_compatible_id_interface_feature_descriptor_t interface_feature;
} usbd_msft_compatible_id_feature_descriptor_t;


typedef struct MCU_PACK {
	u16 wLength;
	u16 wDescriptorType;
	u32 dwWindowsVersion;
	u16 wTotalLength;
} usbd_msft_os2_descriptor_set_header_t;

typedef struct MCU_PACK {
	u16 wLength;
	u16 wDescriptorType;
	u8 bConfigurationIndex; //index value that matches config returned by GET_DESCRIPTOR
	u8 bReserved;
	u16 wTotalLength;
} usbd_msft_os2_configuration_subset_header_t;

typedef struct MCU_PACK {
	u16 wLength;
	u16 wDescriptorType;
	u8 bFirstInterface;
	u8 bReserved;
	u16 wSubsetLength;
} usbd_msft_os2_function_subset_header_t;

typedef struct MCU_PACK {
	u16 wLength;
	u16 wDescriptorType;
	u8 CompatibleID[8];
	u8 SubCompatibleID[8];
} usbd_msft_os2_compatible_id_t;

typedef struct MCU_PACK {
	u16 wLength;
	u16 wDescriptorType;
	u16 wPropertyDataType;
	u16 wPropertyNameLength;
	//PropertyName variable size
	//u16 wPropertyDataLength
	//PropertyData variable size
} usbd_msft_os2_registry_property_descriptor_header_t;

extern const usbd_string_descriptor_t * usbd_extern_get_msft_string_descriptor();

#endif // SOS_USBD_MSFT_H

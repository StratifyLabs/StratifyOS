#ifndef SOS_USBD_MSFT_H
#define SOS_USBD_MSFT_H

#include "types.h"

#define USBD_MSFT_STRING_LENGTH 8
#define USBD_MSFT_VENDOR_CODE_BYTE 's'

typedef USBD_DECLARE_STRING(USBD_MSFT_STRING_LENGTH) msft_string_t;

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


extern const usbd_string_descriptor_t * usbd_extern_get_msft_string_descriptor();

#endif // SOS_USBD_MSFT_H

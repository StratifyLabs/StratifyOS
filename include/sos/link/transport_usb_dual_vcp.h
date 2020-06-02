
/*

Copyright 2011-2018 Tyler Gilbert

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/
#ifndef SOS_LINK_TRANSPORT_USB_DUAL_VCP_H_
#define SOS_LINK_TRANSPORT_USB_DUAL_VCP_H_

#include "transport_usb.h"
#include "usbd/cdc.h"

typedef struct MCU_PACK {
	usbd_configuration_descriptor_t cfg /* The configuration descriptor */;
	usbd_cdc_configuration_descriptor_t vcp0;
	usbd_cdc_configuration_descriptor_t vcp1;
	u8 terminator  /* A null terminator used by the driver (required) */;
} sos_link_transport_usb_dual_vcp_configuration_descriptor_t;


#define SOS_LINK_TRANSPORT_USB_DESC_VCP_0_SIZE 11
#define SOS_LINK_TRANSPORT_USB_DESC_VCP_1_SIZE 11
#define SOS_LINK_TRANSPORT_USB_DESC_VCP_0 'S','t','r','a','t','i','f','y',' ','O','S'
#define SOS_LINK_TRANSPORT_USB_DESC_VCP_1 'S','e','r','i','a','l',' ','P','o','r','t'

#define SOS_LINK_TRANSPORT_USB_DUAL_VCP_PORT (0)
#define SOS_LINK_TRANSPORT_USB_DUAL_VCP0_BULK_ENDPOINT (0x2)
#define SOS_LINK_TRANSPORT_USB_DUAL_VCP0_BULK_ENDPOINT_SIZE (64)
#define SOS_LINK_TRANSPORT_USB_DUAL_VCP0_INTERRUPT_ENDPOINT (0x1)
#define SOS_LINK_TRANSPORT_USB_DUAL_VCP0_INTERRUPT_ENDPOINT_SIZE (16)

#define SOS_LINK_TRANSPORT_USB_DUAL_VCP1_BULK_ENDPOINT (0x4)
#define SOS_LINK_TRANSPORT_USB_DUAL_VCP1_BULK_ENDPOINT_SIZE (64)
#define SOS_LINK_TRANSPORT_USB_DUAL_VCP1_INTERRUPT_ENDPOINT (0x3)
#define SOS_LINK_TRANSPORT_USB_DUAL_VCP1_INTERRUPT_ENDPOINT_SIZE (16)


/*! \brief USB Link String Data
 * \details This structure defines the USB strings structure which includes
 * a string for the manufacturer, product, and serial number.
 */
struct MCU_PACK sos_link_transport_usb_dual_vcp_string_descriptor_t {
	u8 bLength;
	u8 bDescriptorType;
	u16 wLANGID;
	USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_USB_DESC_MANUFACTURER_SIZE) manufacturer;
	USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_USB_DESC_PRODUCT_SIZE) product;
	USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_USB_DESC_SERIAL_SIZE) serial;
	USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_USB_DESC_VCP_0_SIZE) vcp0;
	USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_USB_DESC_VCP_1_SIZE) vcp1;
};

SOS_LINK_TRANSPORT_USB_EXTERN_CONST(dual_vcp);


#endif /* SOS_LINK_TRANSPORT_USB_DUAL_VCP_H_ */

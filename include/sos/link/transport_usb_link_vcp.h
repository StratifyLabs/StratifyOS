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
#ifndef SOS_LINK_TRANSPORT_USB_LINK_VCP_H_
#define SOS_LINK_TRANSPORT_USB_LINK_VCP_H_

#include "device/usbfifo.h"
#include "transport_usb_link.h"
#include "usbd/cdc.h"
#include "usbd/control.h"

typedef struct MCU_PACK {
  usbd_configuration_descriptor_t cfg /* The configuration descriptor */;
  sos_link_transport_usb_link_interface_descriptor_t link_interface;
  usbd_cdc_configuration_descriptor_t vcp;
  u8 terminator /* A null terminator used by the driver (required) */;
} sos_link_transport_usb_link_vcp_configuration_descriptor_t;

#define SOS_LINK_TRANSPORT_LINK_USB_VCP_DESCRIPTOR_SIZE 4
#define SOS_LINK_TRANSPORT_LINK_USB_VCP_DESCRIPTOR 'D', 'a', 't', 'a'

#define SOS_LINK_TRANSPORT_USB_LINK_VCP_INTERRUPT_ENDPOINT (0x1)
#define SOS_LINK_TRANSPORT_USB_LINK_VCP_INTERRUPT_ENDPOINT_SIZE (16)

#define SOS_LINK_TRANSPORT_USB_LINK_VCP_BULK_ENDPOINT (0x3)
#define SOS_LINK_TRANSPORT_USB_LINK_VCP_BULK_ENDPOINT_SIZE (64)

/*! \brief USB Link String Data
 * \details This structure defines the USB strings structure which includes
 * a string for the manufacturer, product, and serial number.
 */
struct MCU_PACK sos_link_transport_usb_link_vcp_string_descriptor_t {
  u8 bLength;
  u8 bDescriptorType;
  u16 wLANGID;
  USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_USB_DESC_MANUFACTURER_SIZE) manufacturer;
  USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_USB_DESC_PRODUCT_SIZE) product;
  USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_USB_DESC_SERIAL_SIZE) serial;
  USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_USB_DESCRIPTOR_SOS_LINK_SIZE) link;
  USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_LINK_USB_VCP_DESCRIPTOR_SIZE) vcp;
};

SOS_LINK_TRANSPORT_USB_EXTERN_CONST(link_vcp);

#endif /* SOS_LINK_TRANSPORT_USB_LINK_VCP_H_ */

// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef SOS_LINK_TRANSPORT_USB_VCP_H_
#define SOS_LINK_TRANSPORT_USB_VCP_H_

#include "transport_usb.h"
#include "usbd/cdc.h"

typedef struct MCU_PACK {
  usbd_configuration_descriptor_t cfg /* The configuration descriptor */;
  usbd_cdc_configuration_descriptor_t vcp;
  u8 terminator /* A null terminator used by the driver (required) */;
} sos_link_transport_usb_vcp_configuration_descriptor_t;

#define SOS_LINK_TRANSPORT_USB_DESC_VCP_SIZE 11
#define SOS_LINK_TRANSPORT_USB_DESC_VCP                                                  \
  'S', 't', 'r', 'a', 't', 'i', 'f', 'y', ' ', 'O', 'S'

#define SOS_LINK_TRANSPORT_USB_VCP_PORT (0)
#define SOS_LINK_TRANSPORT_USB_VCP_INTERRUPT_ENDPOINT (0x1)
#define SOS_LINK_TRANSPORT_USB_VCP_INTERRUPT_ENDPOINT_SIZE (16)

/*! \brief USB Link String Data
 * \details This structure defines the USB strings structure which includes
 * a string for the manufacturer, product, and serial number.
 */
struct MCU_PACK sos_link_transport_usb_vcp_string_descriptor_t {
  u8 bLength;
  u8 bDescriptorType;
  u16 wLANGID;
  USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_USB_DESC_MANUFACTURER_SIZE) manufacturer;
  USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_USB_DESC_PRODUCT_SIZE) product;
  USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_USB_DESC_SERIAL_SIZE) serial;
  USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_USB_DESC_VCP_SIZE) vcp;
};

int sos_link_usbd_cdc_event_handler(void *context, const mcu_event_t *event);
int usbd_cdc_event_handler(void *context, const mcu_event_t *event);

SOS_LINK_TRANSPORT_USB_EXTERN_CONST(vcp);

#endif /* SOS_LINK_TRANSPORT_USB_VCP_H_ */

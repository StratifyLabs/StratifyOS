// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef SOS_LINK_TRANSPORT_LINK_USB_H_
#define SOS_LINK_TRANSPORT_LINK_USB_H_

#include "device/usbfifo.h"
#include "transport_usb.h"
#include "usbd/cdc.h"
#include "usbd/control.h"

typedef struct MCU_PACK {
  usbd_interface_descriptor_t interface_data /* The interface descriptor */;
  usbd_endpoint_descriptor_t data_out /* Endpoint:  Bulk out */;
  usbd_endpoint_descriptor_t data_in /* Endpoint:  Bulk in */;
} sos_link_transport_usb_link_interface_descriptor_t;

typedef struct MCU_PACK {
  usbd_configuration_descriptor_t cfg /* The configuration descriptor */;
  sos_link_transport_usb_link_interface_descriptor_t link_interface;
  u8 terminator /* A null terminator used by the driver (required) */;
} sos_link_transport_usb_link_configuration_descriptor_t;

#define SOS_LINK_TRANSPORT_USB_DESCRIPTOR_SOS_LINK_SIZE 11
#define SOS_LINK_TRANSPORT_USB_DESCRIPTOR_SOS_LINK                                       \
  'S', 't', 'r', 'a', 't', 'i', 'f', 'y', ' ', 'O', 'S'

/*! \brief USB Link String Data
 * \details This structure defines the USB strings structure which includes
 * a string for the manufacturer, product, and serial number.
 */
struct MCU_PACK sos_link_transport_usb_link_string_descriptor_t {
  u8 bLength;
  u8 bDescriptorType;
  u16 wLANGID;
  USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_USB_DESC_MANUFACTURER_SIZE) manufacturer;
  USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_USB_DESC_PRODUCT_SIZE) product;
  USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_USB_DESC_SERIAL_SIZE) serial;
  USBD_DECLARE_STRING(SOS_LINK_TRANSPORT_USB_DESCRIPTOR_SOS_LINK_SIZE) link;
};

SOS_LINK_TRANSPORT_USB_EXTERN_CONST(link);

#define SOS_LINK_TRANSPORT_USB_DECLARE_INTERFACE_CONFIGURATION_DESCRIPTOR(               \
  interface_string, interface_number, endpoint_number, endpoint_size)                    \
  .interface_data =                                                                      \
    {.bLength = sizeof(usbd_interface_descriptor_t),                                     \
     .bDescriptorType = USBD_DESCRIPTOR_TYPE_INTERFACE,                                  \
     .bInterfaceNumber = interface_number,                                               \
     .bAlternateSetting = 0x00,                                                          \
     .bNumEndpoints = 0x02,                                                              \
     .bInterfaceClass = 0xff,                                                            \
     .bInterfaceSubClass = 0x50,                                                         \
     .bInterfaceProtocol = 0x51,                                                         \
     .iInterface = interface_string},                                                    \
  .data_out =                                                                            \
    {.bLength = sizeof(usbd_endpoint_descriptor_t),                                      \
     .bDescriptorType = USBD_DESCRIPTOR_TYPE_ENDPOINT,                                   \
     .bEndpointAddress = endpoint_number,                                                \
     .bmAttributes = USBD_ENDPOINT_ATTRIBUTES_TYPE_BULK,                                 \
     .wMaxPacketSize = endpoint_size,                                                    \
     .bInterval = 1},                                                                    \
  .data_in = {                                                                           \
    .bLength = sizeof(usbd_endpoint_descriptor_t),                                       \
    .bDescriptorType = USBD_DESCRIPTOR_TYPE_ENDPOINT,                                    \
    .bEndpointAddress = (endpoint_number) | 0x80,                                        \
    .bmAttributes = USBD_ENDPOINT_ATTRIBUTES_TYPE_BULK,                                  \
    .wMaxPacketSize = endpoint_size,                                                     \
    .bInterval = 1}

#endif /* SOS_LINK_TRANSPORT_LINK_USB_H_ */

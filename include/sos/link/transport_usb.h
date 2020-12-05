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
#ifndef SOS_LINK_TRANSPORT_USB_H_
#define SOS_LINK_TRANSPORT_USB_H_

#include "device/usbfifo.h"
#include "transport.h"
#include "usbd/cdc.h"
#include "usbd/control.h"
#include "usbd/msft.h"

#define SOS_USBD_VID 0x20A0
#define SOS_USBD_PID 0x41D5

#define SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT 2
#define SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_SIZE 64
#define SOS_LINK_TRANSPORT_USB_PORT 0

#define SOS_LINK_TRANSPORT_MSFT_VENDOR_CODE 'S'

// next valid 3FC
#define SOS_LINK_TRANSPORT_USB_BCD_VERSION 0x004C

#define SOS_LINK_TRANSPORT_USB_EXTERN_CONST(name)                                        \
  extern const usbd_device_descriptor_t                                                  \
    sos_link_transport_usb_##name##_device_descriptor MCU_WEAK;                          \
  extern const usbd_qualifier_descriptor_t                                               \
    sos_link_transport_usb_##name##_qualifier_descriptor MCU_WEAK;                       \
  extern const sos_link_transport_usb_##name##_configuration_descriptor_t                \
    sos_link_transport_usb_##name##_configuration_descriptor MCU_WEAK;                   \
  extern const struct sos_link_transport_usb_##name##_string_descriptor_t                \
    sos_link_transport_usb_##name##_string_descriptor MCU_WEAK;                          \
  extern const usbd_control_constants_t sos_link_transport_usb_##name##_constants

#define SOS_LINK_TRANSPORT_USB_CONST(name, class_event_handler_value)                    \
  const usbd_control_constants_t sos_link_transport_usb_##name##_constants = {           \
    .device = &sos_link_transport_usb_##name##_device_descriptor,                        \
    .config = &sos_link_transport_usb_##name##_configuration_descriptor,                 \
    .qualifier = &sos_link_transport_usb_##name##_qualifier_descriptor,                  \
    .max_packet_zero_size = 64,                                                          \
    .string = &sos_link_transport_usb_##name##_string_descriptor,                        \
    .class_event_handler = class_event_handler_value};

#define SOS_LINK_TRANSPORT_USB_DEVICE_DESCRIPTOR(                                        \
  name, class_value, sub_class_value, protocol_value, version_value)                     \
  const usbd_device_descriptor_t sos_link_transport_usb_##name##_device_descriptor = {   \
    .bLength = sizeof(usbd_device_descriptor_t),                                         \
    .bDescriptorType = USBD_DESCRIPTOR_TYPE_DEVICE,                                      \
    .bcdUSB = 0x0200,                                                                    \
    .bDeviceClass = class_value,                                                         \
    .bDeviceSubClass = sub_class_value,                                                  \
    .bDeviceProtocol = protocol_value,                                                   \
    .bMaxPacketSize = MCU_CORE_USB_MAX_PACKET_ZERO_VALUE,                                \
    .idVendor = SOS_USBD_VID,                                                            \
    .idProduct = SOS_USBD_PID,                                                           \
    .bcdDevice = version_value,                                                          \
    .iManufacturer = 1,                                                                  \
    .iProduct = 2,                                                                       \
    .iSerialNumber = 3,                                                                  \
    .bNumConfigurations = 1};                                                            \
  const usbd_qualifier_descriptor_t                                                      \
    sos_link_transport_usb_##name##_qualifer_descriptor = {                              \
      .bLength = sizeof(usbd_device_descriptor_t),                                       \
      .bDescriptorType = USBD_DESCRIPTOR_TYPE_DEVICE,                                    \
      .bcdUSB = 0x0200,                                                                  \
      .bDeviceClass = class_value,                                                       \
      .bDeviceSubClass = sub_class_value,                                                \
      .bDeviceProtocol = protocol_value,                                                 \
      .bMaxPacketSize = MCU_CORE_USB_MAX_PACKET_ZERO_VALUE,                              \
      .bReserved = 0};

#define SOS_LINK_TRANSPORT_USB_DESC_MANUFACTURER_SIZE 13
#define SOS_LINK_TRANSPORT_USB_DESC_PRODUCT_SIZE 11
#define SOS_LINK_TRANSPORT_USB_DESC_SERIAL_SIZE 16
#define SOS_LINK_TRANSPORT_USB_DESC_MANUFACTURER_STRING                                  \
  'S', 't', 'r', 'a', 't', 'i', 'f', 'y', ' ', 'L', 'a', 'b', 's'
#define SOS_LINK_TRANSPORT_USB_DESC_PRODUCT_STRING                                       \
  'S', 't', 'r', 'a', 't', 'i', 'f', 'y', ' ', 'O', 'S'
#define SOS_LINK_TRANSPORT_USB_DESC_SERIAL_STRING                                        \
  '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'

link_transport_phy_t sos_link_transport_usb_open(
  const char *name,
  usbd_control_t *context,
  const usbd_control_constants_t *constants,
  const devfs_handle_t *usb_handle,
  const usb_attr_t *usb_attr);
int sos_link_transport_usb_read(link_transport_phy_t, void *buf, int nbyte);
int sos_link_transport_usb_write(link_transport_phy_t, const void *buf, int nbyte);
int sos_link_transport_usb_close(link_transport_phy_t *handle);
void sos_link_transport_usb_wait(int msec);
void sos_link_transport_usb_flush(link_transport_phy_t handle);

link_transport_phy_t boot_link_transport_usb_open(
  const char *name,
  usbd_control_t *context,
  const usbd_control_constants_t *constants,
  const devfs_handle_t *usb_handle,
  const usb_attr_t *usb_attr,
  mcu_pin_t usb_up_pin,
  int usb_up_active_high);
int boot_link_transport_usb_read(link_transport_phy_t, void *buf, int nbyte);
int boot_link_transport_usb_write(link_transport_phy_t, const void *buf, int nbyte);
int boot_link_transport_usb_close(link_transport_phy_t *handle);
void boot_link_transport_usb_wait(int msec);
void boot_link_transport_usb_flush(link_transport_phy_t handle);

extern const usbfifo_config_t sos_link_transport_usb_fifo_cfg;
extern usbfifo_state_t sos_link_transport_usb_fifo_state MCU_SYS_MEM;

int sos_link_transport_usb_msft_string_event(
  void *context_object,
  const mcu_event_t *usb_event);

#endif /* SOS_LINK_TRANSPORT_USB_H_ */

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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */

#include "config.h"
#include "device/sys.h"
#include "mcu/boot_debug.h"
#include "mcu/core.h"
#include "mcu/mcu.h"
#include "sos/debug.h"
#include "sos/dev/usb.h"
#include "sos/link.h"
#include "usbd/cdc.h"
#include "usbd/control.h"
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "sos/link/transport_usb_link.h"

#define SOS_REQUIRED_CURRENT 500

#define INTERFACE_NUMBER 0
#define INTERFACE_STRING 4

static const usbd_msft_compatible_id_feature_descriptor_t
  msft_compatible_id_feature_descriptor = {
    .header =
      {
        .length = sizeof(usbd_msft_compatible_id_feature_descriptor_t),
        .bcd = 0x0100,
        .compatible_id_index = 0x0004,
        .section_count[0] = 1,
      },
    .interface_feature = {
      .interface_number = 0,
      .resd0 = 0x01,
      .compatible_id = {'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00}, // WINUSB\0\0
    }};

typedef struct MCU_PACK {
  usbd_msft_os2_descriptor_set_header_t header_descriptor;
  usbd_msft_os2_compatible_id_t compatible_id_descriptor;
} compatible_id_feature_descriptor_t;

#if 0
static const compatible_id_feature_descriptor_t msft_os2_compatible_id_feature_descriptor =
{
	.header_descriptor = {
		.wLength = sizeof(usbd_msft_os2_descriptor_set_header_t),
		.wDescriptorType = USBD_MSFT_OS2_SET_HEADER_DESCRIPTOR,
		.dwWindowsVersion = 0x06030000, //windows 8.1
		.wTotalLength= sizeof(compatible_id_feature_descriptor_t),
	},
	.compatible_id_descriptor = {
		.wLength = sizeof(usbd_msft_os2_compatible_id_t),
		.wDescriptorType = USBD_MSFT_OS2_FEATURE_COMPATIBLE_ID,
		.CompatibleID = {'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00}, //WINUSB\0\0
	}
};
#endif

int link_class_handler(void *object, const mcu_event_t *event) {
  usbd_control_t *context = object;
  u32 o_events = event->o_events;

  if (sos_link_transport_usb_msft_string_event(object, event)) {
    return 1;
  }

  if (
    (o_events & MCU_EVENT_FLAG_SETUP)
    && (context->setup_packet.bRequest == USBD_MSFT_VENDOR_CODE_BYTE)
    && (context->setup_packet.wIndex.w == 0x0004)) {

    u16 len = sizeof(msft_compatible_id_feature_descriptor);
    context->data.dptr = (u8 *)&msft_compatible_id_feature_descriptor;
    if (context->data.nbyte > len) {
      context->data.nbyte = len;
    }
    usbd_control_datain_stage(context);
    return 1;
  }
  return 0;
}

SOS_LINK_TRANSPORT_USB_DEVICE_DESCRIPTOR(
  link,
  USBD_DEVICE_CLASS_VENDOR_SPECIFIC,
  0,
  0,
  SOS_LINK_TRANSPORT_USB_BCD_VERSION | 0)

SOS_LINK_TRANSPORT_USB_CONST(
  link,
  link_class_handler)

const sos_link_transport_usb_link_configuration_descriptor_t
  sos_link_transport_usb_link_configuration_descriptor MCU_WEAK = {

    .cfg =
      {.bLength = sizeof(usbd_configuration_descriptor_t),
       .bDescriptorType = USBD_DESCRIPTOR_TYPE_CONFIGURATION,
       .wTotalLength = sizeof(sos_link_transport_usb_link_configuration_descriptor_t)
                       - 1, // exclude the zero terminator
       .bNumInterfaces = 0x01,
       .bConfigurationValue = 0x01,
       .iConfiguration = 2,
       .bmAttributes = USBD_CONFIGURATION_ATTRIBUTES_BUS_POWERED,
       .bMaxPower = USBD_CONFIGURATION_MAX_POWER_MA(SOS_REQUIRED_CURRENT)},

    .link_interface = {SOS_LINK_TRANSPORT_USB_DECLARE_INTERFACE_CONFIGURATION_DESCRIPTOR(
      INTERFACE_STRING,
      INTERFACE_NUMBER,
      SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT,
      SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_SIZE)},

    .terminator = 0};

const struct sos_link_transport_usb_link_string_descriptor_t
  sos_link_transport_usb_link_string_descriptor MCU_WEAK = {
    .bLength = 4,
    .bDescriptorType = USBD_DESCRIPTOR_TYPE_STRING,
    .wLANGID = 0x0409, // English
    .manufacturer = USBD_ASSIGN_STRING(
      SOS_LINK_TRANSPORT_USB_DESC_MANUFACTURER_SIZE,
      SOS_LINK_TRANSPORT_USB_DESC_MANUFACTURER_STRING),
    .product = USBD_ASSIGN_STRING(
      SOS_LINK_TRANSPORT_USB_DESC_PRODUCT_SIZE,
      SOS_LINK_TRANSPORT_USB_DESC_PRODUCT_STRING),
    .serial = USBD_ASSIGN_STRING(
      SOS_LINK_TRANSPORT_USB_DESC_SERIAL_SIZE,
      0), // dynamically load SN based on silicon
    .link = USBD_ASSIGN_STRING(
      SOS_LINK_TRANSPORT_USB_DESCRIPTOR_SOS_LINK_SIZE,
      SOS_LINK_TRANSPORT_USB_DESCRIPTOR_SOS_LINK)};

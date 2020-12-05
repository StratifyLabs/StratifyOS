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

#include "device/sys.h"
#include "device/usbfifo.h"
#include "mcu/core.h"
#include "mcu/mcu.h"
#include "mcu/usb.h"
#include "sos/debug.h"
#include "sos/dev/usb.h"
#include "sos/link.h"
#include "sos/link/transport_usb.h"
#include "usbd/control.h"
#include <errno.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "usbd/msft.h"

const msft_string_t sos_link_transport_usb_msft_string = USBD_ASSIGN_STRING(
  USBD_MSFT_STRING_LENGTH,
  'M',
  'S',
  'F',
  'T',
  '1',
  '0',
  '0',
  USBD_MSFT_VENDOR_CODE_BYTE);


int sos_link_transport_usb_msft_string_event(
  void *context_object,
  const mcu_event_t *usb_event) {
  u32 o_events = usb_event->o_events;
  usbd_control_t *context = context_object;
  if (
    (o_events & MCU_EVENT_FLAG_SETUP)
    && (context->setup_packet.bRequest == USBD_REQUEST_STANDARD_GET_DESCRIPTOR)
    && (context->setup_packet.wValue.b[1] == USBD_DESCRIPTOR_TYPE_STRING)
    && (context->setup_packet.wValue.b[0] == 0xee)) {
    context->data.dptr = (u8 *)&sos_link_transport_usb_msft_string;
    u16 len = sos_link_transport_usb_msft_string.bLength;
    if (context->data.nbyte > len) {
      context->data.nbyte = len;
    }
    usbd_control_datain_stage(context);
    return 1;
  }
  return 0;
}

usbfifo_state_t sos_link_transport_usb_fifo_state MCU_SYS_MEM;

static int open_pio(mcu_pin_t pin, int active_high) {
  char path[PATH_MAX];
  int fd;
  pio_attr_t attr;
  int len;

  strncpy(path, "/dev/pio", PATH_MAX - 2);
  len = strnlen(path, PATH_MAX - 2);
  path[len] = pin.port + '0';
  path[len + 1] = 0;

  fd = open(path, O_RDWR);
  if (fd < 0) {
    return -1;
  }

  attr.o_pinmask = (1 << pin.pin);
  if (active_high) {
    ioctl(fd, I_PIO_CLRMASK, (void *)attr.o_pinmask);
  } else {
    ioctl(fd, I_PIO_SETMASK, (void *)attr.o_pinmask);
  }
  attr.o_flags = PIO_FLAG_SET_OUTPUT | PIO_FLAG_IS_DIRONLY;
  ioctl(fd, I_PIO_SETATTR, &attr);

  return fd;
}

link_transport_phy_t sos_link_transport_usb_open(
  const char *name,
  usbd_control_t *context,
  const usbd_control_constants_t *constants,
  const devfs_handle_t *usb_handle,
  const usb_attr_t *usb_attr) {

  int fd;
  int result;
  MCU_UNUSED_ARGUMENT(name);

  memset(context, 0, sizeof(usbd_control_t));
  context->constants = constants;
  context->handle = usb_handle;

  usleep(100 * 1000);

  // open USB
  sos_debug_log_info(SOS_DEBUG_USB | SOS_DEBUG_LINK, "Open link-phy-usb");
  errno = 0;
  fd = open("/dev/link-phy-usb", O_RDWR);
  if (fd < 0) {
    sos_debug_log_error(
      SOS_DEBUG_USB | SOS_DEBUG_LINK, "Failed to open link-phy-usb (%d)", errno);
    return LINK_PHY_ERROR;
  }

  // set USB attributes
  sos_debug_log_info(SOS_DEBUG_USB | SOS_DEBUG_LINK, "Set USB attr fd:%d", fd);

  result = ioctl(fd, I_USB_SETATTR, usb_attr);
  if (result < 0) {
    sos_debug_log_error(
      SOS_DEBUG_USB | SOS_DEBUG_LINK, "Failed to set USB attr (%d, %d)", result, errno);
    return LINK_PHY_ERROR;
  }

  sos_debug_log_info(SOS_DEBUG_USB | SOS_DEBUG_LINK, "USB Dev Init");
  // initialize USB device
  cortexm_svcall(usbd_control_svcall_init, context);

  ioctl(fd, I_FIFO_INIT);

  return fd;
}

int sos_link_transport_usb_write(
  link_transport_phy_t handle,
  const void *buf,
  int nbyte) {
  int ret;
  ret = write(handle, buf, nbyte);
  return ret;
}

int sos_link_transport_usb_read(link_transport_phy_t handle, void *buf, int nbyte) {
  int ret;
  ret = read(handle, buf, nbyte);
  return ret;
}

int sos_link_transport_usb_close(link_transport_phy_t *handle) {
  link_transport_phy_t fd = *handle;
  *handle = -1;
  return close(fd);
}

void sos_link_transport_usb_wait(int msec) {
  int i;
  for (i = 0; i < msec; i++) {
    usleep(1000);
  }
}

void sos_link_transport_usb_flush(link_transport_phy_t handle) {
  ioctl(handle, I_FIFO_FLUSH);
}

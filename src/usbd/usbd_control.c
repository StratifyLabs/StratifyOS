// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include <stdio.h>
#include <unistd.h>

#include "device/sys.h"
#include "mcu/boot_debug.h"
#include "sos/config.h"
#include "sos/debug.h"

#include "mcu/usb.h"
#include "sys/ioctl.h"
#include "usbd/control.h"

#include "usbd_standard.h"

static int execute_class_handler(usbd_control_t *context, const mcu_event_t *usb_event)
  MCU_ROOT_EXEC_CODE;
static void stall(usbd_control_t *context) MCU_ROOT_EXEC_CODE;

int execute_class_handler(usbd_control_t *context, const mcu_event_t *usb_event) {
  if (context->constants->class_event_handler != 0) {
    return context->constants->class_event_handler(context, usb_event);
  }
  return 0;
}

void stall(usbd_control_t *context) {
  usbd_control_stall_endpoint(context->handle, (USBD_ENDPOINT_ADDRESS_IN | 0x00));
  context->data.nbyte = 0;
}

void usbd_control_svcall_init(void *args) {
  CORTEXM_SVCALL_ENTER();
  usbd_control_root_init(args);
}

void usbd_control_root_init(void *args) {
  usbd_control_t *context = args;
  mcu_action_t action;

  // Set up the action to take when there is data on the control endpoint
  action.channel = 0;
  action.handler.context = context;
  action.handler.callback = usbd_control_handler;
  action.o_events =
    MCU_EVENT_FLAG_DATA_READY | MCU_EVENT_FLAG_WRITE_COMPLETE | MCU_EVENT_FLAG_SETUP;
  action.prio = 0;
  if (sos_config.usb.set_action(context->handle, &action) < 0) {
    sos_handle_event(
      SOS_EVENT_ROOT_FATAL, "usbd control setaction");
  }

  if (usbd_control_attach(context->handle) < 0) {
    sos_handle_event(
      SOS_EVENT_ROOT_FATAL, "usbd control attach");
  }
}

int usbd_control_handler(void *context_object, const mcu_event_t *usb_event) {
  u32 o_events = usb_event->o_events;
  usbd_control_t *context = context_object;

  if (o_events & MCU_EVENT_FLAG_SETUP) {
    // read the setup packet
    usbd_control_handler_setup_stage(context);

    int ret = usbd_standard_request_handle_setup(context);

    // allow the class handler handle the standard request if the request was handled with
    // usbd_standard_request_handle_setup(), no need to stall
    if ((ret == 0) && (execute_class_handler(context, usb_event) == 0)) {
      stall(context);
    }

  } else if (o_events & MCU_EVENT_FLAG_DATA_READY) { // Data out stage
    if (
      usbd_control_setup_request_direction(context)
      == USBD_REQUEST_TYPE_DIRECTION_HOST_TO_DEVICE) {
      if (context->data.nbyte) {
        usbd_control_dataout_stage(context);
        if (context->data.nbyte == 0) {
          if (usbd_control_setup_request_type(context) == USBD_REQUEST_STANDARD) {
            stall(context);
          } else if (execute_class_handler(context, usb_event) == 0) {
            stall(context);
          }
        }
      }
    } else {
      usbd_control_statusout_stage(context);
    }

  } else if (o_events & MCU_EVENT_FLAG_WRITE_COMPLETE) {
    if (
      usbd_control_setup_request_direction(context)
      == USBD_REQUEST_TYPE_DIRECTION_DEVICE_TO_HOST) {

      if (execute_class_handler(context, usb_event)) {
        return 1;
      }
      usbd_control_datain_stage(context);
    }
  } else if (o_events & MCU_EVENT_FLAG_STALL) {
    usbd_control_unstall_endpoint(
      context->handle, ((usb_event_t *)usb_event->data)->epnum);
  }

  return 1;
}

void *usbd_control_add_ptr(usbd_control_t *context, void *ptr, u32 value) {
  return (char *)ptr + value;
}

void usbd_control_handler_setup_stage(usbd_control_t *context) {
  sos_config.usb.read_endpoint(context->handle, 0x00, (u8 *)&(context->setup_packet));
  context->data.nbyte = context->setup_packet.wLength;
  context->data.is_zlp = 0;
}

void usbd_control_datain_stage(usbd_control_t *context) {
  u32 nbyte;

  // we can only send max packet size at a time
  if (context->data.nbyte >= sos_config.usb.control_endpoint_max_size) {
    nbyte = sos_config.usb.control_endpoint_max_size;
    context->data.is_zlp = 1;
  } else {
    nbyte = context->data.nbyte;
    context->data.is_zlp = 0;
  }

  if (
    (nbyte > 0) || (context->data.is_zlp) // need to send a ZLP to mark the end
  ) {
    sos_config.usb.write_endpoint(context->handle, 0x80, context->data.dptr, nbyte);
    context->data.dptr += nbyte;
    context->data.nbyte -= nbyte;
    if (nbyte == 0) {
      context->data.is_zlp = 0;
    }
  }
}

void usbd_control_dataout_stage(usbd_control_t *context) {
  u32 nbyte;
  nbyte = sos_config.usb.read_endpoint(context->handle, 0x00, context->data.dptr);
  if (nbyte > context->data.nbyte) {
    nbyte = context->data.nbyte;
  }
  context->data.dptr += nbyte;
  context->data.nbyte -= nbyte;
}

// send a zero length packet
void usbd_control_statusin_stage(usbd_control_t *context) {
  sos_config.usb.write_endpoint(context->handle, 0x80, NULL, 0);
}

// receive a zero length packet
void usbd_control_statusout_stage(usbd_control_t *context) {
  sos_config.usb.read_endpoint(context->handle, 0x00, context->buf);
}

int usbd_control_reset(const devfs_handle_t *handle) {
  usb_attr_t attr;
  attr.o_flags = USB_FLAG_RESET;
  return sos_config.usb.set_attributes(handle, &attr);
}

int usbd_control_attach(const devfs_handle_t *handle) {
  usb_attr_t attr;
  attr.o_flags = USB_FLAG_ATTACH;
  return sos_config.usb.set_attributes(handle, &attr);
}

int usbd_control_detach(const devfs_handle_t *handle) {
  usb_attr_t attr;
  attr.o_flags = USB_FLAG_DETACH;
  return sos_config.usb.set_attributes(handle, &attr);
}

int usbd_control_configure(const devfs_handle_t *handle) {
  usb_attr_t attr;
  attr.o_flags = USB_FLAG_CONFIGURE;
  return sos_config.usb.set_attributes(handle, &attr);
}

int usbd_control_unconfigure(const devfs_handle_t *handle) {
  usb_attr_t attr;
  attr.o_flags = USB_FLAG_UNCONFIGURE;
  return sos_config.usb.set_attributes(handle, &attr);
}

int usbd_control_set_address(const devfs_handle_t *handle, u8 address) {
  usb_attr_t attr;
  attr.o_flags = USB_FLAG_SET_ADDRESS;
  attr.address = address;
  return sos_config.usb.set_attributes(handle, &attr);
}

int usbd_control_reset_endpoint(const devfs_handle_t *handle, u8 endpoint_number) {
  usb_attr_t attr;
  attr.o_flags = USB_FLAG_RESET_ENDPOINT;
  attr.address = endpoint_number;
  return sos_config.usb.set_attributes(handle, &attr);
}

int usbd_control_enable_endpoint(const devfs_handle_t *handle, u8 endpoint_number) {
  usb_attr_t attr;
  attr.o_flags = USB_FLAG_ENABLE_ENDPOINT;
  attr.address = endpoint_number;
  return sos_config.usb.set_attributes(handle, &attr);
}

int usbd_control_disable_endpoint(const devfs_handle_t *handle, u8 endpoint_number) {
  usb_attr_t attr;
  attr.o_flags = USB_FLAG_DISABLE_ENDPOINT;
  attr.address = endpoint_number;
  return sos_config.usb.set_attributes(handle, &attr);
}

int usbd_control_stall_endpoint(const devfs_handle_t *handle, u8 endpoint_number) {
  usb_attr_t attr;
  attr.o_flags = USB_FLAG_STALL_ENDPOINT;
  attr.address = endpoint_number;
  return sos_config.usb.set_attributes(handle, &attr);
}

int usbd_control_unstall_endpoint(const devfs_handle_t *handle, u8 endpoint_number) {
  usb_attr_t attr;
  attr.o_flags = USB_FLAG_UNSTALL_ENDPOINT;
  attr.address = endpoint_number;
  return sos_config.usb.set_attributes(handle, &attr);
}

int usbd_control_configure_endpoint(
  const devfs_handle_t *handle,
  const usbd_endpoint_descriptor_t *endpoint_descriptor) {
  usb_attr_t attr;
  attr.o_flags = USB_FLAG_CONFIGURE_ENDPOINT;
  attr.address = endpoint_descriptor->bEndpointAddress;
  attr.max_packet_size = endpoint_descriptor->wMaxPacketSize;
  attr.type = endpoint_descriptor->bmAttributes;
  return sos_config.usb.set_attributes(handle, &attr);
}

/*

Copyright 2011-2016 Tyler Gilbert

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

#include <errno.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include "sos/link/link.h"
#include "mcu/mcu.h"
#include "sos/dev/usb.h"
#include "mcu/usbfifo.h"
#include "mcu/usbd/control.h"
#include "mcu/usbd/cdc.h"
#include "mcu/core.h"
#include "mcu/usb.h"
#include "mcu/debug.h"
#include "mcu/sys.h"
#include "sos/sos_link_transport_usb.h"


static devfs_async_t notify_op;


link_transport_phy_t sos_link_transport_usb_open(const char * name, usbd_control_t * context){
	link_transport_phy_t fd;
	usb_attr_t usb_attr;

	//open USB
	mcu_debug("Open link-phy-usb\n");
	errno = 0;
	fd = open("/dev/link-phy-usb", O_RDWR);
	if( fd < 0 ){
		mcu_debug("Failed to open link-phy-usb (%d)\n", errno);
		return LINK_PHY_ERROR;
	}

	//set USB attributes
	mcu_debug("Set USB attr fd:%d\n", fd);

	memcpy(usb_attr.pin_assignment, mcu_board_config.usb_pin_assignment, sizeof(mcu_pin_t)*USB_PIN_ASSIGNMENT_COUNT);
	usb_attr.o_flags = USB_FLAG_SET_DEVICE;
	usb_attr.freq = mcu_board_config.core_osc_freq;
	if( ioctl(fd, I_USB_SETATTR, &usb_attr) < 0 ){
		mcu_debug("Failed to set USB attr\n");
		return LINK_PHY_ERROR;
	}

	mcu_debug("USB Dev Init\n");
	//initialize USB device
	mcu_core_privcall(usbd_control_priv_init, context);

	return fd;
}

int sos_link_transport_usb_write(link_transport_phy_t handle, const void * buf, int nbyte){
	int ret;
	ret = write(handle, buf, nbyte);
	return ret;
}

void sos_link_transport_usb_notify(const void * buf, int nbyte){
	devfs_handle_t usb;
	usb.port = STRATIFY_LINK_TRANSPORT_USB_PORT;

	notify_op.loc = STRATIFY_LINK_TRANSPORT_USB_BULK_ENDPOINT_IN_ALT;
	notify_op.buf_const = buf;
	notify_op.nbyte = nbyte;
	notify_op.tid = task_get_current();
	notify_op.handler.callback = 0;
	notify_op.handler.context = 0;
	notify_op.flags = O_NONBLOCK | O_RDWR;

	mcu_usb_write(&usb, &notify_op);
}

int sos_link_transport_usb_read(link_transport_phy_t handle, void * buf, int nbyte){
	int ret;
	errno = 0;
	ret = read(handle, buf, nbyte);
	return ret;
}

int sos_link_transport_usb_close(link_transport_phy_t * handle){
	link_transport_phy_t fd = *handle;
	*handle = -1;
	return close(fd);
}

void sos_link_transport_usb_wait(int msec){
	int i;
	for(i = 0; i < msec; i++){
		usleep(1000);
	}
}

void sos_link_transport_usb_flush(link_transport_phy_t handle){
	ioctl(handle, I_FIFO_FLUSH);
}


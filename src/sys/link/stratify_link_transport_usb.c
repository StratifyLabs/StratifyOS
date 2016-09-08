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
#include "iface/link.h"
#include "mcu/mcu.h"
#include "iface/dev/usb.h"
#include "dev/usbfifo.h"
#include "stratify/usb_dev.h"
#include "stratify/usb_dev_cdc.h"
#include "mcu/core.h"
#include "mcu/debug.h"
#include "stratify/usb_dev_typedefs.h"
#include "stratify/usb_dev_defs.h"
#include "dev/sys.h"
#include "iface/stratify_link_transport_usb.h"


#ifdef __STDIO_VCP
void init_stdio_vcp(){
	int fd;
	fd = open("/dev/stdio", O_RDWR);
	if( fd < 0 ){
		return;
	}

	ioctl(fd, I_FIFO_INIT);
	close(fd);
	return;
}
#endif


static usb_dev_context_t usb_context;


link_transport_phy_t stratify_link_transport_usb_open(const char * name, int baudrate){
	link_transport_phy_t fd;
	usb_attr_t usb_attr;

	//open USB
	mcu_debug("Open link-phy-usb\n");
	fd = open("/dev/link-phy-usb", O_RDWR);
	if( fd < 0 ){
		return LINK_PHY_ERROR;
	}

	//set USB attributes
	mcu_debug("Set USB attr fd:%d\n", fd);

	usb_attr.pin_assign = 0;
	usb_attr.mode = USB_ATTR_MODE_DEVICE;
	usb_attr.crystal_freq = mcu_board_config.core_osc_freq;
	if( ioctl(fd, I_USB_SETATTR, &usb_attr) < 0 ){
		return LINK_PHY_ERROR;
	}

	mcu_debug("USB Dev Init\n");
	//initialize USB device
	memset(&usb_context, 0, sizeof(usb_context));
	usb_context.constants = &stratify_link_transport_usb_constants;
	mcu_core_privcall(usb_dev_priv_init, &usb_context);


#ifdef __STDIO_VCP
	init_stdio_vcp();
#endif


	return fd;
}

int stratify_link_transport_usb_write(link_transport_phy_t handle, const void * buf, int nbyte){
	int ret;
	ret = write(handle, buf, nbyte);
	return ret;
}

int stratify_link_transport_usb_read(link_transport_phy_t handle, void * buf, int nbyte){
	int ret;
	errno = 0;
	ret = read(handle, buf, nbyte);
	return ret;
}

int stratify_link_transport_usb_close(link_transport_phy_t handle){
	return close(handle);
}

void stratify_link_transport_usb_wait(int msec){
	int i;
	for(i = 0; i < msec; i++){
		usleep(1000);
	}
}

void stratify_link_transport_usb_flush(link_transport_phy_t handle){
	ioctl(handle, I_FIFO_FLUSH);
}


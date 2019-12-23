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

#include <errno.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include "sos/link.h"
#include "mcu/mcu.h"
#include "sos/dev/usb.h"
#include "device/usbfifo.h"
#include "usbd/control.h"
#include "mcu/core.h"
#include "mcu/usb.h"
#include "mcu/debug.h"
#include "device/sys.h"
#include "sos/link/transport_usb.h"


#define USB0_DEVFIFO_BUFFER_SIZE 64
static char usb0_fifo_buffer[USB0_DEVFIFO_BUFFER_SIZE] MCU_SYS_MEM;
static char usb0_read_buffer[SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_SIZE] MCU_ALIGN(32);
const usbfifo_config_t sos_link_transport_usb_fifo_cfg = {
	.endpoint = SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT,
	.endpoint_size = SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_SIZE,
	.read_buffer = usb0_read_buffer,
	.fifo = {
		.buffer = usb0_fifo_buffer,
		.size = USB0_DEVFIFO_BUFFER_SIZE
	}
};


usbfifo_state_t sos_link_transport_usb_fifo_state MCU_SYS_MEM;

static int open_pio(mcu_pin_t pin, int active_high){
	char path[PATH_MAX];
	int fd;
	pio_attr_t attr;
	int len;

	strncpy(path, "/dev/pio", PATH_MAX-2);
	len = strnlen(path, PATH_MAX-2);
	path[len] = pin.port + '0';
	path[len+1] = 0;

	fd = open(path, O_RDWR);
	if( fd < 0 ){
		return -1;
	}

	attr.o_pinmask = (1<<pin.pin);
	if( active_high ){
		ioctl(fd, I_PIO_CLRMASK, (void*)attr.o_pinmask);
	} else {
		ioctl(fd, I_PIO_SETMASK, (void*)attr.o_pinmask);
	}
	attr.o_flags = PIO_FLAG_SET_OUTPUT | PIO_FLAG_IS_DIRONLY;
	ioctl(fd, I_PIO_SETATTR, &attr);


	return fd;
}

link_transport_phy_t sos_link_transport_usb_open(
		const char * name,
		usbd_control_t * context,
		const usbd_control_constants_t * constants,
		const usb_attr_t * usb_attr,
		mcu_pin_t usb_up_pin,
		int usb_up_active_high){

	int fd;
	int pio_fd;
	int result;
	MCU_UNUSED_ARGUMENT(name);

	if( usb_up_pin.port != 0xff ){
		pio_fd = open_pio(usb_up_pin, usb_up_active_high);
		if( pio_fd < 0 ){
			mcu_debug_log_error(MCU_DEBUG_USB | MCU_DEBUG_LINK, "Failed to open PIO (%d)\n", errno);
			return LINK_PHY_ERROR;
		}
	} else {
		pio_fd = -1;
	}

	memset(context, 0, sizeof(usbd_control_t));
	context->constants = constants;
	context->handle = &(constants->handle);

	usleep(100*1000);

	//open USB
	mcu_debug_log_info(MCU_DEBUG_USB | MCU_DEBUG_LINK, "Open link-phy-usb");
	errno = 0;
	fd = open("/dev/link-phy-usb", O_RDWR);
	if( fd < 0 ){
		mcu_debug_log_error(MCU_DEBUG_USB | MCU_DEBUG_LINK, "Failed to open link-phy-usb (%d)", errno);
		return LINK_PHY_ERROR;
	}

	//set USB attributes
	mcu_debug_log_info(MCU_DEBUG_USB | MCU_DEBUG_LINK, "Set USB attr fd:%d", fd);

	result = ioctl(fd, I_USB_SETATTR, usb_attr);
	if( result < 0 ){
		mcu_debug_log_error(MCU_DEBUG_USB | MCU_DEBUG_LINK, "Failed to set USB attr (%d, %d)", result, errno);
		return LINK_PHY_ERROR;
	}

	mcu_debug_log_info(MCU_DEBUG_USB | MCU_DEBUG_LINK, "USB Dev Init");
	//initialize USB device
	cortexm_svcall(usbd_control_svcall_init, context);

	if( pio_fd >= 0 ){
		if( usb_up_active_high ){
			ioctl(pio_fd, I_PIO_SETMASK, (void*)(1<<usb_up_pin.pin));
		} else {
			ioctl(pio_fd, I_PIO_CLRMASK, (void*)(1<<usb_up_pin.pin));
		}
		close(pio_fd);
	}

	return fd;
}

int sos_link_transport_usb_write(link_transport_phy_t handle, const void * buf, int nbyte){
	int ret;
	ret = write(handle, buf, nbyte);
	//mcu_debug_printf("%d bytes written\n", ret);
	return ret;
}

int sos_link_transport_usb_read(link_transport_phy_t handle, void * buf, int nbyte){
	int ret;
	ret = read(handle, buf, nbyte);
	//mcu_debug_printf("%d bytes arrived\n", ret);
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


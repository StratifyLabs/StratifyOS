/* Copyright 2011-2016 Tyler Gilbert; 
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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * 
 */

#include <stdbool.h>
#include <stdarg.h>

#include "mcu/mcu.h"
#include "link_flags.h"

static int reset_device(link_transport_mdriver_t * driver, bool invoke_bootloader);

int link_bootloader_attr(link_transport_mdriver_t * driver, bootloader_attr_t * attr, uint32_t id){
	if( link_ioctl(driver, LINK_BOOTLOADER_FILDES, I_BOOTLOADER_GETATTR, attr) < 0 ){
		//try again with the older version
		attr->version = 0;
		//link_ioctl(driver, LINK_BOOTLOADER_FILDES, I_BOOTLOADER_GETATTR_14X, attr);
		if( (attr->version == 0x150) ||
				(attr->version == 0x140) ||
				(attr->version == 0x132) ||
				(attr->version == 0x131) ||
				(attr->version == 0x130) ){
			attr->hardware_id = id;
			return 0;
		}

		return -1;
	}

	return 0;
}

int link_isbootloader(link_transport_mdriver_t * driver){
	bootloader_attr_t attr;

	if( link_bootloader_attr(driver, &attr, 0) < 0 ){
		return 0;
	}

	/*
	bootloader_attr_t attr;
	bootloader_attr_14x_t attr_14x;

	if( link_ioctl(handle, LINK_BOOTLOADER_FILDES, I_BOOTLOADER_GETATTR, &attr) < 0 ){

		attr_14x.version = 0;
		link_ioctl(handle, LINK_BOOTLOADER_FILDES, I_BOOTLOADER_GETATTR_14X, &attr_14x);
			//If this fails, no bootloader

		if( (attr_14x.version == 0x150) ||
				(attr_14x.version == 0x140) ||
				(attr_14x.version == 0x132) ||
				(attr_14x.version == 0x131) ||
				(attr_14x.version == 0x130) ){
			link_debug(LINK_DEBUG_WARNING, "Failed to respond to ioctl 0x%X", attr_14x.version);
			return 0;
		}

	}
	*/

	link_debug(LINK_DEBUG_MESSAGE, "Bootloader Version is 0x%X", attr.version);

	//If the above succeeds, the bootloader is present
	return 1;
}

int link_reset(link_transport_mdriver_t * driver){
	link_op_t op;
	link_debug(LINK_DEBUG_MESSAGE, "try to reset--check bootloader");
	if( link_isbootloader(driver) ){
		//execute the request
		op.ioctl.cmd = LINK_CMD_IOCTL;
		op.ioctl.fildes = LINK_BOOTLOADER_FILDES;
		op.ioctl.request = I_BOOTLOADER_RESET;
		op.ioctl.arg = 0;
		link_transport_masterwrite(driver, &op, sizeof(link_ioctl_t));
		driver->dev.close(&(driver->dev.handle));
	} else {
		link_debug(LINK_DEBUG_MESSAGE, "reset device with /dev/core");
		return reset_device(driver, false);
	}
	return 0;
}

int reset_device(link_transport_mdriver_t * driver, bool invoke_bootloader){
	//use "/dev/core" to reset
	int fd;
	link_op_t op;

	fd = link_open(driver, "/dev/core", LINK_O_RDWR);
	if ( fd < 0 ){
		return -1;
	}

	op.ioctl.cmd = LINK_CMD_IOCTL;
	op.ioctl.fildes = fd;
	op.ioctl.arg = (size_t)NULL;

	if( invoke_bootloader == false ){
		link_debug(LINK_DEBUG_MESSAGE, "Try to reset");
		op.ioctl.request = I_CORE_RESET;

	} else {
		link_debug(LINK_DEBUG_MESSAGE, "Try to invoke bootloader");
		op.ioctl.request = I_CORE_INVOKEBOOTLOADER;
	}

	link_transport_masterwrite(driver, &op, sizeof(link_ioctl_t));
	driver->dev.close(&(driver->dev.handle));
	return 0;
}

int link_resetbootloader(link_transport_mdriver_t * driver){
	return reset_device(driver, true);
}

int link_eraseflash(link_transport_mdriver_t * driver){
	if( link_ioctl_delay(driver, LINK_BOOTLOADER_FILDES, I_BOOTLOADER_ERASE, NULL, 0, 500) < 0 ){
		return -1;
	}

	driver->dev.wait(500);
	driver->dev.wait(500);
	return 0;
}

int link_readflash(link_transport_mdriver_t * driver, int addr, void * buf, int nbyte){
	link_op_t op;
	link_reply_t reply;
	int err;
	int len;

	op.read.cmd = LINK_CMD_READ;
	op.read.addr = addr;
	op.read.nbyte = nbyte;


	link_debug(LINK_DEBUG_MESSAGE, "write read flash op");
	err = link_transport_masterwrite(driver, &op, sizeof(link_read_t));
	if ( err < 0 ){
		return err;
	}

	link_debug(LINK_DEBUG_MESSAGE, "read flash data");
	len = link_transport_masterread(driver, buf, nbyte);
	if ( len < 0 ){
		return LINK_TRANSFER_ERR;
	}

	link_debug(LINK_DEBUG_MESSAGE, "read reply");
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		return err;
	}

	if ( reply.err < 0 ){
		link_errno = reply.err_number;
	}

	link_debug(LINK_DEBUG_MESSAGE, "Read %d bytes from device", reply.err);

	return reply.err;
}

int link_writeflash(link_transport_mdriver_t * driver, int addr, const void * buf, int nbyte){
	bootloader_writepage_t wattr;
	int page_size;
	int bytes_written;
	int err;

	bytes_written = 0;
	wattr.addr = addr;
	page_size = BOOTLOADER_WRITEPAGESIZE;
	if( page_size > nbyte ){
		page_size = nbyte;
	}
	wattr.nbyte = page_size;

	link_debug(LINK_DEBUG_MESSAGE, "Page size is %d (%d)", page_size, nbyte);

	do {
		memset(wattr.buf, 0xFF, BOOTLOADER_WRITEPAGESIZE);
		memcpy(wattr.buf, buf, page_size);


		err = link_ioctl_delay(driver, LINK_BOOTLOADER_FILDES, I_BOOTLOADER_WRITEPAGE, &wattr, 0, 0);
		if( err < 0 ){
			return err;
		}

		wattr.addr += page_size;
		buf += page_size;
		bytes_written += page_size;

	} while(bytes_written < nbyte);
	return nbyte;
}

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

#include <string.h>
#include <stdarg.h>

#include "sos/dev/bootloader.h"
#include "link_local.h"

static int reset_device(link_transport_mdriver_t * driver, int invoke_bootloader);

int link_bootloader_attr(link_transport_mdriver_t * driver, bootloader_attr_t * attr, u32 id){
    link_errno = 0;
    int ret = link_ioctl(driver, LINK_BOOTLOADER_FILDES, I_BOOTLOADER_GETINFO, attr);
    if( ret < 0 ){
        if( link_errno == 9 ){ //EBADF
            link_debug(LINK_DEBUG_MESSAGE, "Device is present but is not a bootloader");
            return LINK_DEVICE_PRESENT_BUT_NOT_BOOTLOADER;
        }
        return -1;
	}

	return 0;
}

int link_bootloader_attr_legacy(link_transport_mdriver_t * driver, bootloader_attr_t * attr, u32 id){
    bootloader_attr_legacy_t legacy_attr;
    int ret;
    link_errno = 0;
    if( (ret = link_ioctl(driver, LINK_BOOTLOADER_FILDES, I_BOOTLOADER_GETATTR_LEGACY, &legacy_attr)) < 0 ){
        if( link_errno != 0 ){
            link_debug(LINK_DEBUG_MESSAGE, "Legacy Device is present but is not a bootloader");
            return LINK_DEVICE_PRESENT_BUT_NOT_BOOTLOADER;
        }
        return -1;
    }

    memcpy(attr, &legacy_attr, sizeof(legacy_attr));
    attr->hardware_id = 0x00000001; //CoAction Hero is the only board with legacy bootloader installed

    return 0;
}

int link_isbootloader_legacy(link_transport_mdriver_t * driver){
    bootloader_attr_t attr;
    int ret = link_bootloader_attr_legacy(driver, &attr, 0);
    if( ret == LINK_DEVICE_PRESENT_BUT_NOT_BOOTLOADER ){
        return 0;
    } else if ( ret < 0 ){
        return -1;
    }
    return 1;
}

int link_isbootloader(link_transport_mdriver_t * driver){
	bootloader_attr_t attr;
    int ret;

    ret = link_bootloader_attr(driver, &attr, 0);

    if( ret == LINK_DEVICE_PRESENT_BUT_NOT_BOOTLOADER ){
        return 0;
    } else if ( ret < 0 ){
        return -1;
    }

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
		return reset_device(driver, 0);
	}
	return 0;
}

int reset_device(link_transport_mdriver_t * driver, int invoke_bootloader){
	//use "/dev/core" to reset
	int fd;
	core_attr_t attr;
	int ret = 0;

	fd = link_open(driver, "/dev/core", LINK_O_RDWR);
	if ( fd < 0 ){
		ret = -1;
	} else {
		if( invoke_bootloader == 0 ){
			link_debug(LINK_DEBUG_MESSAGE, "Try to reset");
			attr.o_flags = CORE_FLAG_EXEC_RESET;
		} else {
			link_debug(LINK_DEBUG_MESSAGE, "Try to invoke bootloader");
			attr.o_flags = CORE_FLAG_EXEC_INVOKE_BOOTLOADER;
		}

		if( link_ioctl(driver, fd, I_CORE_SETATTR, &attr) < 0 ){
			//try the old version of the bootloader
			if( link_ioctl(driver, fd, I_CORE_INVOKEBOOTLOADER_2, &attr) < 0 ){
				ret = -1;
			}
		}
		//since the device has been reset -- close the handle
		driver->dev.close(&(driver->dev.handle));
	}
	return ret;
}

int link_resetbootloader(link_transport_mdriver_t * driver){
	return reset_device(driver, 1);
}

int link_eraseflash(link_transport_mdriver_t * driver){
    if( link_ioctl_delay(driver, LINK_BOOTLOADER_FILDES, I_BOOTLOADER_ERASE, NULL, 0, 750) < 0 ){
		return -1;
	}

	driver->dev.wait(500);
	driver->dev.wait(500);
	driver->dev.wait(500);
	driver->dev.wait(500);
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

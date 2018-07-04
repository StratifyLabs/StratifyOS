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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * 
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include "sos/fs/sffs.h"
#include "sys/sffs/sffs_dev.h"
#include "sos/dev/drive.h"
#include "../scheduler/scheduler_local.h"
#include "../unistd/unistd_local.h"

#include "mcu/debug.h"


int sffs_dev_getlist_block(const void * cfg){
    return SFFS_STATE(cfg)->list_block;
}

void sffs_dev_setlist_block(const void * cfg, int list_block){
    SFFS_STATE(cfg)->list_block = list_block;
}

void sffs_dev_setdelay_mutex(pthread_mutex_t * mutex){
	cortexm_svcall(scheduler_root_set_delaymutex, mutex);
}

int sffs_dev_getserialno(const void * cfg){
    return SFFS_STATE(cfg)->serialno;
}

void sffs_dev_setserialno(const void * cfg, int serialno){
    SFFS_STATE(cfg)->serialno = serialno;
}

int sffs_dev_open(const void * cfg){
    int result;
    result = sysfs_shared_open(SFFS_DRIVE(cfg));
    if( result < 0 ){ return result; }
    return sysfs_shared_ioctl(SFFS_DRIVE(cfg), I_DRIVE_GETINFO, &(SFFS_STATE(cfg)->dattr));
}

int sffs_dev_write(const void * cfg, int loc, const void * buf, int nbyte){
	int ret;
	//int i;
	char buffer[nbyte];
    ret = sysfs_shared_write(SFFS_DRIVE(cfg), loc,  buf, nbyte);
    if( ret < 0 ){ return ret; }
	memset(buffer, 0, nbyte);
    sysfs_shared_read(SFFS_DRIVE(cfg), loc, buffer, nbyte);
	if ( memcmp(buffer, buf, nbyte) != 0 ){
        return SYSFS_SET_RETURN(EIO);
	}

	return ret;
}

int sffs_dev_read(const void * cfg, int loc, void * buf, int nbyte){
    return sysfs_shared_read(SFFS_DRIVE(cfg), loc, buf, nbyte);
}


int sffs_dev_erase(const void * cfg){
	drive_attr_t attr;
	int usec;
	attr.o_flags = DRIVE_FLAG_ERASE_DEVICE;
    if( sysfs_shared_ioctl(SFFS_DRIVE(cfg), I_DRIVE_SETATTR, &attr) < 0 ){
		return -1;
	}

    usec = SFFS_STATE(cfg)->dattr.erase_device_time;

	usleep(usec);
	return 0;
}

int sffs_dev_erasesection(const void * cfg, int loc){
	drive_attr_t attr;
    int result;
	int usec;

	attr.o_flags = DRIVE_FLAG_ERASE_BLOCKS;
	attr.start = loc;
	attr.end = loc;

    if( (result = sysfs_shared_ioctl(SFFS_DRIVE(cfg), I_DRIVE_SETATTR, &attr)) < 0 ){
        return result;
	}


    usec = SFFS_STATE(cfg)->dattr.erase_block_time;

	usleep(usec);
	return 0;
}

int sffs_dev_close(const void * cfg){
    return sysfs_shared_close(SFFS_DRIVE(cfg));
}


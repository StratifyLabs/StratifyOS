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

#include <reent.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>
#include "sos/fs/sysfs.h"

#include "mcu/mcu.h"
#include "../unistd/unistd_fs.h"
#include "../scheduler/scheduler_local.h"
#include "../unistd/unistd_local.h"

#include "sos/fs/devfs.h"
#include "sos/fs/sysfs.h"

#include "devfs_local.h"

typedef struct {
	int err;
	int mode;
	const devfs_device_t * device;
} root_args_t;


static void svcall_open_device(void * args) MCU_ROOT_EXEC_CODE;
static void svcall_close_device(void * args) MCU_ROOT_EXEC_CODE;
static int get_total(const devfs_device_t * list);
static void svcall_ioctl(void * args) MCU_ROOT_EXEC_CODE;

int get_total(const devfs_device_t * list){
	int total;
	total = 0;
	while( list[total].driver.open != NULL ){
		total++;
	}
	return total;
}

const devfs_device_t * devfs_lookup_device(const devfs_device_t * list, const char * device_name){
	int i;
	i = 0;
	while( devfs_is_terminator(&(list[i])) == 0 ){
		if ( strncmp(device_name, list[i].name, NAME_MAX) == 0 ){
			return &list[i];
		}
		i++;
	}
	return 0;
}

int devfs_lookup_name(const devfs_device_t * list, const devfs_device_t * device, char name[NAME_MAX]){
	int i;
	i = 0;
	while( devfs_is_terminator(&(list[i])) == 0 ){
		if( device == (list + i) ){
			name[NAME_MAX-1] = 0;
			strncpy(name, list[i].name, NAME_MAX-1);
			return 0;
		}
		i++;
	}
	return -1;
}

const devfs_handle_t * devfs_lookup_handle(const devfs_device_t * list, const char * name){
	const devfs_device_t * device = devfs_lookup_device(list, name);
	if( device != 0 ){
		return &(device->handle);
	}
	return 0;
}


void svcall_open_device(void * args){
	CORTEXM_SVCALL_ENTER();
	root_args_t * p = (root_args_t*)args;

	if( p->mode == O_RDONLY ){
		if( sysfs_is_r_ok(p->device->mode, p->device->uid, SYSFS_GROUP) == 0 ){
			p->err = SYSFS_SET_RETURN(EPERM);
			return;
		}
	} else if( p->mode == O_WRONLY ){
		if( sysfs_is_w_ok(p->device->mode, p->device->uid, SYSFS_GROUP) == 0 ){
			p->err = SYSFS_SET_RETURN(EPERM);
			return;
		}
	} else {
		if( sysfs_is_rw_ok(p->device->mode, p->device->uid, SYSFS_GROUP) == 0 ){
			p->err = SYSFS_SET_RETURN(EPERM);
			return;
		}
	}

	p->err = p->device->driver.open( &(p->device->handle) );
}

int devfs_init(const void * cfg){
	//no initialization is required
	return 0;
}

int devfs_opendir(const void * cfg, void ** handle, const char * path){
	const devfs_device_t * dev;
	const devfs_device_t * list = (const devfs_device_t*)cfg;

	if ( strncmp(path, "", PATH_MAX) != 0 ){
		//there is only one valid folder (the top)
		dev = devfs_lookup_device(list, path);
		if ( dev == 0 ){
			return SYSFS_SET_RETURN(ENOENT);
		} else {
			return SYSFS_SET_RETURN(ENOTDIR);
		}
	}
	*handle = NULL;
	return 0;
}

int devfs_readdir_r(const void * cfg, void * handle, int loc, struct dirent * entry){
	int total;
	const devfs_device_t * dev_list = (const devfs_device_t*)cfg; //the cfg value is the device list
	//this populates the entry for the loc position
	total = get_total(dev_list);
	if ( (uint32_t)loc < total ){
		strcpy(entry->d_name, dev_list[loc].name);
		entry->d_ino = loc;
		return 0;
	}
	return SYSFS_SET_RETURN(ENOENT);
}

int devfs_closedir(const void * cfg, void ** handle){
	//This doesn't need to do anything
	return 0;
}

int devfs_open(const void * cfg, void ** handle, const char * path, int flags, int mode){
	root_args_t args;
	const devfs_device_t * list = (const devfs_device_t*)cfg;

	//check the flags O_CREAT, O_APPEND, O_TRUNC are not supported
	if ( (flags & O_APPEND) | (flags & O_CREAT) | (flags & O_TRUNC) ){
		return SYSFS_SET_RETURN(ENOTSUP);
	}


	//Check to see if the device is in the list
	args.device = devfs_lookup_device(list, path);
	if ( args.device != NULL ){
		args.mode = flags & O_ACCMODE;
		cortexm_svcall(svcall_open_device, &args);
		if ( args.err < 0 ){
			return args.err;
		}
		*handle = (void*)args.device;
		return 0;
	}
	return SYSFS_SET_RETURN(ENOENT);
}

int devfs_fstat(const void * cfg, void * handle, struct stat * st){
	const devfs_device_t * dev = handle;
	//populate the characteristics
	int num;
	num = (dev - devfs_list)/sizeof(devfs_device_t);
	st->st_dev = 0;
	st->st_rdev = num;
	st->st_ino = num;
	st->st_size = 0;
	st->st_uid = dev->uid;
	st->st_mode = dev->mode;
	return 0;
}

int devfs_stat(const void * cfg, const char * path, struct stat * st){
	//populate the characteristics of the device
	const devfs_device_t * list = (const devfs_device_t*)cfg;
	const devfs_device_t * dev;

	dev = devfs_lookup_device(list, path);
	if ( dev == NULL ){
		return SYSFS_SET_RETURN(ENOENT);
	}
	return devfs_fstat(cfg, (void*)dev, st);
}

int devfs_read(const void * config, void * handle, int flags, int loc, void * buf, int nbyte){
	//config points to the list of devices (not used because handle already has the device)
	//handle points to the actual device
	return devfs_data_transfer(config, handle, flags, loc, buf, nbyte, 1);
}

int devfs_write(const void * cfg, void * handle, int flags, int loc, const void * buf, int nbyte){
	return devfs_data_transfer(cfg, handle, flags, loc, (void*)buf, nbyte, 0);
}

int devfs_aio(const void * config, void * handle, struct aiocb * aio){
	return devfs_aio_data_transfer(handle, aio);
}

int devfs_ioctl(const void * cfg, void * handle, int request, void * ctl){
	sysfs_ioctl_t args;

	if( request == I_DEVFS_GETNAME ){
		const devfs_device_t * list = (const devfs_device_t*)cfg;
		return devfs_lookup_name(list, handle, ctl);
	}

	args.cfg = cfg;
	args.handle = handle;
	args.request = request;
	args.ctl = ctl;
	args.result = 0;
	cortexm_svcall(svcall_ioctl, &args);
	return args.result;
}

void svcall_ioctl(void * args){
	CORTEXM_SVCALL_ENTER();
	sysfs_ioctl_t * p = args;
	const devfs_device_t * dev = (const devfs_device_t*)p->handle;

	//check permissions on this device - IOCTL needs read/write access
	if( sysfs_is_rw_ok(dev->mode, dev->uid, SYSFS_GROUP) == 0 ){
		p->result = SYSFS_SET_RETURN(EPERM);
		return;
	}

	//for IOCTLR, IOCTLW or IOCTLRW - check that p->ctl
	//is in a memory location that belongs to the process Issue #126
	if( p->ctl ){ //a null value can have meaning for some drivers -- all IOCTL drivers should check for a null ctl value
		if( _IOCTL_IOCTLRW(p->request) ){
			u32 size = _IOCTL_SIZE(p->request);
			if( task_validate_memory(p->ctl, size) < 0 ){
				p->result = SYSFS_SET_RETURN(EPERM);
				return;
			}
		}
	}

	p->result = dev->driver.ioctl(&dev->handle, p->request, p->ctl);
}

void svcall_close_device(void * args){
	CORTEXM_SVCALL_ENTER();
	root_args_t * p = (root_args_t*)args;
	p->err = p->device->driver.close(&p->device->handle);
}

int devfs_close(const void * cfg, void ** handle){
	root_args_t args;
	args.err = 0;
	args.device = *handle;
	cortexm_svcall(svcall_close_device, &args);
	*handle = NULL;
	return args.err;
}


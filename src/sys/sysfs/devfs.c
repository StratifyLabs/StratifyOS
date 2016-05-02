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

#include <reent.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>
#include <stratify/sysfs.h>

#include "mcu/mcu.h"
#include "../unistd/unistd_fs.h"
#include "../unistd/unistd_flags.h"
#include "../sched/sched_flags.h"

#include "iface/device_config.h"
#include "../unistd/unistd_flags.h"

typedef struct {
	int err;
	const device_t * handle;
} priv_args_t;


static void priv_open_device(void * args) MCU_PRIV_EXEC_CODE;
static void priv_devfs_close(void * args) MCU_PRIV_EXEC_CODE;


static int get_total(const device_t * list){
	int total;
	total = 0;
	while( list[total].driver.open != NULL ){
		total++;
	}
	return total;
}

static const device_t * load(const device_t * list, const char * device_name){
	int i;
	i = 0;
	while( device_is_terminator(&(list[i])) == false ){
		if ( strcmp(device_name, list[i].name) == 0 ){
			return &list[i];
		}
		i++;
	}
	return NULL;
}

/*
static int find(const device_t * list, const device_t * dev){
	int i;
	i = 0;
	while( list[i].name != NULL ){
		if (&list[i] == dev){
			return i;
		}
		i++;
	}
	return -1;
}
*/

void priv_open_device(void * args){
	priv_args_t * p = (priv_args_t*)args;
	p->err = p->handle->driver.open( &p->handle->cfg );
}

int devfs_init(const void * cfg){
	//no initialization is required
	return 0;
}

int devfs_opendir(const void * cfg, void ** handle, const char * path){
	const device_t * dev;
	const device_t * list = (const device_t*)cfg;

	if ( strcmp(path, "") != 0 ){
		//there is only one valid folder (the top)
			dev = load(list, path);
		if ( dev == NULL ){
			errno = ENOENT;
		} else {
			errno = ENOTDIR;
		}
		return -1;
	}
	*handle = NULL;
	return 0;
}

int devfs_readdir_r(const void * cfg, void * handle, int loc, struct dirent * entry){
	int total;
	const device_t * dev_list = (const device_t*)cfg; //the cfg value is the device list
	//this populates the entry for the loc position
	total = get_total(dev_list);
	if ( (uint32_t)loc < total ){
		strcpy(entry->d_name, dev_list[loc].name);
		entry->d_ino = loc;
		return 0;
	}
	errno = ENOENT;
	return -1;
}

int devfs_closedir(const void * cfg, void ** handle){
	//This doesn't need to do anything
	return 0;
}


int devfs_open(const void * cfg, void ** handle, const char * path, int flags, int mode){
	priv_args_t args;
	const device_t * list = (const device_t*)cfg;

	//check the flags O_CREAT, O_APPEND, O_TRUNC are not supported
	if ( (flags & O_APPEND) | (flags & O_CREAT) | (flags & O_TRUNC) ){
		errno = ENOTSUP;
		return -1;
	}


	//Check to see if the device is in the list
	args.handle = load(list, path);
	if ( args.handle != NULL ){
		mcu_core_privcall(priv_open_device, &args);
		if ( args.err < 0 ){
			return args.err;
		}
		*handle = (void*)args.handle;
		return 0;
	}
	errno = ENOENT;
	return -1;
}

int devfs_fstat(const void * cfg, void * handle, struct stat * st){
	int num;
	const device_t * dev = handle;
	//populate the characteristics
	//num = find(list, dev);
	num = 0;
	st->st_dev = 0;
	st->st_rdev = num;
	st->st_ino = num;
	st->st_size = 0;
	st->st_uid = dev->uid;
	st->st_gid = dev->gid;
	st->st_mode = dev->mode;
	return 0;
}

int devfs_stat(const void * cfg, const char * path, struct stat * st){
	//populate the characteristics of the device
	const device_t * list = (const device_t*)cfg;
	const device_t * dev;

	dev = load(list, path);
	if ( dev == NULL ){
		errno = ENOENT;
		return -1;
	}
	return devfs_fstat(cfg, (void*)dev, st);
}

int devfs_priv_read(const void * cfg, void * handle, device_transfer_t * op){
	const device_t * dev;
	dev = (const device_t*)(handle);
	return dev->driver.read(&dev->cfg, op);
}

int devfs_priv_write(const void * cfg, void * handle, device_transfer_t * op){
	const device_t * dev;
	dev = (const device_t*)handle;
	return dev->driver.write(&dev->cfg, op);
}

int devfs_priv_ioctl(const void * cfg, void * handle, int request, void * ctl){
	const device_t * dev;
	dev = (const device_t*)handle;
	return dev->driver.ioctl(&dev->cfg, request, ctl);
}

void priv_devfs_close(void * args){
	priv_args_t * p = (priv_args_t*)args;
	p->err = p->handle->driver.close(&p->handle->cfg);
}

int devfs_close(const void * cfg, void ** handle){
	priv_args_t args;
	args.err = 0;
	args.handle = *handle;
	mcu_core_privcall(priv_devfs_close, &args);
	*handle = NULL;
	return args.err;
}


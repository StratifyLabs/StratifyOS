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
#include "../unistd/unistd_flags.h"
#include "../sched/sched_flags.h"

#include "mcu/debug.h"


int sffs_dev_getlist_block(const void * cfg){
	const sffs_cfg_t * cfgp = cfg;
	return cfgp->state->list_block;
}

void sffs_dev_setlist_block(const void * cfg, int list_block){
	const sffs_cfg_t * cfgp = cfg;
	cfgp->state->list_block = list_block;
}

void sffs_dev_setdelay_mutex(pthread_mutex_t * mutex){
	mcu_core_privcall(sched_priv_set_delaymutex, mutex);
}

int sffs_dev_getserialno(const void * cfg){
	const sffs_cfg_t * cfgp = cfg;
	return cfgp->state->serialno;
}

void sffs_dev_setserialno(const void * cfg, int serialno){
	const sffs_cfg_t * cfgp = cfg;
	cfgp->state->serialno = serialno;
}

int sffs_dev_open(const void * cfg){
	const sffs_cfg_t * cfgp = cfg;

	cfgp->open_file->flags = O_RDWR;
	cfgp->open_file->loc = 0;
	cfgp->open_file->fs = cfgp->devfs;
	cfgp->open_file->handle = NULL;

	if( cfgp->devfs->open(
			cfgp->devfs->cfg,
			&(cfgp->open_file->handle),
			cfgp->name,
			O_RDWR,
			0) < 0 ){
		mcu_debug("Failed to open device\n");
				return -1;
			}

	return u_ioctl(cfgp->open_file, I_DRIVE_GETINFO, &(cfgp->state->dattr));
}

int sffs_dev_write(const void * cfg, int loc, const void * buf, int nbyte){
	int ret;
	//int i;
	char buffer[nbyte];
	const sffs_cfg_t * cfgp = cfg;
	if ( cfgp->open_file->fs == NULL ){
		errno = EIO;
		return -1;
	}
	cfgp->open_file->loc = loc;
	ret = u_write(cfgp->open_file, buf, nbyte);
	if( ret != nbyte ){
		//mcu_debug("Only wrote %d bytes\n", ret);
		return -1;
	}
	memset(buffer, 0, nbyte);
	cfgp->open_file->loc = loc;
	u_read(cfgp->open_file, buffer, nbyte);
	if ( memcmp(buffer, buf, nbyte) != 0 ){
		return -1;
	}

	return ret;
}

int sffs_dev_read(const void * cfg, int loc, void * buf, int nbyte){
	const sffs_cfg_t * cfgp = cfg;
	if ( cfgp->open_file->fs == NULL ){
		errno = EIO;
		return -1;
	}
	cfgp->open_file->loc = loc;
	return u_read(cfgp->open_file, buf, nbyte);
}


int sffs_dev_erase(const void * cfg){
	const sffs_cfg_t * cfgp = cfg;
	if ( cfgp->open_file->fs == NULL ){
		errno = EIO;
		return -1;
	}
	int usec;
	if( u_ioctl(cfgp->open_file, I_DRIVE_ERASEDEVICE, NULL) < 0 ){
		return -1;
	}

	usec = cfgp->state->dattr.erase_device_time;

	usleep(usec);
	return 0;
}

int sffs_dev_erasesection(const void * cfg, int loc){
	const sffs_cfg_t * cfgp;
	int usec;
	cfgp = cfg;
	if ( cfgp->open_file->fs == NULL ){
		errno = EIO;
		return -1;
	}

	if( u_ioctl(cfgp->open_file, I_DRIVE_ERASEBLOCK, (void*)loc) < 0 ){
		return -1;
	}

	usec = cfgp->state->dattr.erase_block_time;

	usleep(usec);
	return 0;
}

int sffs_dev_close(const void * cfg){
	const sffs_cfg_t * cfgp = cfg;
	return cfgp->devfs->close(
			cfgp->devfs->cfg,
			&(cfgp->open_file->handle));
}


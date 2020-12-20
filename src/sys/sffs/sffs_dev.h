// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md



#ifndef SFFS_DEV_H_
#define SFFS_DEV_H_

#include <pthread.h>

#include "sos/fs/sffs.h"

/*
 * The dev is the device that is used to store the data.
 *
 * The device must be implemented as a sos/dev/drive.h device.
 *
 * This module has the functions needed to read/write/ioctl
 * the device.
 *
 *
 *
 *
 */

#define SFFS_CONFIG(cfg) ((const sffs_config_t*)cfg)
#define SFFS_STATE(cfg) ((sffs_state_t*)(((const sffs_config_t*)cfg)->drive.state))
#define SFFS_DRIVE(cfg) &(((const sffs_config_t*)cfg)->drive)
#define SFFS_DRIVE_MUTEX(cfg) &(((const sffs_config_t*)cfg)->drive.state->mutex)

int sffs_dev_open(const void * cfg);
int sffs_dev_write(const void * cfg, int loc, const void * buf, int nbyte);
int sffs_dev_read(const void * cfg, int loc, void * buf, int nbyte);
int sffs_dev_close(const void * cfg);

static inline int sffs_dev_getsize(const void * cfg){
	return SFFS_STATE(cfg)->dattr.num_write_blocks * SFFS_STATE(cfg)->dattr.write_block_size;
}

static inline int sffs_dev_geterasesize(const void * cfg){
	return SFFS_STATE(cfg)->dattr.erase_block_size;
}

static inline int sffs_dev_getblocksize(const void * cfg){
	return SFFS_STATE(cfg)->dattr.write_block_size;
}

int sffs_dev_erase(const void * cfg);
int sffs_dev_erasesection(const void * cfg, int loc);

int sysfs_access(int file_mode, int file_uid, int file_gid, int amode);
int sffs_sys_geteuid();
int sffs_sys_getegid();

int sffs_dev_getlist_block(const void * cfg);
void sffs_dev_setlist_block(const void * cfg, int list_block);
int sffs_dev_getserialno(const void * cfg);
void sffs_dev_setserialno(const void * cfg, int serialno);

void sffs_dev_setdelay_mutex(pthread_mutex_t * mutex);

#endif /* SFFS_DEV_H_ */

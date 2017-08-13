/*
 * devfs_local.h
 *
 *  Created on: Aug 12, 2017
 *      Author: tgil
 */

#ifndef SYSFS_DEVFS_LOCAL_H_
#define SYSFS_DEVFS_LOCAL_H_


#include "sos/fs/sysfs.h"
#include "sos/fs/devfs.h"

int devfs_data_transfer(const void * config, const devfs_device_t * device, int flags, int loc, void * buf, int nbyte, int is_read);


#endif /* SYSFS_DEVFS_LOCAL_H_ */

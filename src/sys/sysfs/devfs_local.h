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
#include "aio.h"
#include "../sched/sched_local.h"
#include "../signal/sig_local.h"


int devfs_data_transfer(const void * config, const devfs_device_t * device, int flags, int loc, void * buf, int nbyte, int is_read);
int devfs_aio_data_transfer(const devfs_device_t * device, struct aiocb * aiocbp);

#endif /* SYSFS_DEVFS_LOCAL_H_ */

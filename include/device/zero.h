// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup SYS_NULL Null Device
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details This device discards input and returns EOF when read.
 *
 *
 */

/*! \file  */

#ifndef DEV_ZERO_H_
#define DEV_ZERO_H_

#include "sos/fs/devfs.h"


int zero_open(const devfs_handle_t * handle);
int zero_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int zero_read(const devfs_handle_t * handle, devfs_async_t * rop);
int zero_write(const devfs_handle_t * handle, devfs_async_t * wop);
int zero_close(const devfs_handle_t * handle);


#endif /* DEV_ZERO_H_ */


/*! @} */

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

/*! \addtogroup ENET Ethernet
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details
 *
 */

/*! \file
 * \brief Ethernet Header File
 *
 */

#ifndef ENET_H_
#define ENET_H_

#include "sos/dev/enet.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

int mcu_enet_open(const devfs_handle_t * cfg) MCU_PRIV_CODE;
int mcu_enet_read(const devfs_handle_t * cfg, devfs_async_t * rop) MCU_PRIV_CODE;
int mcu_enet_write(const devfs_handle_t * cfg, devfs_async_t * wop) MCU_PRIV_CODE;
int mcu_enet_ioctl(const devfs_handle_t * cfg, int request, void * ctl) MCU_PRIV_CODE;

int mcu_enet_close(const devfs_handle_t * cfg) MCU_PRIV_CODE;
int mcu_enet_getinfo(int port, void * ctl) MCU_PRIV_CODE;
int mcu_enet_setattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_enet_setaction(int port, void * ctl) MCU_PRIV_CODE;
int mcu_enet_inittxpkt(int port, void * ctl) MCU_PRIV_CODE;

#ifdef __cplusplus
}
#endif


#endif /* ENET_H_ */

/*! @} */

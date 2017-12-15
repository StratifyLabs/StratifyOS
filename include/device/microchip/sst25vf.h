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

/*! \addtogroup SST25VF SST25VF Serial Flash Storage
 *
 * @{
 *
 */

#ifndef DEV_MICROCHIP_SST25VF_H_
#define DEV_MICROCHIP_SST25VF_H_

#include "sos/dev/drive.h"
#include "mcu/spi.h"

typedef struct {
	spi_config_t spi;
	mcu_pin_t cs;
	mcu_pin_t hold /*! Hold Pin */;
	mcu_pin_t wp /*! Write protect pin */;
	u32 size /*! The size of the memory on the device */;
} sst25vf_config_t;

typedef struct {
	const char * buf;
	int nbyte;
	u8 cmd[8];
	devfs_async_t op;
	mcu_event_handler_t handler;
	int prot;
} sst25vf_state_t;

int sst25vf_open(const devfs_handle_t * cfg);
int sst25vf_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int sst25vf_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int sst25vf_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int sst25vf_close(const devfs_handle_t * cfg);

int sst25vf_tmr_open(const devfs_handle_t * cfg);
int sst25vf_tmr_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int sst25vf_tmr_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int sst25vf_tmr_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int sst25vf_tmr_close(const devfs_handle_t * cfg);

int sst25vf_ssp_open(const devfs_handle_t * cfg);
int sst25vf_ssp_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int sst25vf_ssp_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int sst25vf_ssp_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int sst25vf_ssp_close(const devfs_handle_t * cfg);

int sst25vf_ssp_tmr_open(const devfs_handle_t * cfg);
int sst25vf_ssp_tmr_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int sst25vf_ssp_tmr_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int sst25vf_ssp_tmr_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int sst25vf_ssp_tmr_close(const devfs_handle_t * cfg);



#endif /* DEV_MICROCHIP_SST25VF_H_ */

/*! @} */

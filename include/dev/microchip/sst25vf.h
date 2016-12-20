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

#include "mcu/mcu.h"
#include "iface/dev/microchip/sst25vf.h"

typedef struct {
	const char * buf;
	int nbyte;
	u8 cmd[8];
	device_transfer_t op;
	mcu_callback_t callback;
	void * context;
	int prot;
} sst25vf_state_t;

int sst25vf_open(const device_cfg_t * cfg);
int sst25vf_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int sst25vf_read(const device_cfg_t * cfg, device_transfer_t * rop);
int sst25vf_write(const device_cfg_t * cfg, device_transfer_t * wop);
int sst25vf_close(const device_cfg_t * cfg);

int sst25vf_tmr_open(const device_cfg_t * cfg);
int sst25vf_tmr_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int sst25vf_tmr_read(const device_cfg_t * cfg, device_transfer_t * rop);
int sst25vf_tmr_write(const device_cfg_t * cfg, device_transfer_t * wop);
int sst25vf_tmr_close(const device_cfg_t * cfg);

int sst25vf_ssp_open(const device_cfg_t * cfg);
int sst25vf_ssp_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int sst25vf_ssp_read(const device_cfg_t * cfg, device_transfer_t * rop);
int sst25vf_ssp_write(const device_cfg_t * cfg, device_transfer_t * wop);
int sst25vf_ssp_close(const device_cfg_t * cfg);


#endif /* DEV_MICROCHIP_SST25VF_H_ */

/*! @} */

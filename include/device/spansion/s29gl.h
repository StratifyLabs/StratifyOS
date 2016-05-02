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

#ifndef DEVICE_SPANSION_S29GL_H_
#define DEVICE_SPANSION_S29GL_H_

#include "hwdl.h"
#include "iface/dev/spansion/s29gl.h"

typedef struct {
	const char * buf;
	int nbyte;
	uint8_t cmd[8];
	device_transfer_t op;
	mcu_callback_t callback;
	void * context;
	int prot;
} s29gl_state_t;

int s29gl_open(const device_cfg_t * cfg);
int s29gl_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int s29gl_read(const device_cfg_t * cfg, device_transfer_t * rop);
int s29gl_write(const device_cfg_t * cfg, device_transfer_t * wop);
int s29gl_close(const device_cfg_t * cfg);

int s29gl_tmr_open(const device_cfg_t * cfg);
int s29gl_tmr_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int s29gl_tmr_read(const device_cfg_t * cfg, device_transfer_t * rop);
int s29gl_tmr_write(const device_cfg_t * cfg, device_transfer_t * wop);
int s29gl_tmr_close(const device_cfg_t * cfg);


#endif /* DEVICE_SPANSION_S29GL_H_ */

/*! @} */

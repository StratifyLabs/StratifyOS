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


#ifndef DEV_SYS_H_
#define DEV_SYS_H_

#include "iface/device_config.h"
#include "iface/dev/sys.h"
#include "mcu/task.h"
#include "mcu/types.h"


int led_open(const device_cfg_t * cfg);
int led_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int led_read(const device_cfg_t * cfg, device_transfer_t * rop);
int led_write(const device_cfg_t * cfg, device_transfer_t * wop);
int led_close(const device_cfg_t * cfg);

int null_open(const device_cfg_t * cfg);
int null_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int null_read(const device_cfg_t * cfg, device_transfer_t * rop);
int null_write(const device_cfg_t * cfg, device_transfer_t * wop);
int null_close(const device_cfg_t * cfg);

int zero_open(const device_cfg_t * cfg);
int zero_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int zero_read(const device_cfg_t * cfg, device_transfer_t * rop);
int zero_write(const device_cfg_t * cfg, device_transfer_t * wop);
int zero_close(const device_cfg_t * cfg);

int full_open(const device_cfg_t * cfg);
int full_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int full_read(const device_cfg_t * cfg, device_transfer_t * rop);
int full_write(const device_cfg_t * cfg, device_transfer_t * wop);
int full_close(const device_cfg_t * cfg);

int random_open(const device_cfg_t * cfg);
int random_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int random_read(const device_cfg_t * cfg, device_transfer_t * rop);
int random_write(const device_cfg_t * cfg, device_transfer_t * wop);
int random_close(const device_cfg_t * cfg);

int urandom_open(const device_cfg_t * cfg);
int urandom_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int urandom_read(const device_cfg_t * cfg, device_transfer_t * rop);
int urandom_write(const device_cfg_t * cfg, device_transfer_t * wop);
int urandom_close(const device_cfg_t * cfg);

int sys_open(const device_cfg_t * cfg);
int sys_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int sys_read(const device_cfg_t * cfg, device_transfer_t * rop);
int sys_write(const device_cfg_t * cfg, device_transfer_t * wop);
int sys_close(const device_cfg_t * cfg);

int sys_23_open(const device_cfg_t * cfg);
int sys_23_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int sys_23_read(const device_cfg_t * cfg, device_transfer_t * rop);
int sys_23_write(const device_cfg_t * cfg, device_transfer_t * wop);
int sys_23_close(const device_cfg_t * cfg);

typedef struct {
	const char * buf;
	int * nbyte;
	int count;
	int timeout;
	uint8_t cmd[16];
	device_transfer_t op;
	mcu_callback_t callback;
	void * context;
	u32 flags;
} sdspi_state_t;

typedef struct {
	device_gpio_t hold /*! \brief Hold Pin */;
	device_gpio_t wp /*! \brief Write protect pin */;
	device_gpio_t miso /*! \brief The write complete pin */;
	uint32_t size /*! \brief The size of the memory on the device */;
} sdspi_cfg_t;



int sdspi_open(const device_cfg_t * cfg);
int sdspi_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int sdspi_read(const device_cfg_t * cfg, device_transfer_t * rop);
int sdspi_write(const device_cfg_t * cfg, device_transfer_t * wop);
int sdspi_close(const device_cfg_t * cfg);

int sdssp_open(const device_cfg_t * cfg);
int sdssp_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int sdssp_read(const device_cfg_t * cfg, device_transfer_t * rop);
int sdssp_write(const device_cfg_t * cfg, device_transfer_t * wop);
int sdssp_close(const device_cfg_t * cfg);



extern uint8_t sys_euid;

#define SYS_USER 0
#define SYS_ROOT 1

#define sys_isroot() ( task_root_asserted( task_get_current() ) )
#define sys_setuser() ( task_deassert_root( task_get_current() ) )
#define sys_setroot() ( task_assert_root( task_get_current() ) )



#endif /* DEV_SYS_H_ */


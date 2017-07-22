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

#include "sos/dev/sys.h"
#include "sos/dev/spi.h"

#include "sos/fs/devfs.h"
#include "mcu/task.h"


int sys_open(const devfs_handle_t * handle);
int sys_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int sys_read(const devfs_handle_t * handle, devfs_async_t * rop);
int sys_write(const devfs_handle_t * handle, devfs_async_t * wop);
int sys_close(const devfs_handle_t * handle);

int sys_23_open(const devfs_handle_t * handle);
int sys_23_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int sys_23_read(const devfs_handle_t * handle, devfs_async_t * rop);
int sys_23_write(const devfs_handle_t * handle, devfs_async_t * wop);
int sys_23_close(const devfs_handle_t * handle);

typedef struct {
	const char * buf;
	int * nbyte;
	int count;
	int timeout;
	uint8_t cmd[16];
	devfs_async_t op;
	mcu_event_handler_t handler;
	u32 flags;
} sdspi_state_t;

typedef struct {
	spi_attr_t attr;
	mcu_pin_t cs;
	mcu_pin_t hold /*! \brief Hold Pin */;
	mcu_pin_t wp /*! \brief Write protect pin */;
	mcu_pin_t miso /*! \brief The write complete pin */;
	uint32_t size /*! \brief The size of the memory on the device */;
} sdspi_config_t;






extern uint8_t sys_euid;

#define SYS_USER 0
#define SYS_ROOT 1

#define sys_isroot() ( task_root_asserted( task_get_current() ) )
#define sys_setuser() ( task_deassert_root( task_get_current() ) )
#define sys_setroot() ( task_assert_root( task_get_current() ) )


#define SYS_DEVICE { \
		.name = "sys", \
		DEVFS_MODE(0666, 0, 0, S_IFCHR), \
		DEVFS_DRIVER(sys), \
		.cfg.port = 0 \
}

#define SYS_23_DEVICE { \
		.name = "sys", \
		DEVFS_MODE(0666, 0, 0, S_IFCHR), \
		DEVFS_DRIVER(sys_23), \
		.cfg.port = 0 \
}


#define NULL_DEVICE { \
		.name = "null", \
		DEVFS_MODE(0666, 0, 0, S_IFCHR), \
		DEVFS_DRIVER(null), \
		.cfg.port = 0 \
}

#define FULL_DEVICE { \
		.name = "full", \
		DEVFS_MODE(0666, 0, 0, S_IFCHR), \
		DEVFS_DRIVER(full), \
		.cfg.port = 0 \
}


typedef struct MCU_PACK {
	uint16_t r2;
} sdspi_status_t;


#define I_SDSPI_STATUS _IOCTLW('S', 2, sdspi_status_t)



#endif /* DEV_SYS_H_ */


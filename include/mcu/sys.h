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
#include "mcu/types.h"


int led_open(const devfs_handle_t * cfg);
int led_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int led_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int led_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int led_close(const devfs_handle_t * cfg);

int null_open(const devfs_handle_t * cfg);
int null_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int null_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int null_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int null_close(const devfs_handle_t * cfg);

int zero_open(const devfs_handle_t * cfg);
int zero_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int zero_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int zero_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int zero_close(const devfs_handle_t * cfg);

int full_open(const devfs_handle_t * cfg);
int full_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int full_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int full_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int full_close(const devfs_handle_t * cfg);

int random_open(const devfs_handle_t * cfg);
int random_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int random_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int random_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int random_close(const devfs_handle_t * cfg);

int urandom_open(const devfs_handle_t * cfg);
int urandom_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int urandom_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int urandom_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int urandom_close(const devfs_handle_t * cfg);

int sys_open(const devfs_handle_t * cfg);
int sys_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int sys_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int sys_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int sys_close(const devfs_handle_t * cfg);

int sys_23_open(const devfs_handle_t * cfg);
int sys_23_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int sys_23_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int sys_23_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int sys_23_close(const devfs_handle_t * cfg);

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
} sdspi_cfg_t;



int sdspi_open(const devfs_handle_t * cfg);
int sdspi_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int sdspi_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int sdspi_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int sdspi_close(const devfs_handle_t * cfg);

int sdssp_open(const devfs_handle_t * cfg);
int sdssp_ioctl(const devfs_handle_t * cfg, int request, void * ctl);
int sdssp_read(const devfs_handle_t * cfg, devfs_async_t * rop);
int sdssp_write(const devfs_handle_t * cfg, devfs_async_t * wop);
int sdssp_close(const devfs_handle_t * cfg);



extern uint8_t sys_euid;

#define SYS_USER 0
#define SYS_ROOT 1

#define sys_isroot() ( task_root_asserted( task_get_current() ) )
#define sys_setuser() ( task_deassert_root( task_get_current() ) )
#define sys_setroot() ( task_assert_root( task_get_current() ) )


#define SYS_DEVICE { \
		.name = "sys", \
		DEVICE_MODE(0666, 0, 0, S_IFCHR), \
		DEVICE_DRIVER(sys), \
		.cfg.port = 0 \
}

#define SYS_23_DEVICE { \
		.name = "sys", \
		DEVICE_MODE(0666, 0, 0, S_IFCHR), \
		DEVICE_DRIVER(sys_23), \
		.cfg.port = 0 \
}


#define NULL_DEVICE { \
		.name = "null", \
		DEVICE_MODE(0666, 0, 0, S_IFCHR), \
		DEVICE_DRIVER(null), \
		.cfg.port = 0 \
}


#define ZERO_DEVICE { \
		.name = "zero", \
		DEVICE_MODE(0666, 0, 0, S_IFCHR), \
		DEVICE_DRIVER(zero), \
		.cfg.port = 0 \
}


#define FULL_DEVICE { \
		.name = "full", \
		DEVICE_MODE(0666, 0, 0, S_IFCHR), \
		DEVICE_DRIVER(full), \
		.cfg.port = 0 \
}

#define LED_DEVICE(dev_name, active_level, pio_port0, pio_pin0, pio_port1, pio_pin1, pio_port2, pio_pin2, pio_port3, pio_pin3) { \
		.name = dev_name, \
		DEVICE_MODE(0666, 0, 0, S_IFCHR), \
		DEVICE_DRIVER(led), \
		.cfg.port = 0, \
		.cfg.pin_assign = active_level, \
		.cfg.pcfg.pio[0].port = pio_port0, \
		.cfg.pcfg.pio[0].pin = pio_pin0, \
		.cfg.pcfg.pio[1].port = pio_port1, \
		.cfg.pcfg.pio[1].pin = pio_pin1, \
		.cfg.pcfg.pio[2].port = pio_port2, \
		.cfg.pcfg.pio[2].pin = pio_pin2, \
		.cfg.pcfg.pio[3].port = pio_port3, \
		.cfg.pcfg.pio[3].pin = pio_pin3, \
}

#define LED_DEVICE_UNUSED (-1)

#define RANDOM_DEVICE { \
		.name = "random", \
		DEVICE_MODE(0666, 0, 0, S_IFCHR), \
		DEVICE_DRIVER(random), \
		.cfg.port = 0 \
}

#define URANDOM_DEVICE { \
		.name = "urandom", \
		DEVICE_MODE(0666, 0, 0, S_IFCHR), \
		DEVICE_DRIVER(urandom), \
		.cfg.port = 0 \
}

#define SDSPI_DEVICE(device_name, port_number, pin_assign_value, cs_port_value, cs_pin_value, bitrate_value, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFBLK), \
		DEVICE_DRIVER(sdspi), \
		.handle.port = port_number, \
		.handle.state = state_ptr, \
		.handle.config = cfg_ptr \
}

#define SDSSP_DEVICE(device_name, port_number, pin_assign_value, cs_port_value, cs_pin_value, bitrate_value, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFBLK), \
		DEVICE_DRIVER(sdssp), \
		.handle.port = port_number, \
		.handle.state = state_ptr, \
		.handle.config = cfg_ptr \
}

typedef struct MCU_PACK {
	uint16_t r2;
} sdspi_status_t;

#define SDSPI_DEVICE_CFG(cs_port, cs_pin, hold_port, hold_pin, wp_port, wp_pin, miso_port, miso_pin, device_size, bitrate) { \
		.attr.o_flags = (SPI_FLAG_MASTER | SPI_FLAG_MODE0 | SPI_FLAG_FORMAT_SPI), \
		.attr.freq = bitrate, \
		.cs.port = cs_port, \
		.cs.pin = cs_pin, \
		.hold.port = hold_port, \
		.hold.pin = hold_pin, \
		.wp.port = wp_port, \
		.wp.pin = wp_pin, \
		.miso.port = miso_port, \
		.miso.pin = miso_pin, \
		.size = (device_size) \
}


#define I_SDSPI_STATUS _IOCTLW('S', 2, sdspi_status_t)


#define DEVFIFO_DEVICE(device_name, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFCHR), \
		DEVICE_DRIVER(devfifo), \
		.cfg.state = state_ptr, \
		.cfg.dcfg = cfg_ptr \
}


#define DEVFIFO_DEVICE_CFG(target_device, target_buffer, buffer_size, getbyte, setaction, callback_event) { \
	.dev = target_device, \
	.buffer = target_buffer, \
	.size = buffer_size, \
	.req_getbyte = getbyte, \
	.req_setaction = setaction, \
	.event = callback_event \
}


#endif /* DEV_SYS_H_ */


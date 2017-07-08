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

#include "mcu/types.h"
#include "sos/dev/microchip/sst25vf.h"

typedef struct {
	spi_attr_t attr;
	mcu_pin_t cs;
	mcu_pin_t hold /*! Hold Pin */;
	mcu_pin_t wp /*! Write protect pin */;
	mcu_pin_t miso /*! The write complete pin */;
	u32 size /*! The size of the memory on the device */;
} sst25vf_cfg_t;

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


#define SST25VF_DEVICE(device_name, port_number, pin_assign_value, cs_port_value, cs_pin_value, bitrate_value, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFBLK), \
		DEVICE_DRIVER(sst25vf), \
		.handle.port = port_number, \
		.handle.state = state_ptr, \
		.handle.config = cfg_ptr \
}

#define SST25VF_SSP_DEVICE(device_name, port_number, pin_assign_value, cs_port_value, cs_pin_value, bitrate_value, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFBLK), \
		DEVICE_DRIVER(sst25vf_ssp), \
		.handle.port = port_number, \
		.handle.state = state_ptr, \
		.handle.config = cfg_ptr \
}

/*
 * 		.cfg.pin_assign = pin_assign_value, \
		.cfg.pcfg.spi.mode = SPI_ATTR_MODE0, \
		.cfg.pcfg.spi.cs.port = cs_port_value, \
		.cfg.pcfg.spi.cs.pin = cs_pin_value, \
		.cfg.pcfg.spi.width = 8, \
		.cfg.pcfg.spi.format = SPI_ATTR_FORMAT_SPI, \
		.cfg.bitrate = bitrate_value, \
 */


#define SST25VF_TMR_DEVICE(device_name, port_number, pin_assign_value, cs_port_value, cs_pin_value, bitrate_value, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFBLK), \
		DEVICE_DRIVER(sst25vf_tmr), \
		.handle.port = port_number, \
		.handle.state = state_ptr, \
		.handle.config = cfg_ptr \
}

#define SST25VF_DEVICE_CFG(cs_port, cs_pin, hold_port, hold_pin, wp_port, wp_pin, miso_port, miso_pin, device_size, bitrate) { \
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



#endif /* DEV_MICROCHIP_SST25VF_H_ */

/*! @} */

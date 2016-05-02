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

/*! \ingroup MCP4725
 * @{
 */

/*! \file  */

#include <errno.h>
#include "mcu/core.h"
#include "mcu/debug.h"
#include "mcu/i2c.h"
#include "iface/dev/spansion/s29gl.h"

#define S29GL_BLOCK_ERASE_SIZE (64*1024)
#define S29GL_CHIP_ERASE_TIME 32000000
#define S29GL_BLOCK_ERASE_TIME 500000

static void s29gl_disk_protect(const device_cfg_t * cfg);
static void s29gl_disk_unprotect(const device_cfg_t * cfg);
static void s29gl_disk_erase_block(const device_cfg_t * cfg, int block);
static void s29gl_disk_erase_device(const device_cfg_t * cfg);
static void s29gl_disk_power_down(const device_cfg_t * cfg);
static void s29gl_disk_power_up(const device_cfg_t * cfg);



int s29gl_open(const device_cfg_t * cfg){
	return 0;
}


int s29gl_ioctl(const device_cfg_t * cfg, int request, void * ctl){
	const s29gl_cfg_t * cfgp = (const s29gl_cfg_t *)(cfg->dcfg);
	switch(request){
	case I_DISK_PROTECT:
		s29gl_disk_protect(cfg);
		break;
	case I_DISK_UNPROTECT:
		s29gl_disk_unprotect(cfg);
		break;
	case I_DISK_ERASE_BLOCK:
		s29gl_disk_erase_block(cfg, (ssize_t)ctl);
		break;
	case I_DISK_ERASE_DEVICE:
		s29gl_disk_erase_device(cfg);
		break;
	case I_DISK_POWER_DOWN:
		s29gl_disk_power_down(cfg);
		break;
	case I_DISK_POWER_UP:
		s29gl_disk_power_up(cfg);
		break;
	case I_DISK_GET_BLOCKSIZE:
		return S29GL_BLOCK_ERASE_SIZE;
	case I_DISK_GET_DEVICE_ERASETIME:
		return S29GL_CHIP_ERASE_TIME;
	case I_DISK_GET_BLOCK_ERASETIME:
		return S29GL_BLOCK_ERASE_TIME;
	case I_DISK_GET_SIZE:
		return cfgp->size;
	default:
		errno = EINVAL;
		return -1;
	}
	return 0;
}

int s29gl_read(const device_cfg_t * cfg, device_transfer_t * rop){
	uint32_t addr;
	const s29gl_cfg_t * cfgp = (const s29gl_cfg_t *)(cfg->dcfg);
	//simply memcpy
	addr = cfgp->addr_mask + (rop->loc << 1);
	memcpy(rop->buf, (void*)addr, rop->nbyte);
	return rop->nbyte;
}

int s29gl_write(const device_cfg_t * cfg, device_transfer_t * wop){
	const s29gl_cfg_t * cfgp = (const s29gl_cfg_t *)(cfg->dcfg);
	uint32_t mask = cfgp->addr_mask;
	uint16_t * p;
	const uint16_t * d;
	int loc = (wop->loc);
	int i;
	d = (const uint16_t*)wop->cbuf;
	uint16_t tmp0, tmp1;
	int timeout;

	_mcu_core_priv_enable_interrupts(0);

	p = (uint16_t*)(mask | (0x555 << 1));
	*p = 0xAA;
	p = (uint16_t*)(mask | (0x2AA << 1));
	*p = 0x55;
	p = (uint16_t*)(mask | (0x555 << 1));
	*p = 0x90;
	p = (uint16_t*)(mask | (0x00 << 1));
	tmp0 = *p;


	for(i=0; i < wop->nbyte; i+=2){
		p = (uint16_t*)(mask | (0x555 << 1));
		*p = 0xAA;
		p = (uint16_t*)(mask | (0x2AA << 1));
		*p = 0x55;
		p = (uint16_t*)(mask | (0x555 << 1));
		*p = 0xA0;
		p = (uint16_t*)(mask | (loc << 1));
		*p = d[i];
		loc += 2;


		timeout = 0;
		//now wait for write to complete
		do {
			tmp0 = *p;
			tmp1 = *p;
			timeout++;
			if( timeout == 5000 ){
				errno = EIO;
				return -1;
			}
		} while( (tmp0 != tmp1) || (tmp1 != d[i]) );
	}

	return wop->nbyte;
}

int s29gl_close(const device_cfg_t * cfg){
	return 0;
}

static void s29gl_disk_protect(const device_cfg_t * cfg){

}

static void s29gl_disk_unprotect(const device_cfg_t * cfg){

}

static void s29gl_disk_erase_block(const device_cfg_t * cfg, int block){
	const s29gl_cfg_t * cfgp = (const s29gl_cfg_t *)(cfg->dcfg);
	uint32_t mask = cfgp->addr_mask;
	uint16_t * p;
	uint32_t sector_addr = (block * S29GL_BLOCK_ERASE_SIZE) >> 16;

	p = (uint16_t*)(mask | 0x555);
	*p = 0xAA;
	p = (uint16_t*)(mask | 0x2AA);
	*p = 0x55;
	p = (uint16_t*)(mask | 0x555);
	*p = 0x80;
	p = (uint16_t*)(mask | 0x555);
	*p = 0xAA;
	p = (uint16_t*)(mask | 0x2AA);
	*p = 0x55;
	p = (uint16_t*)(mask | sector_addr);
	*p = 0x30;
}

static void s29gl_disk_erase_device(const device_cfg_t * cfg){
	const s29gl_cfg_t * cfgp = (const s29gl_cfg_t *)(cfg->dcfg);
	uint32_t mask = cfgp->addr_mask;
	uint16_t * p;

	p = (uint16_t*)(mask | 0x555);
	*p = 0xAA;
	p = (uint16_t*)(mask | 0x2AA);
	*p = 0x55;
	p = (uint16_t*)(mask | 0x555);
	*p = 0x80;
	p = (uint16_t*)(mask | 0x555);
	*p = 0xAA;
	p = (uint16_t*)(mask | 0x2AA);
	*p = 0x55;
	p = (uint16_t*)(mask | 0x555);
	*p = 0x10;
}

static void s29gl_disk_power_down(const device_cfg_t * cfg){

}

static void s29gl_disk_power_up(const device_cfg_t * cfg){

}



/*! @} */

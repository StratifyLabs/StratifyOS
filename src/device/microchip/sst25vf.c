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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */


#include <errno.h>
#include <unistd.h>
#include "mcu/pio.h"
#include "mcu/spi.h"
#include "mcu/debug.h"
#include "sst25vf_local.h"

static int complete_spi_write(const devfs_handle_t * cfg, uint32_t ignore);
static int continue_spi_write(const devfs_handle_t * cfg, uint32_t ignore);


int sst25vf_open(const devfs_handle_t * cfg){
	int err;
	uint8_t status;
	pio_attr_t pio_attr;
	sst25vf_state_t * state = (sst25vf_state_t*)cfg->state;
	const sst25vf_config_t * config = cfg->config;

	/*
	spi_attr_t spi_cfg;
	spi_cfg.pin_assign = cfg->pin_assign;
	spi_cfg.width = cfg->pcfg.spi.width;
	spi_cfg.mode = cfg->pcfg.spi.mode;
	spi_cfg.format = cfg->pcfg.spi.format;
	spi_cfg.bitrate = cfg->bitrate;
	spi_cfg.master = SPI_ATTR_MASTER;
	*/
	err = mcu_spi_open(cfg);
	if ( err < 0 ){
		return err;
	}

	if( (err = mcu_spi_ioctl(cfg, I_SPI_SETATTR, (void*)&(config->attr))) < 0 ){
		return err;
	}

	sst25vf_share_deassert_cs(cfg);
	pio_attr.o_pinmask = (1<<config->cs.pin);
	pio_attr.o_flags = PIO_FLAG_SET_OUTPUT | PIO_FLAG_IS_DIRONLY;
	mcu_pio_setattr(config->cs.port, &pio_attr);

	sst25vf_share_write_disable(cfg);

	//Now ping the device to see if it responds
	sst25vf_share_power_up(cfg);
	sst25vf_share_global_protect(cfg);
	status = sst25vf_share_read_status(cfg);

	if ( status != 0x9C ){
		//Global protect command failed
		errno = EIO;
		return -1;
	}


	if ( sst25vf_share_global_unprotect(cfg) ){
		errno = EIO;
		return -1;
	}
	status = sst25vf_share_read_status(cfg);

	if ( status != 0x80 ){
		//global unprotect failed
		errno = EIO;
		return -1;
	}

	state->prot = 0;

	//The device is ready to use
	return 0;
}

static int complete_spi_read(const devfs_handle_t * cfg, uint32_t ignore){
	sst25vf_state_t * state = (sst25vf_state_t*)cfg->state;
	sst25vf_share_deassert_cs(cfg);
	if( state->handler.callback != NULL ){
		state->handler.callback(state->handler.context, NULL);
		state->handler.callback = NULL;
	}
	return 0;
}


int sst25vf_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	sst25vf_state_t * state = (sst25vf_state_t*)cfg->state;
	const sst25vf_config_t * dcfg = (const sst25vf_config_t *)(cfg->config);
	state->handler.callback = rop->handler.callback;
	state->handler.context = rop->handler.context;
	rop->handler.context = (void*)cfg;
	rop->handler.callback = (mcu_callback_t)complete_spi_read;

	if ( rop->loc >= dcfg->size ){
		return EOF;
	}

	if ( rop->loc + rop->nbyte > dcfg->size ){
		rop->nbyte = dcfg->size - rop->loc; //update the bytes read to not go past the end of the disk
	}

	sst25vf_share_assert_cs(cfg);
	sst25vf_share_write_opcode_addr(cfg, SST25VF_INS_RD_HS, rop->loc);
	mcu_spi_swap(cfg->port, NULL); //dummy byte output
	return mcu_spi_read(cfg, rop);
}

static void assert_delay(){
	int loop_max;
	int tmp;

	if ( _mcu_core_getclock() <= 25000000 ){
		return;
	} else if ( _mcu_core_getclock() > 100000000 ){
		loop_max = 75;
	} else if ( _mcu_core_getclock() > 50000000 ){
		loop_max = 60;
	} else if ( _mcu_core_getclock() > 25000000 ){
		loop_max = 30;
	}

	//Must be high for either 100ns (25MHz model) or 50ns (50MHz and 80MHz) models
	//This loop is 4 instructions --
	for(tmp = 0; tmp < loop_max; tmp++){
		asm volatile("nop\n");
	}
}

int continue_spi_write(const devfs_handle_t * cfg, uint32_t ignore){
	sst25vf_state_t * state = (sst25vf_state_t *)cfg->state;
	int tmp;
	//should be called 10 us after complete_spi_write() executes

	sst25vf_share_deassert_cs(cfg);

	if( state->nbyte > 0 ){
		state->cmd[0] = SST25VF_INS_SEQ_PROGRAM;
		state->cmd[1] = state->buf[0];
		if ( state->nbyte > 1 ){
			state->cmd[2] = state->buf[1];
			tmp = 2;
		} else {
			state->cmd[2] = 0xFF;
			tmp = 1;
		}

		assert_delay();
		sst25vf_share_assert_cs(cfg);
		state->op.nbyte = 3;
		mcu_spi_write(cfg, &(state->op));
		state->buf += tmp;
		state->nbyte -= tmp;
	} else {

		sst25vf_share_write_disable(cfg);
		sst25vf_share_write_dbsy(cfg);

		sst25vf_share_read_status(cfg);

		//Set the buffer to NULL to indicate the device is not busy
		state->buf = NULL;

		//call the event handler to show the operation is complete
		if ( state->handler.callback != NULL ){
			state->handler.callback(state->handler.context, NULL);
			state->handler.callback = NULL;
		}
	}
	return 0;
}

int complete_spi_write(const devfs_handle_t * cfg, uint32_t ignore){
	mcu_action_t action;
	sst25vf_config_t * sst_cfg = (sst25vf_config_t*)cfg->config;
	sst25vf_state_t * state = (sst25vf_state_t*)cfg->state;


	//configure the GPIO to interrupt on a rising edge
	action.handler.context = (void*)cfg;
	action.handler.callback = (mcu_callback_t)continue_spi_write;
	action.channel = sst_cfg->miso.pin;
	action.o_events = MCU_EVENT_FLAG_RISING;
	action.prio = 0;
	mcu_pio_setaction(sst_cfg->miso.port, &action);

	sst25vf_share_deassert_cs(cfg);
	assert_delay();
	sst25vf_share_assert_cs(cfg);
	//continue_spi_write() will be called when the SPI flash is done writing and the GPIO is triggered
	if( state->nbyte > 0 ){
		return 1;
	} else {
		return 0;
	}
}

int sst25vf_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	int tmp;
	int err;
	uint8_t *addrp;
	sst25vf_state_t * state = (sst25vf_state_t*)cfg->state;
	//sst25vf_config_t * sst_cfg = (sst25vf_config_t*)cfg->config;

	if( state->prot == 1 ){
		errno = EROFS;
		return -1;
	}

	if ( state->buf != NULL ){
		errno = EBUSY;
		return -1;
	}

	//This is the final callback and context when all the writing is done
	state->handler = wop->handler;
	state->buf = wop->buf;
	state->nbyte = wop->nbyte;

	sst25vf_share_write_ebsy(cfg);
	sst25vf_share_write_enable(cfg);

	//set Auto increment
	addrp = (uint8_t*)&(wop->loc);
	state->cmd[0] = SST25VF_INS_SEQ_PROGRAM;
	state->cmd[1] = addrp[2];
	state->cmd[2] = addrp[1];
	state->cmd[3] = addrp[0];

	if ( wop->loc & 0x01 ){
		//starts on an odd address
		state->cmd[4] = 0xFF;
		state->cmd[5] = state->buf[0];
		tmp = 1;
	} else {
		state->cmd[4] = state->buf[0];
		if ( state->nbyte > 1 ){
			state->cmd[5] = state->buf[1];
			tmp = 2;
		} else {
			state->cmd[5] = 0xFF;
			tmp = 1;
		}
	}

	state->op.flags = wop->flags;
	state->op.handler.callback = (mcu_callback_t)complete_spi_write;
	state->op.handler.context = (void*)cfg;
	state->op.buf_const = state->cmd;
	state->op.nbyte = 6;
	state->op.loc = 0;

	sst25vf_share_assert_cs(cfg);
	err = mcu_spi_write(cfg, &state->op);
	state->buf = state->buf + tmp;
	state->nbyte = state->nbyte - tmp;

	return err;
}

int sst25vf_ioctl(const devfs_handle_t * cfg, int request, void * ctl){
	sst25vf_config_t * sst_cfg = (sst25vf_config_t*)cfg->config;
	sst25vf_state_t * state = (sst25vf_state_t*)cfg->state;
	drive_info_t * attr;

	switch(request){
	case I_DRIVE_ERASE_BLOCK:
	case I_DRIVE_ERASE_DEVICE:
		if( state->prot == 1 ){
			errno = EROFS;
			return -1;
		}
		break;
	}

	switch(request){
	case I_DRIVE_PROTECT:
		sst25vf_share_global_protect(cfg);
		state->prot = 1;
		break;
	case I_DRIVE_UNPROTECT:
		sst25vf_share_global_unprotect(cfg);
		state->prot = 0;
		break;
	case I_DRIVE_ERASE_BLOCK:
		sst25vf_share_block_erase_4kb(cfg, (ssize_t)ctl);
		break;
	case I_DRIVE_ERASE_DEVICE:
		sst25vf_share_chip_erase(cfg);
		break;
	case I_DRIVE_POWER_DOWN:
		sst25vf_share_power_down(cfg);
		break;
	case I_DRIVE_POWER_UP:
		sst25vf_share_power_up(cfg);
		break;
	case I_DRIVE_GETINFO:
		attr = ctl;

		attr->address_size = 1;
		attr->bitrate = 50000000;
		attr->erase_block_size = SST25VF_BLOCK_ERASE_SIZE;
		attr->erase_block_time = SST25VF_BLOCK_ERASE_TIME;
		attr->erase_device_time = SST25VF_CHIP_ERASE_TIME;
		attr->num_write_blocks = sst_cfg->size / SST25VF_BLOCK_SIZE;
		attr->write_block_size = SST25VF_BLOCK_SIZE;
		return 0;

	case I_DRIVE_GET_BLOCKSIZE:
		return SST25VF_BLOCK_ERASE_SIZE;
	case I_DRIVE_GET_DEVICE_ERASETIME:
		return SST25VF_CHIP_ERASE_TIME;
	case I_DRIVE_GET_BLOCK_ERASETIME:
		return SST25VF_BLOCK_ERASE_TIME;
	case I_DRIVE_GET_SIZE:
		return sst_cfg->size;
	default:
		return mcu_spi_ioctl(cfg, request, ctl);
	}
	return 0;
}

int sst25vf_close(const devfs_handle_t * cfg){
	return 0;
}



/*! @} */


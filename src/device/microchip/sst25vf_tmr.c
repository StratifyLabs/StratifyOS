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


#include <errno.h>
#include <unistd.h>
#include "mcu/pio.h"
#include "mcu/spi.h"
#include "mcu/tmr.h"
#include "mcu/debug.h"
#include "sst25vf_local.h"

static void complete_spi_write(const devfs_handle_t * cfg, uint32_t ignore);
static void continue_spi_write(const devfs_handle_t * cfg, uint32_t ignore);

int sst25vf_tmr_open(const devfs_handle_t * cfg){
	int err;
	u8 status;
	pio_attr_t attr;
	const sst25vf_cfg_t * config = cfg->config;

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
	attr.o_pinmask = (1<<config->cs.pin);
	attr.o_flags = PIO_FLAG_SET_OUTPUT | PIO_FLAG_IS_DIRONLY;
	mcu_pio_setattr(config->cs.port, &attr);

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

	//The device is ready to use
	return 0;
}

static void complete_spi_read(const devfs_handle_t * cfg, mcu_event_t ignore){
	sst25vf_state_t * state = (sst25vf_state_t*)cfg->state;
	sst25vf_share_deassert_cs(cfg);
	//if( state->handler.callback != NULL ){
	//state->handler.callback(state->handler.context, (mcu_event_t)NULL);
	mcu_execute_event_handler(&(state->handler), 0, 0);
	state->handler.callback = NULL;
	//}
}

int sst25vf_tmr_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	sst25vf_state_t * state = (sst25vf_state_t*)cfg->state;
	const sst25vf_cfg_t * dcfg = (const sst25vf_cfg_t *)(cfg->config);
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


void continue_spi_write(const devfs_handle_t * cfg, uint32_t ignore){
	sst25vf_state_t * state = (sst25vf_state_t *)cfg->state;
	const sst25vf_cfg_t * sst_cfg = (sst25vf_cfg_t*)cfg->config;
	mcu_action_t action;
	uint8_t * addrp;
	tmr_attr_t tmr_attr;
	//should be called 10 us after complete_spi_write() executes

	//Disable the TMR interrupt
	action.handler.callback = NULL;
	action.handler.context = NULL;
	action.o_events = TMR_EVENT_NONE;
	action.channel = sst_cfg->miso.pin;
	action.prio = 0;
	tmr_attr.o_flags = TMR_FLAG_DISABLE;
	mcu_tmr_setattr(sst_cfg->miso.port, &tmr_attr);
	mcu_tmr_setaction(sst_cfg->miso.port, &action);
	tmr_attr.o_flags = TMR_FLAG_ENABLE;
	mcu_tmr_setattr(sst_cfg->miso.port, &tmr_attr); //start the timer


	if( state->nbyte > 0 ){
		sst25vf_share_assert_cs(cfg);
		addrp = (uint8_t*)&(state->op.loc);
		state->cmd[0] = SST25VF_INS_PROGRAM;
		state->cmd[1] = addrp[2];
		state->cmd[2] = addrp[1];
		state->cmd[3] = addrp[0];
		state->cmd[4] = state->buf[0];

		state->op.nbyte = 5;
		mcu_spi_write(cfg, &(state->op));
		state->op.loc++;
		state->buf += 1;
		state->nbyte -= 1;
	} else {

		sst25vf_share_write_disable(cfg);

		sst25vf_share_read_status(cfg);

		//Set the buffer to NULL to indicate the device is not busy
		state->buf = NULL;

		//call the event handler to show the operation is complete
		if ( state->handler.callback != NULL ){
			state->handler.callback(state->handler.context, NULL);
			state->handler.callback = NULL;
		}
	}
}

void complete_spi_write(const devfs_handle_t * cfg, uint32_t ignore){
	uint32_t tval;
	mcu_action_t action;
	mcu_channel_t attr;
	sst25vf_cfg_t * sst_cfg = (sst25vf_cfg_t*)cfg->config;
	tmr_attr_t tmr_attr;

	sst25vf_share_deassert_cs(cfg);

	//configure the TMR to interrupt in 10 microseconds
	action.handler.context = (void*)cfg;
	action.handler.callback = (mcu_callback_t)continue_spi_write;
	action.channel = sst_cfg->miso.pin;
	action.o_events = TMR_EVENT_INTERRUPT;
	action.prio = 0;

	attr.channel = sst_cfg->miso.pin;

	//turn the timer off
	tmr_attr.o_flags = TMR_FLAG_DISABLE;
	mcu_tmr_setattr(sst_cfg->miso.port, &tmr_attr); //start the timer
	mcu_tmr_setaction(sst_cfg->miso.port, &action);
	tval = mcu_tmr_get(sst_cfg->miso.port, NULL);
	attr.value = tval + 20;
	if( attr.value > (1000000*2048) ){
		attr.value -= (1000000*2048);
	}
	mcu_tmr_setoc(sst_cfg->miso.port, &attr);

	//everything is set; turn the timer back on
	tmr_attr.o_flags = TMR_FLAG_ENABLE;
	mcu_tmr_setattr(sst_cfg->miso.port, &tmr_attr); //start the timer
}

int sst25vf_tmr_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	int err;
	uint8_t *addrp;
	sst25vf_state_t * state = (sst25vf_state_t*)cfg->state;

	if ( state->buf != NULL ){
		errno = EBUSY;
		return -1;
	}

	//This is the final callback and context when all the writing is done
	state->handler.callback = wop->handler.callback;
	state->handler.context = wop->handler.context;
	state->buf = wop->buf;
	state->nbyte = wop->nbyte;

	sst25vf_share_write_enable(cfg);

	//Byte program
	addrp = (uint8_t*)&(wop->loc);
	state->cmd[0] = SST25VF_INS_PROGRAM;
	state->cmd[1] = addrp[2];
	state->cmd[2] = addrp[1];
	state->cmd[3] = addrp[0];
	state->cmd[4] = state->buf[0];

	sst25vf_share_assert_cs(cfg);
	state->op.flags = wop->flags;
	state->op.handler.callback = (mcu_callback_t)complete_spi_write;
	state->op.handler.context = (void*)cfg;
	state->op.buf_const = state->cmd;
	state->op.nbyte = 5;
	state->op.loc = wop->loc;

	err = mcu_spi_write(cfg, &state->op);
	state->op.loc++;
	state->buf = state->buf + 1;
	state->nbyte = state->nbyte - 1;
	return err;
}

int sst25vf_tmr_ioctl(const devfs_handle_t * cfg, int request, void * ctl){
	sst25vf_cfg_t * sst_cfg = (sst25vf_cfg_t*)cfg->config;
	switch(request){
	case I_DRIVE_PROTECT:
		sst25vf_share_global_protect(cfg);
		break;
	case I_DRIVE_UNPROTECT:
		sst25vf_share_global_unprotect(cfg);
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

int sst25vf_tmr_close(const devfs_handle_t * cfg){
	sst25vf_share_power_down(cfg);
	return mcu_spi_close(cfg);
}



/*! @} */


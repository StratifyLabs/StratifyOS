/* Copyright 2011-2018 Tyler Gilbert; 
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
#include "sos/sos.h"
#include "mcu/pio.h"
#include "mcu/spi.h"
#include "mcu/tmr.h"
#include "mcu/debug.h"
#include "sst25vf_local.h"

static void complete_spi_write(const devfs_handle_t * handle, uint32_t ignore);
static void continue_spi_write(const devfs_handle_t * handle, uint32_t ignore);

int sst25vf_tmr_open(const devfs_handle_t * handle){
	int err;
	u8 status;
	pio_attr_t attr;
	const sst25vf_config_t * config = handle->config;
	devfs_handle_t pio_handle;
	pio_handle.port = config->cs.port;
	pio_handle.config = 0;

	err = mcu_spi_open(handle);
	if ( err < 0 ){
		return err;
	}

	if( (err = mcu_spi_setattr(handle, (void*)&(config->spi.attr))) < 0 ){
		return err;
	}

	sst25vf_share_deassert_cs(handle);
	attr.o_pinmask = (1<<config->cs.pin);
	attr.o_flags = PIO_FLAG_SET_OUTPUT | PIO_FLAG_IS_DIRONLY;
	mcu_pio_setattr(&pio_handle, &attr);

	sst25vf_share_write_disable(handle);

	//Now ping the device to see if it responds
	sst25vf_share_power_up(handle);
	sst25vf_share_global_protect(handle);
	status = sst25vf_share_read_status(handle);

	if ( status != 0x9C ){
		//Global protect command failed
        return SYSFS_SET_RETURN(EIO);
	}


	if ( sst25vf_share_global_unprotect(handle) ){
        return SYSFS_SET_RETURN(EIO);
	}
	status = sst25vf_share_read_status(handle);

	if ( status != 0x80 ){
		//global unprotect failed
        return SYSFS_SET_RETURN(EIO);
	}

	//The device is ready to use
	return 0;
}

static void complete_spi_read(const devfs_handle_t * handle, mcu_event_t ignore){
    sst25vf_state_t * state = (sst25vf_state_t*)handle->state;
    sst25vf_share_deassert_cs(handle);
	//if( state->handler.callback != NULL ){
	//state->handler.callback(state->handler.context, (mcu_event_t)NULL);
	mcu_execute_event_handler(&(state->handler), 0, 0);
	state->handler.callback = NULL;
	//}
}

int sst25vf_tmr_read(const devfs_handle_t * handle, devfs_async_t * rop){
	sst25vf_state_t * state = (sst25vf_state_t*)handle->state;
	const sst25vf_config_t * dcfg = (const sst25vf_config_t *)(handle->config);
	state->handler.callback = rop->handler.callback;
	state->handler.context = rop->handler.context;
	rop->handler.context = (void*)handle;
	rop->handler.callback = (mcu_callback_t)complete_spi_read;

	if ( rop->loc >= dcfg->size ){
		return EOF;
	}

	if ( rop->loc + rop->nbyte > dcfg->size ){
		rop->nbyte = dcfg->size - rop->loc; //update the bytes read to not go past the end of the disk
	}

	sst25vf_share_assert_cs(handle);
	sst25vf_share_write_opcode_addr(handle, SST25VF_INS_RD_HS, rop->loc);
	mcu_spi_swap(handle, NULL); //dummy byte output
	return mcu_spi_read(handle, rop);
}


void continue_spi_write(const devfs_handle_t * handle, uint32_t ignore){
    sst25vf_state_t * state = (sst25vf_state_t *)handle->state;
    const sst25vf_config_t * sst_cfg = (sst25vf_config_t*)handle->config;
	mcu_action_t action;
	uint8_t * addrp;
	devfs_handle_t tmr_handle;
	tmr_handle.port = sst_cfg->wp.port;
	//should be called 10 us after complete_spi_write() executes

	//Disable the TMR interrupt
	action.handler.callback = 0;
	action.handler.context = 0;
	action.o_events = MCU_EVENT_FLAG_NONE;
	action.channel = sst_cfg->wp.pin;
	action.prio = 0;
	mcu_tmr_disable(&tmr_handle, 0);
	mcu_tmr_setaction(&tmr_handle, &action);
	mcu_tmr_enable(&tmr_handle, 0);


	if( state->nbyte > 0 ){
        sst25vf_share_assert_cs(handle);
		addrp = (uint8_t*)&(state->op.loc);
		state->cmd[0] = SST25VF_INS_PROGRAM;
		state->cmd[1] = addrp[2];
		state->cmd[2] = addrp[1];
		state->cmd[3] = addrp[0];
		state->cmd[4] = state->buf[0];

		state->op.nbyte = 5;
        mcu_spi_write(handle, &(state->op));
		state->op.loc++;
		state->buf += 1;
		state->nbyte -= 1;
	} else {

        sst25vf_share_write_disable(handle);

        sst25vf_share_read_status(handle);

		//Set the buffer to NULL to indicate the device is not busy
		state->buf = NULL;

		//call the event handler to show the operation is complete
		if ( state->handler.callback != NULL ){
			state->handler.callback(state->handler.context, NULL);
			state->handler.callback = NULL;
		}
	}
}

void complete_spi_write(const devfs_handle_t * handle, uint32_t ignore){
	uint32_t tval;
	mcu_action_t action;
	mcu_channel_t channel;
    sst25vf_config_t * sst_cfg = (sst25vf_config_t*)handle->config;
	devfs_handle_t tmr_handle;
	tmr_handle.port = sst_cfg->wp.port;

    sst25vf_share_deassert_cs(handle);

	//configure the TMR to interrupt in 10 microseconds
    action.handler.context = (void*)handle;
	action.handler.callback = (mcu_callback_t)continue_spi_write;
	action.channel = sst_cfg->spi.attr.pin_assignment.miso.pin;
	action.o_events = MCU_EVENT_FLAG_MATCH;
	action.prio = 0;

	channel.loc = sst_cfg->wp.pin;

	//turn the timer off
	mcu_tmr_disable(&tmr_handle, 0);
	mcu_tmr_setaction(&tmr_handle, &action);
    mcu_tmr_get(&tmr_handle, &tval);
	channel.value = tval + 20;
	if( channel.value > (STFY_USECOND_PERIOD) ){
		channel.value -= (STFY_USECOND_PERIOD);
	}
	mcu_tmr_setchannel(&tmr_handle, &channel);

	//everything is set; turn the timer back on
	mcu_tmr_enable(&tmr_handle, 0);
}

int sst25vf_tmr_write(const devfs_handle_t * handle, devfs_async_t * wop){
	int err;
	uint8_t *addrp;
    sst25vf_state_t * state = (sst25vf_state_t*)handle->state;

	if ( state->buf != NULL ){
        return SYSFS_SET_RETURN(EBUSY);
	}

	//This is the final callback and context when all the writing is done
	state->handler.callback = wop->handler.callback;
	state->handler.context = wop->handler.context;
	state->buf = wop->buf;
	state->nbyte = wop->nbyte;

    sst25vf_share_write_enable(handle);

	//Byte program
	addrp = (uint8_t*)&(wop->loc);
	state->cmd[0] = SST25VF_INS_PROGRAM;
	state->cmd[1] = addrp[2];
	state->cmd[2] = addrp[1];
	state->cmd[3] = addrp[0];
	state->cmd[4] = state->buf[0];

    sst25vf_share_assert_cs(handle);
	state->op.flags = wop->flags;
	state->op.handler.callback = (mcu_callback_t)complete_spi_write;
    state->op.handler.context = (void*)handle;
	state->op.buf_const = state->cmd;
	state->op.nbyte = 5;
	state->op.loc = wop->loc;

    err = mcu_spi_write(handle, &state->op);
	state->op.loc++;
	state->buf = state->buf + 1;
	state->nbyte = state->nbyte - 1;
	return err;
}

int sst25vf_tmr_ioctl(const devfs_handle_t * handle, int request, void * ctl){
    return sst25vf_share_ioctl(handle, request, ctl);
}

int sst25vf_tmr_close(const devfs_handle_t * handle){
    sst25vf_share_power_down(handle);
    return mcu_spi_close(handle);
}



/*! @} */


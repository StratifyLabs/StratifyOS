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

#include <fcntl.h>
#include <errno.h>
#include <stddef.h>
#include "iface/dev/uartfifo.h"
#include "dev/uartfifo.h"
#include "mcu/debug.h"


static int set_read_action(const device_cfg_t * cfg, mcu_callback_t callback){
	mcu_action_t action;
	const uartfifo_cfg_t * cfgp = cfg->dcfg;
	action.callback = callback;
	action.context = (void*)cfg;
	action.event = UART_EVENT_DATA_READY;
	action.prio = 0;
	if( mcu_uart_setaction(cfgp->port, &action) < 0 ){
		return -1;
	}

	return 0;
}

static int data_received(void * context, mcu_event_t data){
	char c;
	const device_cfg_t * cfg;
	const uartfifo_cfg_t * cfgp;
	uartfifo_state_t * state;
	cfg = context;
	cfgp = cfg->dcfg;
	state = cfg->state;

	while( mcu_uart_getbyte(cfgp->port, &c) == 0 ){
		cfgp->fifo.buffer[ state->fifo.head ] = c;
		fifo_inc_head(&(state->fifo), cfgp->fifo.size);
	}


	fifo_data_received(&(cfgp->fifo), &(state->fifo));

	return 1; //leave the callback in place
}

int uartfifo_open(const device_cfg_t * cfg){
	const uartfifo_cfg_t * cfgp = cfg->dcfg;
	uartfifo_state_t * state = cfg->state;
	fifo_flush(&(state->fifo));
	state->fifo.rop = NULL;
	//setup the device to write to the fifo when data arrives
	if( mcu_uart_open((const device_cfg_t*)&(cfgp->port)) < 0 ){
		return -1;
	}

	return 0;
}

int uartfifo_ioctl(const device_cfg_t * cfg, int request, void * ctl){
	fifo_attr_t * attr = ctl;
	mcu_action_t * action = ctl;
	const uartfifo_cfg_t * cfgp = cfg->dcfg;
	uartfifo_state_t * state = cfg->state;
	int ret;
	switch(request){
	case I_FIFO_GETATTR:
		fifo_getattr(attr, &(cfgp->fifo), &(state->fifo));
		break;
	case I_GLOBAL_SETACTION:
	case I_UART_SETACTION:
		if( action->callback == 0 ){
			//This needs to cancel an ongoing operation
			fifo_cancel_rop(&(state->fifo));
			return 0;
		}
		errno = ENOTSUP;
		return -1;
	case I_UART_FLUSH:
	case I_FIFO_FLUSH:
		fifo_flush(&(state->fifo));
		return mcu_uart_flush(cfgp->port, NULL);
		break;
	case I_UART_SETATTR:
		if(  (ret = mcu_uart_setattr(cfgp->port, ctl)) < 0 ){
			return ret;
		}
		/* no break */
	case I_FIFO_INIT:
		fifo_flush(&(state->fifo));
		if ( set_read_action(cfg, data_received) < 0 ){
			return -1;
		}
		break;
	default:
		return mcu_uart_ioctl((const device_cfg_t*)&(cfgp->port), request, ctl);
}
return 0;
}


int uartfifo_read(const device_cfg_t * cfg, device_transfer_t * rop){
	const uartfifo_cfg_t * cfgp = cfg->dcfg;
	uartfifo_state_t * state = cfg->state;
	return fifo_read_local(&(cfgp->fifo), &(state->fifo), rop);
}

int uartfifo_write(const device_cfg_t * cfg, device_transfer_t * wop){
	const uartfifo_cfg_t * cfgp = cfg->dcfg;

	//FIFO is not used for writing; hardware is written directly
	return mcu_uart_write((const device_cfg_t*)&(cfgp->port), wop);
}

int uartfifo_close(const device_cfg_t * cfg){
	const uartfifo_cfg_t * cfgp = cfg->dcfg;

	//clear the callback for the device
	if( set_read_action(cfg, NULL) < 0 ){
		return -1;
	}

	return mcu_uart_close((const device_cfg_t*)&(cfgp->port));
}


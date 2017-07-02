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
#include <stddef.h>
#include "mcu/debug.h"
#include "mcu/pio.h"
#include "mcu/core.h"

typedef struct {
	mcu_callback_t callback;
	void * context;
	uint8_t ref_count;
} pio_local_t;

static pio_local_t _mcu_pio_local[MCU_PIO_PORTS] MCU_SYS_MEM;
static int _mcu_pio_set_event(int port, int event, int pin);
__IO uint32_t * _mcu_get_iocon_regs(int port, int pin);


void _mcu_pio_dev_power_on(int port){
	if ( _mcu_pio_local[port].ref_count == 0 ){
		_mcu_pio_local[port].callback = NULL;
	}
	_mcu_pio_local[port].ref_count++;
}

void _mcu_pio_dev_power_off(int port){
	if ( _mcu_pio_local[port].ref_count > 0 ){
		if ( _mcu_pio_local[port].ref_count == 1 ){
			_mcu_pio_local[port].callback = NULL;
		}
		_mcu_pio_local[port].ref_count--;
	}
}

int _mcu_pio_dev_powered_on(int port){
	LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_GPIO;

	if ( _mcu_pio_local[port].ref_count > 0 ){
		return 1;
	} else {
		return 0;
	}
}

int _mcu_pio_set_event(int port, int event, int pin){
	uint32_t polarity;

	polarity = 0;

	if( event == PIO_ACTION_EVENT_RISING ){
		polarity = (1<<pin);
	} else if( event == PIO_ACTION_EVENT_BOTH ) {
		return -1;
	}

	if( port == 0 ){

	}
	return 0;
}

int _mcu_pio_dev_write(const device_cfg_t * cfg, device_transfer_t * wop){
	int port;
	mcu_action_t * action;
	port = cfg->periph.port;

	if( wop->nbyte != sizeof(mcu_action_t) ){
		errno = EINVAL;
		return -1;
	}

	action = wop->buf;
	action->callback = wop->callback;
	action->context = wop->context;
	return mcu_pio_setaction(port, &action);
}


int mcu_pio_setaction(int port, void * ctl){
	int err;
	mcu_action_t * action = (mcu_action_t*)ctl;

	err = _mcu_pio_set_event(port, action->event, action->channel);
	if ( err ){
		return err;
	}

	_mcu_pio_local[port].callback = action->callback;
	_mcu_pio_local[port].context = action->context;

	//set the interrupt on the GPIO Group pins
	//_mcu_cortexm_priv_enable_irq((void*)(GINT0_IRQn + port));

	return 0;
}


int mcu_pio_getattr(int port, void * ctl){
	//read the direction pin status
	errno = ENOTSUP;
	return -1;
}

int mcu_pio_setattr(int port, void * ctl){
	int i;
	int mode;
	pio_attr_t * attr;
	attr = ctl;
	__IO uint32_t * regs_iocon;

	if( attr->mode & PIO_MODE_INPUT ){
		//set as input
		//LPC_GPIO->DIR[port] &= ~attr->mask;
	}

	if( attr->mode & (PIO_MODE_OUTPUT) ){
		//set output pins as output
		//LPC_GPIO->DIR[port] |= attr->mask;
	}

	if( attr->mode & PIO_MODE_DIRONLY ){
		return 0;
	}

	mode = (1<<6); //bit 6 is reserved and always set to 1
	if( attr->mode & PIO_MODE_PULLUP ){
		mode |= (2<<3);
	} else if( attr->mode & PIO_MODE_REPEATER ){
		mode |= (3<<3);
	} else if( attr->mode & PIO_MODE_FLOAT ){
		mode |= (0<<3);
	} else if ( attr->mode & PIO_MODE_PULLDOWN ){
		mode |= 1<<3;
	}

	if( attr->mode & PIO_MODE_HYSTERESIS ){
		mode |= 1<<5;
	}

	if( !(attr->mode & PIO_MODE_ANALOG) ){
		mode |= (1<<7);
	}

	if( attr->mode & PIO_MODE_OPENDRAIN ){
		mode |= 1<<10;
	}

	for(i = 0; i < 12; i++){
		if ( (1<<i) & (attr->mask) ){
			regs_iocon = _mcu_get_iocon_regs(port, i);
			if( regs_iocon != NULL ){
				*regs_iocon = mode;
				_mcu_core_set_pinsel_func(port, i, CORE_PERIPH_PIO, port); //set the pin to use GPIO

				//special case for I2C values
				if( port == 0 ){
					if( (i == 4) || (i == 5)){
						*regs_iocon |= (1<<8); //GPIO mode
					}
				}
			}
		}
	}

	return 0;
}

int mcu_pio_setmask(int port, void * ctl){
	uint32_t mask = (int)ctl;
	//LPC_GPIO->SET[port] = mask;
	return 0;
}

int mcu_pio_clrmask(int port, void * ctl){
	uint32_t mask = (int)ctl;
	//LPC_GPIO->CLR[port] = mask;
	return 0;
}

int mcu_pio_get(int port, void * ctl){
	//return LPC_GPIO->PIN[port];
}

int mcu_pio_set(int port, void * ctl){
	//LPC_GPIO->PIN[port] = (uint32_t)ctl;
	return 0;
}

void _mcu_core_pio_isr(int port){
	//clear the interrupt
	if( port == 0 ){
		//LPC_GPIO_GROUP_INT0->CTRL |= 1;
	} else {
		//LPC_GPIO_GROUP_INT1->CTRL |= 1;
	}

	if( _mcu_pio_local[port].callback != 0 ){
		if( _mcu_pio_local[port].callback(_mcu_pio_local[port].context, 0) == 0 ){
			_mcu_pio_local[port].callback = 0;
		}
	}
}

void _mcu_core_pio0_isr(){ _mcu_core_pio_isr(0); }
void _mcu_core_pio1_isr(){ _mcu_core_pio_isr(1); }

__IO uint32_t * _mcu_get_iocon_regs(int port, int pin){
	__IO uint32_t * regs;
	switch(port){
	case 0:
		//regs = &LPC_IOCON->RESET_PIO0_0 + pin;
		break;
	case 1:
		//pins 1.6,1.9,1.12,1.30 don't exist
		if( pin > 6 ) pin--;
		if( pin > 9 ) pin--;
		if( pin > 12 ) pin--;
		if( pin > 30 ) pin--;
		//regs = &LPC_IOCON->PIO1_0 + pin;
		break;
	default:
		regs = 0;
		break;
	}

	return regs;
}


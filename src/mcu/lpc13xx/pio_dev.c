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
static LPC_GPIO_Type * const pio_regs_table[MCU_PIO_PORTS] = MCU_PIO_REGS;
__IO uint32_t * _mcu_get_iocon_regs(int port, int pin);

static __IO uint32_t * const iocon_table[3*12+6] = {
		(__IO uint32_t*)(LPC_IOCON_BASE + offsetof(LPC_IOCON_Type, RESET_PIO0_0)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO0_1)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO0_2)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO0_3)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO0_4)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO0_5)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO0_6)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO0_7)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO0_8)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO0_9)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, JTAG_TCK_PIO0_10)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, JTAG_TDI_PIO0_11)),
		(__IO uint32_t*)(LPC_IOCON_BASE + offsetof(LPC_IOCON_Type, JTAG_TMS_PIO1_0)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, JTAG_TDO_PIO1_1)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, JTAG_nTRST_PIO1_2)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, SWDIO_PIO1_3)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO1_4)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO1_5)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO1_6)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO1_7)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO1_8)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO1_9)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO1_10)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO1_11)),
		(__IO uint32_t*)(LPC_IOCON_BASE + offsetof(LPC_IOCON_Type, PIO2_0)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO2_1)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO2_2)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO2_3)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO2_4)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO2_5)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO2_6)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO2_7)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO2_8)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO2_9)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO2_10)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO2_11)),
		(__IO uint32_t*)(LPC_IOCON_BASE + offsetof(LPC_IOCON_Type, PIO3_0)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO3_1)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO3_2)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO3_3)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO3_4)),
		(__IO uint32_t*)(LPC_IOCON_BASE+ offsetof(LPC_IOCON_Type, PIO3_5))
};

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
	uint32_t mask;
	LPC_GPIO_Type * regs = pio_regs_table[port];

	mask = (1<<pin);

	//turn interrupt off
	regs->IE &= ~(mask);

	switch(event){
	case PIO_ACTION_EVENT_NONE:
		return 0;
	case PIO_ACTION_EVENT_RISING:
		regs->IEV |= mask;
		regs->IBE &= ~mask;
		break;
	case PIO_ACTION_EVENT_FALLING:
		regs->IEV &= ~mask;
		regs->IBE &= ~mask;
		break;
	case PIO_ACTION_EVENT_BOTH:
		regs->IBE |= mask;
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	regs->IE |= (mask);

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

	//set the interrupt on the EINT pins
	_mcu_cortexm_priv_enable_irq((void*)(EINT3_IRQn + (3-port)));

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
	LPC_GPIO_Type * gpio_regs = pio_regs_table[port];

	if( attr->mode & PIO_MODE_INPUT ){
		//set as input
		gpio_regs->DIR &= ~attr->mask;
	}

	if( attr->mode & (PIO_MODE_OUTPUT) ){
		//set output pins as output
		gpio_regs->DIR |= attr->mask;
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
	LPC_GPIO_Type * gpio_regs = pio_regs_table[port];
	uint32_t mask = (int)ctl;
	gpio_regs->MASKED_ACCESS[mask] = mask;
	return 0;
}

int mcu_pio_clrmask(int port, void * ctl){
	LPC_GPIO_Type * gpio_regs = pio_regs_table[port];
	uint32_t mask = (int)ctl;
	gpio_regs->MASKED_ACCESS[mask] = 0;
	return 0;
}

int mcu_pio_get(int port, void * ctl){
	LPC_GPIO_Type * gpio_regs = pio_regs_table[port];
	return gpio_regs->DATA;
}

int mcu_pio_set(int port, void * ctl){
	LPC_GPIO_Type * regs = pio_regs_table[port];
	regs->DATA=(int)ctl;
	return 0;
}

void _mcu_core_pio_isr(int port){
	LPC_GPIO_Type * regs = pio_regs_table[port];
	uint32_t status;
	status = regs->RIS;
	regs->IC |= status;
	if( _mcu_pio_local[port].callback != 0 ){
		if( _mcu_pio_local[port].callback(_mcu_pio_local[port].context, &status) == 0 ){
			_mcu_pio_local[port].callback = 0;
		}
	}
}

void _mcu_core_pio0_isr(){ _mcu_core_pio_isr(0); }
void _mcu_core_pio1_isr(){ _mcu_core_pio_isr(1); }
void _mcu_core_pio2_isr(){ _mcu_core_pio_isr(2); }
void _mcu_core_pio3_isr(){ _mcu_core_pio_isr(3); }

__IO uint32_t * _mcu_get_iocon_regs(int port, int pin){
	switch(port){
	case 0:
	case 1:
	case 2:
		if( pin > 11 ){
			return NULL;
		}
		break;
	case 3:
		if( pin > 5 ){
			return NULL;
		}
		break;
	}
	return iocon_table[port*12+pin];
}


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
#include "mcu/eint.h"
#include "mcu/pio.h"
#include "mcu/core.h"

typedef struct MCU_PACK {
	mcu_callback_t callback;
	void * context;
	uint8_t ref_count;
} eint_local_t;

static eint_local_t _mcu_eint_local[MCU_EINT_PORTS] MCU_SYS_MEM;
static int _mcu_eint_set_event(int port, int event);

void _mcu_eint_dev_power_on(int port){
	if ( _mcu_eint_local[port].ref_count == 0 ){
		_mcu_eint_local[port].callback = NULL;
	}
	_mcu_eint_local[port].ref_count++;
}

void _mcu_eint_dev_power_off(int port){
	if ( _mcu_eint_local[port].ref_count > 0 ){
		if ( _mcu_eint_local[port].ref_count == 1 ){
			_mcu_eint_local[port].callback = NULL;
		}
		_mcu_eint_local[port].ref_count--;
	}
}

int _mcu_eint_dev_powered_on(int port){
	LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_GPIO;

	if ( _mcu_eint_local[port].ref_count > 0 ){
		return 1;
	} else {
		return 0;
	}
}

int _mcu_eint_set_event(int port, int event){
	uint32_t mask;
	volatile uint32_t * start_logic_edge;
	volatile uint32_t * start_logic_enable;
	volatile uint32_t * start_logic_clear;

	if( port > 31 ){
		port = port - 32;
		start_logic_edge = &(LPC_SYSCON->STARTAPRP1);
		start_logic_enable = &(LPC_SYSCON->STARTERP1);
		start_logic_clear = &(LPC_SYSCON->STARTRSRP1CLR);
	} else {
		start_logic_edge = &(LPC_SYSCON->STARTAPRP0);
		start_logic_enable = &(LPC_SYSCON->STARTERP0);
		start_logic_clear = &(LPC_SYSCON->STARTRSRP0CLR);
	}

	mask = (1<<port);


	//turn interrupt off
	*start_logic_enable &= ~(mask);

	switch(event){
	case EINT_ACTION_EVENT_UNCONFIGURED:
		return 0;
	case EINT_ACTION_EVENT_RISING:
		*start_logic_edge |= mask;
		break;
	case EINT_ACTION_EVENT_FALLING:
		*start_logic_edge &= ~mask;
		break;
	case EINT_ACTION_EVENT_BOTH:
	default:
		errno = EINVAL;
		return -1;
	}

	*start_logic_enable |= (mask);
	*start_logic_clear |= mask;
	return 0;
}

int _mcu_eint_dev_write(const device_cfg_t * cfg, device_transfer_t * wop){
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
	return mcu_eint_setaction(port, &action);
}


int mcu_eint_setaction(int port, void * ctl){
	int err;
	mcu_action_t * action = (mcu_action_t*)ctl;

	err = _mcu_eint_set_event(port, action->event);
	if ( err ){
		return err;
	}

	_mcu_eint_local[port].callback = action->callback;
	_mcu_eint_local[port].context = action->context;

	//set the interrupt on the wakeup pins
	_mcu_cortexm_priv_enable_irq((void*)(WAKEUP0_IRQn + port));

	return 0;
}


int mcu_eint_getattr(int port, void * ctl){
	//read the direction pin status
	int pio_port, pinmask;
	pio_sample_t p;
	eint_attr_t * attr = (eint_attr_t*)ctl;
	pio_port = port / 12;
	pinmask = 1<< (port % 12);
	p = mcu_pio_get(pio_port, 0);
	attr->pin_assign = 0;
	attr->value = ((p & (pinmask)) == pinmask);
	return 0;
}

int mcu_eint_setattr(int port, void * ctl){
	return 0;
}

void _mcu_core_eint_isr(int port){
	uint32_t mask;
	//clear the interrupt flag
	mask = (1<<port);
	LPC_SYSCON->STARTRSRP0CLR |= mask;

	if( _mcu_eint_local[port].callback != 0 ){
		if( _mcu_eint_local[port].callback(_mcu_eint_local[port].context, 0) == 0 ){
			_mcu_eint_local[port].callback = 0;
		}
	}

}

void _mcu_core_eint_isr_32(int port){
	uint32_t pin = port - 32;
	uint32_t mask;
	mask = (1<<pin);
	LPC_SYSCON->STARTRSRP1CLR |= mask;

	if( _mcu_eint_local[port].callback != 0 ){
		if( _mcu_eint_local[port].callback(_mcu_eint_local[port].context, 0) == 0 ){
			_mcu_eint_local[port].callback = 0;
		}
	}

}


void _mcu_core_eint0_isr(){ _mcu_core_eint_isr(0); }
void _mcu_core_eint1_isr(){ _mcu_core_eint_isr(1); }
void _mcu_core_eint2_isr(){ _mcu_core_eint_isr(2); }
void _mcu_core_eint3_isr(){ _mcu_core_eint_isr(3); }
void _mcu_core_eint4_isr(){ _mcu_core_eint_isr(4); }
void _mcu_core_eint5_isr(){ _mcu_core_eint_isr(5); }
void _mcu_core_eint6_isr(){ _mcu_core_eint_isr(6); }
void _mcu_core_eint7_isr(){ _mcu_core_eint_isr(7); }
void _mcu_core_eint8_isr(){ _mcu_core_eint_isr(8); }
void _mcu_core_eint9_isr(){ _mcu_core_eint_isr(9); }
void _mcu_core_eint10_isr(){ _mcu_core_eint_isr(10); }
void _mcu_core_eint11_isr(){ _mcu_core_eint_isr(11); }

void _mcu_core_eint12_isr(){ _mcu_core_eint_isr(12); }
void _mcu_core_eint13_isr(){ _mcu_core_eint_isr(13); }
void _mcu_core_eint14_isr(){ _mcu_core_eint_isr(14); }
void _mcu_core_eint15_isr(){ _mcu_core_eint_isr(15); }
void _mcu_core_eint16_isr(){ _mcu_core_eint_isr(16); }
void _mcu_core_eint17_isr(){ _mcu_core_eint_isr(17); }
void _mcu_core_eint18_isr(){ _mcu_core_eint_isr(18); }
void _mcu_core_eint19_isr(){ _mcu_core_eint_isr(19); }
void _mcu_core_eint20_isr(){ _mcu_core_eint_isr(20); }
void _mcu_core_eint21_isr(){ _mcu_core_eint_isr(21); }
void _mcu_core_eint22_isr(){ _mcu_core_eint_isr(22); }
void _mcu_core_eint23_isr(){ _mcu_core_eint_isr(23); }

void _mcu_core_eint24_isr(){ _mcu_core_eint_isr(24); }
void _mcu_core_eint25_isr(){ _mcu_core_eint_isr(25); }
void _mcu_core_eint26_isr(){ _mcu_core_eint_isr(26); }
void _mcu_core_eint27_isr(){ _mcu_core_eint_isr(27); }
void _mcu_core_eint28_isr(){ _mcu_core_eint_isr(28); }
void _mcu_core_eint29_isr(){ _mcu_core_eint_isr(29); }
void _mcu_core_eint30_isr(){ _mcu_core_eint_isr(30); }
void _mcu_core_eint31_isr(){ _mcu_core_eint_isr(31); }
void _mcu_core_eint32_isr(){ _mcu_core_eint_isr_32(32); }
void _mcu_core_eint33_isr(){ _mcu_core_eint_isr_32(33); }
void _mcu_core_eint34_isr(){ _mcu_core_eint_isr_32(34); }
void _mcu_core_eint35_isr(){ _mcu_core_eint_isr_32(35); }

void _mcu_core_eint36_isr(){ _mcu_core_eint_isr_32(36); }
void _mcu_core_eint37_isr(){ _mcu_core_eint_isr_32(37); }
void _mcu_core_eint38_isr(){ _mcu_core_eint_isr_32(38); }
void _mcu_core_eint39_isr(){ _mcu_core_eint_isr_32(39); }



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
#include "mcu/eint.h"
#include "mcu/core.h"
#include "mcu/debug.h"

#if MCU_EINT_PORTS > 0


typedef struct {
	mcu_event_handler_t handler;
	uint8_t ref_count;
} eint_local_t;
static eint_local_t eint_local[4] MCU_SYS_MEM;


//static eint_action_event_t get_event(int port);
static int set_event(int port, eint_action_event_t event);
static void reset_eint_port(int port);
static void exec_callback(int port, void * data);


void _mcu_eint_dev_power_on(int port){
	if ( eint_local[port].ref_count == 0 ){
		eint_local[port].ref_count++;
		reset_eint_port(port);
	}
	eint_local[port].ref_count++;
}

void _mcu_eint_dev_power_off(int port){
	if ( eint_local[port].ref_count > 0 ){
		if ( eint_local[port].ref_count == 1 ){
			reset_eint_port(port);
		}
		eint_local[port].ref_count--;
	}
}

int _mcu_eint_dev_powered_on(int port){
	return eint_local[port].ref_count;
}

int _mcu_eint_dev_write(const device_cfg_t * cfg, device_transfer_t * wop){
	int port;
	mcu_action_t * action;
	port = cfg->periph.port;
	if ( eint_local[port].handler.callback != 0 ){
		//The interrupt is on -- port is busy
		errno = EAGAIN;
		return -1;
	}
	if( wop->nbyte != sizeof(mcu_action_t) ){
		errno = EINVAL;
		return -1;
	}

	action = (mcu_action_t*)wop->buf;
	action->callback = wop->callback;
	action->context = wop->context;
	return mcu_eint_setaction(port, action);
}

int mcu_eint_setaction(int port, void * ctl){
	mcu_action_t * action = (mcu_action_t*)ctl;

	if( action->callback == 0 ){
		exec_callback(port, MCU_EVENT_SET_CODE(MCU_EVENT_OP_CANCELLED));
	}

	if( _mcu_core_priv_validate_callback(action->callback) < 0 ){
		return -1;
	}

	eint_local[port].handler.callback = action->callback;
	eint_local[port].handler.context = action->context;

	set_event(port, action->event);
	_mcu_core_setirqprio(EINT0_IRQn + port, action->prio);

	return 0;
}

int mcu_eint_getattr(int port, void * ctl){
	eint_attr_t * ctlp;
	ctlp = (eint_attr_t *)ctl;
	ctlp->pin_assign = 0;

#ifdef LPC_GPIO2
	switch(port){
	case 0:
		ctlp->value = ((LPC_GPIO2->PIN & (1<<10)) == (1<<10));
		break;
	case 1:
		ctlp->value = ((LPC_GPIO2->PIN & (1<<11)) == (1<<11));
		break;
	case 2:
		ctlp->value = ((LPC_GPIO2->PIN & (1<<12)) == (1<<12));
		break;
	case 3:
		ctlp->value = ((LPC_GPIO2->PIN & (1<<13)) == (1<<13));
		break;
	}
#endif

	return 0;
}

int mcu_eint_setattr(int port, void * ctl){
	eint_attr_t * ctlp;
	pio_attr_t pattr;

	ctlp = (eint_attr_t *)ctl;

	if ( ctlp->pin_assign != 0 ){
		errno = EINVAL;
		return -1 - offsetof(eint_attr_t, pin_assign);
	}

	reset_eint_port(port);

	pattr.mode = ctlp->mode;

	switch(port){
	case 0:
		//set the pinmode
		pattr.mask = 1<<10;
		mcu_pio_setattr(2, &pattr);
		_mcu_core_set_pinsel_func(2,10,CORE_PERIPH_EINT,0);
		break;
	case 1:
		pattr.mask = 1<<11;
		mcu_pio_setattr(2, &pattr);
		_mcu_core_set_pinsel_func(2,11,CORE_PERIPH_EINT,1);
		break;
	case 2:
		pattr.mask = 1<<12;
		mcu_pio_setattr(2, &pattr);
		_mcu_core_set_pinsel_func(2,12,CORE_PERIPH_EINT,2);
		break;
	case 3:
		pattr.mask = 1<<13;
		mcu_pio_setattr(2, &pattr);
		_mcu_core_set_pinsel_func(2,13,CORE_PERIPH_EINT,3);
		break;
	}


	return 0;
}

void reset_eint_port(int port){
	_mcu_core_priv_disable_irq((void*)(EINT0_IRQn + port));

	eint_local[port].handler.callback = 0;

	LPC_SC->EXTPOLAR &= ~(1<<port);
	LPC_SC->EXTMODE &= ~(1<<port);

	switch(port){
	case 0:
		_mcu_core_set_pinsel_func(2,10,CORE_PERIPH_PIO,2);
		break;
	case 1:
		_mcu_core_set_pinsel_func(2,11,CORE_PERIPH_PIO,2);
		break;
	case 2:
		_mcu_core_set_pinsel_func(2,12,CORE_PERIPH_PIO,2);
		break;
	case 3:
		_mcu_core_set_pinsel_func(2,13,CORE_PERIPH_PIO,2);
		break;
	}

	LPC_SC->EXTINT |= (1<<port); //Clear the interrupt flag
}

int set_event(int port, eint_action_event_t event){
	int err;
	err = 0;

	_mcu_core_priv_disable_irq((void*)(EINT0_IRQn + port));

	LPC_SC->EXTPOLAR &= ~(1<<port);
	LPC_SC->EXTMODE &= ~(1<<port);
	switch(event){
	case EINT_ACTION_EVENT_RISING:
		LPC_SC->EXTPOLAR |= (1<<port);
		LPC_SC->EXTMODE |= (1<<port);
		break;
	case EINT_ACTION_EVENT_FALLING:
		LPC_SC->EXTMODE |= (1<<port);
		break;
	case EINT_ACTION_EVENT_HIGH:
		LPC_SC->EXTPOLAR |= (1<<port);
		break;
	case EINT_ACTION_EVENT_LOW:
		break;
	case EINT_ACTION_EVENT_UNCONFIGURED:
		LPC_SC->EXTINT |= (1<<port); //Clear the interrupt flag
		return 0;
		break;
	case EINT_ACTION_EVENT_BOTH:
	default:
		errno = EINVAL;
		err = -1;
		break;
	}
	if ( err == 0 ){
		LPC_SC->EXTINT |= (1<<port); //Clear the interrupt flag
	}

	_mcu_core_priv_enable_irq((void*)EINT0_IRQn + port);
	return err;
}

/*
eint_action_event_t get_event(int port){
	uint32_t polar;
	uint32_t mode;

	polar = LPC_SC->EXTPOLAR;
	mode = LPC_SC->EXTMODE;

	if ( polar & (1<<port) ){
		//High or rising
		if ( mode & (1<<port) ){
			//edge sensitive
			return EINT_ACTION_EVENT_RISING;
		} else {
			return EINT_ACTION_EVENT_HIGH;
		}
	} else {
		//low or falling
		if ( mode & (1<<port) ){
			//edge sensitive
			return EINT_ACTION_EVENT_FALLING;
		} else {
			return EINT_ACTION_EVENT_LOW;
		}
	}
}
 */

void exec_callback(int port, void * data){
	_mcu_core_exec_event_handler(&(eint_local[port].handler), data);
}

//Interrupt Handling
void _mcu_eint_isr(int port){
	exec_callback(port, 0);
	LPC_SC->EXTINT |= (1<<port); //Clear the interrupt flag
}



void _mcu_core_eint0_isr(){
	_mcu_eint_isr(0);
}

void _mcu_core_eint1_isr(){
	_mcu_eint_isr(1);
}

void _mcu_core_eint2_isr(){
	_mcu_eint_isr(2);
}

#if defined __lpc17xx
extern void _mcu_core_pio0_isr();
#endif

void _mcu_core_eint3_isr(){
	//check for GPIO interrupts first
#if defined __lpc17xx
	if ( LPC_GPIOINT->IntStatus ){
		_mcu_core_pio0_isr();
	}
#endif

	if ( LPC_SC->EXTINT & (1<<3)){
		_mcu_eint_isr(3);
	}
}

#endif

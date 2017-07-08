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
#include "mcu/cortexm.h"
#include "mcu/eint.h"
#include "mcu/pio.h"
#include "mcu/core.h"
#include "mcu/debug.h"

#if MCU_EINT_PORTS > 0


typedef struct {
	mcu_event_handler_t handler;
	uint8_t ref_count;
} eint_local_t;
static eint_local_t eint_local[4] MCU_SYS_MEM;


//static eint_action_event_t get_event(int port);
static int set_event(int port, u32 event);
static void reset_eint_port(int port);
static void exec_callback(int port, u32 o_events);


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

int _mcu_eint_dev_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	int port;
	mcu_action_t * action;
	port = cfg->port;
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
	action->handler.callback = wop->handler.callback;
	action->handler.context = wop->handler.context;
	return mcu_eint_setaction(port, action);
}

int mcu_eint_setaction(int port, void * ctl){
	mcu_action_t * action = (mcu_action_t*)ctl;

	if( action->handler.callback == 0 ){
		exec_callback(port, MCU_EVENT_FLAG_CANCELED);
	}

	if( _mcu_cortexm_priv_validate_callback(action->handler.callback) < 0 ){
		return -1;
	}

	eint_local[port].handler.callback = action->handler.callback;
	eint_local[port].handler.context = action->handler.context;

	set_event(port, action->o_events);
	_mcu_cortexm_set_irq_prio(EINT0_IRQn + port, action->prio);

	return 0;
}

int mcu_eint_getinfo(int port, void * ctl){
	eint_info_t * info = ctl;

	info->o_flags = 0;

	return 0;
}

int mcu_eint_setattr(int port, void * ctl){
	eint_attr_t * attr = ctl;
	pio_attr_t pattr;

	reset_eint_port(port);

	pattr.o_flags = attr->o_flags;
	int i;
	for(i=0; i < EINT_PIN_ASSIGNMENT_COUNT; i++){
		if( mcu_is_port_valid(attr->pin_assignment[i].port) ){
			pattr.o_pinmask = 1<<attr->pin_assignment[i].pin;
			mcu_pio_setattr(attr->pin_assignment[i].port, &pattr);
			if ( _mcu_core_set_pinsel_func(attr->pin_assignment[i].port, attr->pin_assignment[i].pin, CORE_PERIPH_EINT, port) ){
				return -1;  //pin failed to allocate as a UART pin
			}
		}
	}

	return 0;
}

void reset_eint_port(int port){
	_mcu_cortexm_priv_disable_irq((void*)(EINT0_IRQn + port));

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

int set_event(int port, u32 o_events){
	int err;
	err = 0;

	_mcu_cortexm_priv_disable_irq((void*)(EINT0_IRQn + port));

	LPC_SC->EXTPOLAR &= ~(1<<port);
	LPC_SC->EXTMODE &= ~(1<<port);
	if( o_events & MCU_EVENT_FLAG_RISING ){
		LPC_SC->EXTPOLAR |= (1<<port);
		LPC_SC->EXTMODE |= (1<<port);
	} else if( o_events & MCU_EVENT_FLAG_FALLING ){
		LPC_SC->EXTMODE |= (1<<port);
	} else if( o_events & MCU_EVENT_FLAG_HIGH ){
		LPC_SC->EXTPOLAR |= (1<<port);
	} else if( o_events & MCU_EVENT_FLAG_LOW){

	} else {
		LPC_SC->EXTINT |= (1<<port); //Clear the interrupt flag
		return 0;
	}

	if ( err == 0 ){
		LPC_SC->EXTINT |= (1<<port); //Clear the interrupt flag
	}

	_mcu_cortexm_priv_enable_irq((void*)EINT0_IRQn + port);
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

void exec_callback(int port, u32 o_events){
	mcu_execute_event_handler(&(eint_local[port].handler), o_events, 0);
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

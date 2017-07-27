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
#include "cortexm/cortexm.h"
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


void mcu_eint_dev_power_on(const devfs_handle_t * handle){
	int port = handle->port;
	if ( eint_local[port].ref_count == 0 ){
		eint_local[port].ref_count++;
		reset_eint_port(port);
	}
	eint_local[port].ref_count++;
}

void mcu_eint_dev_power_off(const devfs_handle_t * handle){
	int port = handle->port;
	if ( eint_local[port].ref_count > 0 ){
		if ( eint_local[port].ref_count == 1 ){
			reset_eint_port(port);
		}
		eint_local[port].ref_count--;
	}
}

int mcu_eint_dev_is_powered(const devfs_handle_t * handle){
	int port = handle->port;
	return eint_local[port].ref_count;
}

int mcu_eint_dev_write(const devfs_handle_t * handle, devfs_async_t * wop){
	int port;
	mcu_action_t * action;
	port = handle->port;
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
	return mcu_eint_setaction(handle, action);
}

int mcu_eint_setaction(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	mcu_action_t * action = (mcu_action_t*)ctl;

	if( action->handler.callback == 0 ){
		exec_callback(port, MCU_EVENT_FLAG_CANCELED);
	}

	if( cortexm_validate_callback(action->handler.callback) < 0 ){
		return -1;
	}

	eint_local[port].handler.callback = action->handler.callback;
	eint_local[port].handler.context = action->handler.context;

	set_event(port, action->o_events);
	cortexm_set_irq_prio(EINT0_IRQn + port, action->prio);

	return 0;
}

int mcu_eint_getinfo(const devfs_handle_t * handle, void * ctl){
	eint_info_t * info = ctl;

	info->o_flags = 0;

	return 0;
}

int mcu_eint_setattr(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	pio_attr_t pattr;
	devfs_handle_t pio_handle;


	const eint_attr_t * attr = mcu_select_attr(handle, ctl);
	if( attr == 0 ){
		return -1;
	}

	reset_eint_port(port);

	pattr.o_flags = attr->o_flags;
	int i;
	for(i=0; i < MCU_PIN_ASSIGNMENT_COUNT(eint_pin_assignment_t); i++){
		const mcu_pin_t * pin = mcu_pin_at(&(attr->pin_assignment), i);
		if( mcu_is_port_valid(pin->port) ){
			pattr.o_pinmask = 1<<pin->pin;
			pio_handle.port = pin->port;
			pio_handle.config = 0;
			mcu_pio_setattr(&pio_handle, &pattr);
			if ( mcu_core_set_pinsel_func(pin, CORE_PERIPH_EINT, port) ){
				return -1;  //pin failed to allocate as a UART pin
			}
		}
	}

	return 0;
}

void reset_eint_port(int port){
	cortexm_disable_irq((void*)(EINT0_IRQn + port));

	eint_local[port].handler.callback = 0;

	LPC_SC->EXTPOLAR &= ~(1<<port);
	LPC_SC->EXTMODE &= ~(1<<port);

	mcu_pin_t pin;
	pin.port = 2;

	switch(port){
	case 0:
		pin.pin = 10;
		break;
	case 1:
		pin.pin = 11;
		break;
	case 2:
		pin.pin = 12;
		break;
	case 3:
		pin.pin = 13;
		break;
	}

	mcu_core_set_pinsel_func(&pin,CORE_PERIPH_PIO,2);


	LPC_SC->EXTINT |= (1<<port); //Clear the interrupt flag
}

int set_event(int port, u32 o_events){
	int err;
	err = 0;

	cortexm_disable_irq((void*)(EINT0_IRQn + port));

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

	cortexm_enable_irq((void*)EINT0_IRQn + port);
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
void mcu_eint_isr(int port){
	exec_callback(port, 0);
	LPC_SC->EXTINT |= (1<<port); //Clear the interrupt flag
}



void mcu_core_eint0_isr(){
	mcu_eint_isr(0);
}

void mcu_core_eint1_isr(){
	mcu_eint_isr(1);
}

void mcu_core_eint2_isr(){
	mcu_eint_isr(2);
}

#if defined __lpc17xx
extern void mcu_core_pio0_isr();
#endif

void mcu_core_eint3_isr(){
	//check for GPIO interrupts first
#if defined __lpc17xx
	if ( LPC_GPIOINT->IntStatus ){
		mcu_core_pio0_isr();
	}
#endif

	if ( LPC_SC->EXTINT & (1<<3)){
		mcu_eint_isr(3);
	}
}

#endif

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
#include "mcu/debug.h"
#include "mcu/pio.h"
#include "mcu/core.h"
#include "mcu/debug.h"


#if MCU_PIO_PORTS > 0


typedef struct {
	mcu_event_handler_t handler;
	uint8_t ref_count;
} pio_local_t;

static pio_local_t m_pio0_local MCU_SYS_MEM;
static pio_local_t m_pio2_local MCU_SYS_MEM;

static int set_event(int port, int event, int pin);

void _mcu_pio_dev_power_on(int port){
	if ( port == 0 ){
		if ( m_pio0_local.ref_count == 0 ){
			m_pio0_local.handler.callback = NULL;
		}
		m_pio0_local.ref_count++;
	} else if ( port == 2 ){
		if ( m_pio2_local.ref_count == 0 ){
			m_pio2_local.handler.callback = NULL;
		}
		m_pio2_local.ref_count++;
	}
}

void _mcu_pio_dev_power_off(int port){

	if ( port == 0 ){
		if ( m_pio0_local.ref_count > 0 ){
			if ( m_pio0_local.ref_count == 1 ){
				m_pio0_local.handler.callback = NULL;
			}
			m_pio0_local.ref_count--;
		}
	} else if ( port == 2 ){
		if ( m_pio2_local.ref_count > 0 ){
			if ( m_pio2_local.ref_count == 1 ){
				m_pio2_local.handler.callback = NULL;
			}
			m_pio2_local.ref_count--;
		}
	}
}

int _mcu_pio_dev_powered_on(int port){
	if ( port == 0 ){
		if ( m_pio0_local.ref_count > 0 ){
			return 1;
		} else {
			return 0;
		}
	} else if ( port == 2 ){
		if ( m_pio2_local.ref_count > 0 ){
			return 1;
		} else {
			return 0;
		}
	} else {
		return 1;
	}
}

int set_event(int port, int event, int pin){

	if ( port == 0 ){

#ifdef __lpc17xx
		if ( (pin > 30) ||
				(pin == 12) ||
				(pin == 13) ||
				(pin == 14) ){
			errno = EINVAL;
			return -1 - offsetof(mcu_action_t, channel);
		}
#endif

		LPC_GPIOINT->IO0IntEnR &= ~(1<<pin);
		LPC_GPIOINT->IO0IntEnF &= ~(1<<pin);

		if( event & MCU_EVENT_FLAG_RISING ){
			LPC_GPIOINT->IO0IntEnR |= (1<<pin);
		}

		if( event & MCU_EVENT_FLAG_FALLING ){
			LPC_GPIOINT->IO0IntEnF |= (1<<pin);
		}


	} else if ( port == 2 ){

#ifdef __lpc17xx
		if ( (pin > 13) ){
			errno = EINVAL;
			return -1 - offsetof(mcu_action_t, channel);
		}
#endif

		LPC_GPIOINT->IO2IntEnR &= ~(1<<pin);
		LPC_GPIOINT->IO2IntEnF &= ~(1<<pin);

		if( event & MCU_EVENT_FLAG_RISING ){
			LPC_GPIOINT->IO2IntEnR |= (1<<pin);
		}

		if( event & MCU_EVENT_FLAG_FALLING ){
			LPC_GPIOINT->IO2IntEnF |= (1<<pin);
		}
	}
	return 0;
}


int _mcu_pio_dev_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	int port;
	mcu_action_t * action;
	port = cfg->port;

	if( wop->nbyte != sizeof(mcu_action_t) ){
		errno = EINVAL;
		return -1;
	}

	action = (mcu_action_t*)wop->buf;
	action->handler.callback = wop->handler.callback;
	action->handler.context = wop->handler.context;
	return mcu_pio_setaction(port, action);
}

int mcu_pio_setaction(int port, void * ctl){
	int err;
	mcu_action_t * action = (mcu_action_t*)ctl;

	if( action->handler.callback == 0 ){
		if( port == 0 ){
			mcu_execute_event_handler(&(m_pio0_local.handler), MCU_EVENT_FLAG_CANCELED, 0);
		} else if ( port == 2 ){
			mcu_execute_event_handler(&(m_pio2_local.handler), MCU_EVENT_FLAG_CANCELED, 0);
		}
	}

	err = set_event(port, action->o_events, action->channel);
	if ( err ){
		return err;
	}

	if ( port == 0 ){
		if( _mcu_cortexm_priv_validate_callback(action->handler.callback) < 0 ){
			return -1;
		}

		m_pio0_local.handler.callback = action->handler.callback;
		m_pio0_local.handler.context = action->handler.context;
		LPC_GPIOINT->IO0IntClr = -1; //clear pending interrupts
	} else if ( port == 2 ){
		if( _mcu_cortexm_priv_validate_callback(action->handler.callback) < 0 ){
			return -1;
		}

		m_pio2_local.handler.callback = action->handler.callback;
		m_pio2_local.handler.context = action->handler.context;
		LPC_GPIOINT->IO2IntClr = -1; //clear pending interrupts
	} else {
		errno = EINVAL;
		return -1;
	}


#ifdef LPCXX7X_8X
	//This is the interrupt for GPIO0 and GPIO2
	_mcu_cortexm_priv_enable_irq((void*)GPIO_IRQn);
	_mcu_cortexm_set_irq_prio(GPIO_IRQn, action->prio);
#else
	//This is the interrupt for GPIO0 and GPIO2 (shared with EINT3)
	_mcu_cortexm_priv_enable_irq((void*)EINT3_IRQn);
	_mcu_cortexm_set_irq_prio(EINT3_IRQn, action->prio);
#endif

	return 0;
}


int mcu_pio_getinfo(int port, void * ctl){
	//read the direction pin status
	errno = ENOTSUP;
	return -1;
}

int mcu_pio_setattr(int port, void * ctl){
	int i;
	int mode;
	pio_attr_t * attr;
	attr = ctl;
#ifdef LPCXX7X_8X
	__IO u32 * regs_iocon;
#else
	int shift;
	__IO u32 * regs_pin;
	__IO u32 * regs_od = &(LPC_PINCON->PINMODE_OD0);
#endif
	LPC_GPIO_Type * gpio_regs = (LPC_GPIO_Type *)LPC_GPIO0_BASE + port;

	if( attr->o_flags & PIO_FLAG_SET_INPUT ){
		gpio_regs->DIR &= ~(attr->o_pinmask);
	}

	if( attr->o_flags & (PIO_FLAG_SET_OUTPUT) ){
		//set output pins as output
		gpio_regs->DIR |= attr->o_pinmask;
	}

	if( attr->o_flags & PIO_FLAG_IS_DIRONLY ){
		return 0;
	}

	mode = 0;


#ifdef LPCXX7X_8X

	if( attr->o_flags & PIO_FLAG_IS_PULLUP ){
		mode = 2;
	} else if( attr->o_flags & PIO_FLAG_IS_REPEATER ){
		mode = 3;
	} else if( attr->o_flags & PIO_FLAG_IS_FLOAT ){
		mode = 0;
	} else if ( attr->o_flags & PIO_FLAG_IS_PULLDOWN ){
		mode = 1;
	}

	for(i = 0; i < 32; i++){
		if ( (1<<i) & (attr->o_pinmask) ){
			regs_iocon = (u32*)LPC_IOCON + port*32 + i;
			_mcu_core_set_pinsel_func(port, i, CORE_PERIPH_PIO, port); //set the pin to use GPIO

			if( attr->o_flags & PIO_FLAG_IS_OPENDRAIN ){
				*regs_iocon |= (1<<10);
			} else {
				*regs_iocon &= ~(1<<10);
			}

			if( attr->o_flags & PIO_FLAG_IS_HYSTERESIS ){
				*regs_iocon |= (1<<5);
			} else {
				*regs_iocon &= ~(1<<5);
			}

			if( (attr->o_flags & PIO_FLAG_IS_ANALOG) == 0 ){
				*regs_iocon |= (1<<7);
			} else {
				*regs_iocon &= ~(1<<7);
			}

			if( (attr->o_flags & (PIO_FLAG_IS_SPEED_BLAZING|PIO_FLAG_IS_SPEED_HIGH) ) != 0 ){
				*regs_iocon |= (1<<9);
			} else {
				*regs_iocon &= ~(1<<9);
			}

			*regs_iocon &= ~(0x03 << 3);
			*regs_iocon |= (mode << 3);

		}
	}

#else

	if( attr->o_flags & PIO_FLAG_IS_PULLUP ){
		mode = 0;
	} else if( attr->o_flags & PIO_FLAG_IS_REPEATER ){
		mode = 1;
	} else if( attr->o_flags & PIO_FLAG_IS_FLOAT ){
		mode = 2;
	} else if ( attr->o_flags & PIO_FLAG_IS_PULLDOWN ){
		mode = 3;
	}

	for(i = 0; i < 8*sizeof(pio_sample_t); i++){
		if ( (1<<i) & attr->o_pinmask ){

			_mcu_core_set_pinsel_func(port, i, CORE_PERIPH_PIO, 0); //set the pin to use GPIO

			if( attr->o_flags & PIO_FLAG_IS_OPENDRAIN ){
				regs_od[port] |= (1<<i);
			} else {
				regs_od[port] &= ~(1<<i);
			}

			regs_pin = &(&(LPC_PINCON->PINMODE0))[port*2 + i/16];
			shift = (i < 16 ? i*2 : ((i-16)*2));
			MCU_CLR_MASK( *regs_pin, (3<<shift) );
			MCU_SET_MASK( *regs_pin, (mode<<(shift) ) );
		}
	}
#endif

	return 0;
}

int mcu_pio_setmask(int port, void * ctl){
	LPC_GPIO_Type * gpio_regs = (LPC_GPIO_Type *)LPC_GPIO0_BASE;
	gpio_regs[port].SET=(int)ctl;
	return 0;
}

int mcu_pio_clrmask(int port, void * ctl){
	LPC_GPIO_Type * gpio_regs = (LPC_GPIO_Type *)LPC_GPIO0_BASE;
	gpio_regs[port].CLR=(int)ctl;
	return 0;
}

int mcu_pio_get(int port, void * ctl){
	LPC_GPIO_Type * gpio_regs = (LPC_GPIO_Type *)LPC_GPIO0_BASE;
	return gpio_regs[port].PIN;
}

int mcu_pio_set(int port, void * ctl){
	LPC_GPIO_Type * gpio_regs = (LPC_GPIO_Type *)LPC_GPIO0_BASE;
	gpio_regs[port].PIN=(int)ctl;
	return 0;
}

void exec_cancelled0(){
	mcu_execute_event_handler(&(m_pio0_local.handler), MCU_EVENT_FLAG_CANCELED, 0);
}

void exec_cancelled2(){
	mcu_execute_event_handler(&(m_pio2_local.handler), MCU_EVENT_FLAG_CANCELED, 0);
}

//On __lpc17xx The pio interrupts use the eint3 interrupt service routine -- this function should be called from there
void _mcu_core_pio0_isr(){
	pio_event_t ev;

	if ( LPC_GPIOINT->IntStatus & (1<<0) ){
		ev.status = 0;
		ev.rising = LPC_GPIOINT->IO0IntStatR;
		ev.falling = LPC_GPIOINT->IO0IntStatF;
		LPC_GPIOINT->IO0IntClr = ev.rising | ev.falling;
		mcu_execute_event_handler(&(m_pio0_local.handler), MCU_EVENT_FLAG_RISING | MCU_EVENT_FLAG_FALLING, &ev);
	}

	if ( LPC_GPIOINT->IntStatus & (1<<2) ){
		ev.status = 0;
		ev.rising = LPC_GPIOINT->IO2IntStatR;
		ev.falling = LPC_GPIOINT->IO2IntStatF;
		LPC_GPIOINT->IO2IntClr = ev.rising | ev.falling;
		mcu_execute_event_handler(&(m_pio2_local.handler), MCU_EVENT_FLAG_RISING | MCU_EVENT_FLAG_FALLING, &ev);
	}
}


#endif


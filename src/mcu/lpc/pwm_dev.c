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
#include <fcntl.h>
#include "mcu/cortexm.h"
#include "mcu/pwm.h"
#include "mcu/cortexm.h"
#include "mcu/core.h"
#include "mcu/debug.h"

#if MCU_PWM_PORTS > 0


#define READ_OVERFLOW (1<<0)
#define WRITE_OVERFLOW (1<<1)

static void update_pwm(int port, int chan, int duty);
static void exec_callback(int port, LPC_PWM_Type * regs, u32 o_events);

typedef struct MCU_PACK {
	const uint32_t * volatile duty;
	volatile int pwm_nbyte_len;
	uint8_t chan;
	uint8_t pin_assign;
	uint8_t enabled_channels;
	uint8_t ref_count;
	mcu_event_handler_t handler;
} pwm_local_t;

static pwm_local_t pwm_local[MCU_PWM_PORTS] MCU_SYS_MEM;

LPC_PWM_Type * const pwm_regs_table[MCU_PWM_PORTS] = MCU_PWM_REGS;
u8 const pwm_irqs[MCU_PWM_PORTS] = MCU_PWM_IRQS;

void _mcu_pwm_dev_power_on(int port){
	if ( pwm_local[port].ref_count == 0 ){
		switch(port){
#ifdef LPCXX7X_8X
		case 0:
			_mcu_lpc_core_enable_pwr(PCPWM0);
			_mcu_cortexm_priv_enable_irq((void*)PWM0_IRQn);
			break;
#endif
		case 1:
			_mcu_lpc_core_enable_pwr(PCPWM1);
			_mcu_cortexm_priv_enable_irq((void*)(u32)(pwm_irqs[port]));
			break;
		}

	}
	pwm_local[port].ref_count++;
}

void _mcu_pwm_dev_power_off(int port){
	if ( pwm_local[port].ref_count > 0 ){
		if ( pwm_local[port].ref_count == 1 ){
			switch(port){
#ifdef LPCXX7X_8X
			case 0:
				_mcu_cortexm_priv_disable_irq((void*)(PWM0_IRQn));
				_mcu_lpc_core_disable_pwr(PCPWM0);
				break;
#endif
			case 1:
				_mcu_cortexm_priv_disable_irq((void*)(u32)(pwm_irqs[port]));
				_mcu_lpc_core_disable_pwr(PCPWM1);
				break;

			}
		}
		pwm_local[port].ref_count--;
	}
}

int _mcu_pwm_dev_powered_on(int port){
	switch(port){
#ifdef LPCXX7X_8X
	case 0:
		return _mcu_lpc_core_pwr_enabled(PCPWM0);
#endif
	case 1:
		return _mcu_lpc_core_pwr_enabled(PCPWM1);
	}
	return 0;
}

int mcu_pwm_getinfo(int port, void * ctl){
	pwm_info_t * info = ctl;
	LPC_PWM_Type * regs = pwm_regs_table[port];

#ifdef __lpc17xx
	if( regs == 0 ){
		errno = ENODEV;
		return -1;
	}
#endif

	info->o_flags = PWM_FLAG_IS_ACTIVE_HIGH | PWM_FLAG_IS_ACTIVE_LOW;


	return 0;
}

int mcu_pwm_setattr(int port, void * ctl){
	//check the GPIO configuration
	int i;
	u32 tmp;
	u32 enabled_channels;
	pwm_attr_t * attr = ctl;
	LPC_PWM_Type * regs = pwm_regs_table[port];

#ifdef __lpc17xx
	if( regs == 0 ){
		errno = ENODEV;
		return -1;
	}
#endif

	if ( attr->freq == 0 ){
		errno = EINVAL;
		return -1 - offsetof(pwm_attr_t, freq);
	}

	//Configure the GPIO

	if( mcu_core_set_pin_assignment(attr->pin_assignment, PWM_PIN_ASSIGNMENT_COUNT, CORE_PERIPH_PWM, port) < 0 ){
		return -1;
	}

	enabled_channels = 0;
	for(i=0; i < PWM_PIN_ASSIGNMENT_COUNT; i++){
		if( mcu_is_port_valid(attr->pin_assignment[i].port) ){
//need a table to convert port/pin to channel


		}

	}



	tmp = mcu_board_config.core_periph_freq / attr->freq;
	if ( tmp > 0 ){
		tmp = tmp - 1;
	}

	regs->TCR = 0; //Disable the counter while the registers are being updated

	regs->PR = tmp;
	//Configure to reset on match0 in PWM Mode
	regs->MR0 = attr->top;
	regs->LER |= (1<<0);
	regs->MCR = (1<<1); //enable the reset
	regs->TCR = (1<<3)|(1<<0); //Enable the counter in PWM mode
	regs->PCR = (enabled_channels & 0x3F) << 9;


	return 0;
}

int mcu_pwm_setaction(int port, void * ctl){
	mcu_action_t * action = (mcu_action_t*)ctl;
	LPC_PWM_Type * regs = pwm_regs_table[port];
	if( action->handler.callback == 0 ){
		//cancel any ongoing operation
		if ( regs->MCR & (1<<0) ){ //If the interrupt is enabled--the pwm is busy
			exec_callback(port, regs, MCU_EVENT_FLAG_CANCELED);
		}
	}


	if( _mcu_cortexm_priv_validate_callback(action->handler.callback) < 0 ){
		return -1;
	}

	pwm_local[port].handler.callback = action->handler.callback;
	pwm_local[port].handler.context = action->handler.context;

	_mcu_cortexm_set_irq_prio(pwm_irqs[port], action->prio);

	//need to decode the event
	return 0;
}

int mcu_pwm_set(int port, void * ctl){
	mcu_channel_t * writep = ctl;
	LPC_PWM_Type * regs = pwm_regs_table[port];

#ifdef __lpc17xx
	if( regs == 0 ){
		errno = ENODEV;
		return -1;
	}
#endif

	if ( regs->MCR & (1<<0) ){ //If the interrupt is enabled--the pwm is busy
		//Device is busy and can't start a new write
		errno = EBUSY;
		return -1;
	}

	update_pwm(port, writep->loc, writep->value);
	return 0;
}


int _mcu_pwm_dev_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	int port = cfg->port;
	LPC_PWM_Type * regs = pwm_regs_table[port];

#ifdef __lpc17xx
	if( regs == 0 ){
		errno = ENODEV;
		return -1;
	}
#endif

	if ( regs->MCR & (1<<0) ){ //If the interrupt is enabled--the pwm is busy
		errno = EAGAIN;
		return -1;
	}

	pwm_local[port].pwm_nbyte_len = wop->nbyte >> 2;
	pwm_local[port].duty = (const uint32_t *)wop->buf;
	regs->MCR |= (1<<0); //enable the interrupt
	pwm_local[port].chan = wop->loc;

	if( _mcu_cortexm_priv_validate_callback(wop->handler.callback) < 0 ){
		return -1;
	}

	pwm_local[port].handler.callback = wop->handler.callback;
	pwm_local[port].handler.context = wop->handler.context;

	return 0;
}

void update_pwm(int port, int chan, int duty){
	LPC_PWM_Type * regs = pwm_regs_table[port];

	switch(chan){
	case 0:
		regs->MR1 = duty;
		break;
	case 1:
		regs->MR2 = duty;
		break;
	case 2:
		regs->MR3 = duty;
		break;
	case 3:
		regs->MR4 = duty;
		break;
	case 4:
		regs->MR5 = duty;
		break;
	case 5:
		regs->MR6 = duty;
		break;
	}

	regs->LER |= (1<<(chan+1));
}

void exec_callback(int port, LPC_PWM_Type * regs, u32 o_events){
	//stop updating the duty cycle
	pwm_local[port].duty = NULL;

	//Disable the interrupt
	regs->MCR = (1<<1); //leave the reset on, but disable the interrupt

	//call the event handler
	mcu_execute_event_handler(&(pwm_local[port].handler), o_events, 0);
}


static void _mcu_core_pwm_isr(int port){
	//Clear the interrupt flag
	LPC_PWM_Type * regs = pwm_regs_table[port];

	regs->IR |= (1<<0);

	if ( pwm_local[port].pwm_nbyte_len ){
		if ( pwm_local[port].duty != NULL ){
			update_pwm(port, pwm_local[port].chan, *pwm_local[port].duty++);
		}
		pwm_local[port].pwm_nbyte_len--;
	} else {
		exec_callback(port, regs, MCU_EVENT_FLAG_WRITE_COMPLETE);
	}
}

#ifdef LPCXX7X_8X
void _mcu_core_pwm0_isr(){
	_mcu_core_pwm_isr(0);
}
#endif

//This will execute when MR0 overflows
void _mcu_core_pwm1_isr(){
	_mcu_core_pwm_isr(1);
}

#endif


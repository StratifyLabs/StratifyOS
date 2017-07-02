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
static void exec_callback(int port, LPC_PWM_Type * regs, void * data);

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

int mcu_pwm_getattr(int port, void * ctl){
	pwm_attr_t * ctlp;
	ctlp = (pwm_attr_t*)ctl;
	LPC_PWM_Type * regs = pwm_regs_table[port];

#ifdef __lpc17xx
	if( regs == 0 ){
		errno = ENODEV;
		return -1;
	}
#endif

	ctlp->pin_assign = pwm_local[port].pin_assign;
	ctlp->enabled_channels = pwm_local[port].enabled_channels;
	ctlp->top = regs->MR0;
	ctlp->freq = mcu_board_config.core_periph_freq / ( regs->PR + 1 );
	ctlp->flags = PWM_ATTR_FLAGS_ACTIVE_HIGH;

	return 0;
}

int mcu_pwm_setattr(int port, void * ctl){
	//check the GPIO configuration
	uint32_t tmp;
	pwm_attr_t * ctl_ptr = (pwm_attr_t *)ctl;
	LPC_PWM_Type * regs = pwm_regs_table[port];

#ifdef __lpc17xx
	if( regs == 0 ){
		errno = ENODEV;
		return -1;
	}
#endif

	if ( ctl_ptr->enabled_channels & ~0x3F ){
		errno = EINVAL;
		return -1 - offsetof(pwm_attr_t, enabled_channels);
	}

	if ( ctl_ptr->freq == 0 ){
		errno = EINVAL;
		return -1 - offsetof(pwm_attr_t, freq);
	}

	//Configure the GPIO

	switch(port){
#ifdef LPCXX7X_8X

	case 0:
		if(1){
			switch( ctl_ptr->pin_assign ){
			case 0:
				if ( ctl_ptr->enabled_channels & (1<<0) ){
					_mcu_core_set_pinsel_func(1, 2, CORE_PERIPH_PWM, 0);
				}
				if ( ctl_ptr->enabled_channels & (1<<1) ){
					_mcu_core_set_pinsel_func(1, 3, CORE_PERIPH_PWM, 0);
				}
				if ( ctl_ptr->enabled_channels & (1<<2) ){
					_mcu_core_set_pinsel_func(1, 5, CORE_PERIPH_PWM, 0);
				}
				if ( ctl_ptr->enabled_channels & (1<<3) ){
					_mcu_core_set_pinsel_func(1, 6, CORE_PERIPH_PWM, 0);
				}
				if ( ctl_ptr->enabled_channels & (1<<4) ){
					_mcu_core_set_pinsel_func(1, 7, CORE_PERIPH_PWM, 0);
				}
				if ( ctl_ptr->enabled_channels & (1<<5) ){
					_mcu_core_set_pinsel_func(1, 11, CORE_PERIPH_PWM, 0);
				}
				break;
			case 1:
				if ( ctl_ptr->enabled_channels & (1<<0) ){
					_mcu_core_set_pinsel_func(3, 16, CORE_PERIPH_PWM, 0);
				}
				if ( ctl_ptr->enabled_channels & (1<<1) ){
					_mcu_core_set_pinsel_func(3, 17, CORE_PERIPH_PWM, 0);
				}
				if ( ctl_ptr->enabled_channels & (1<<2) ){
					_mcu_core_set_pinsel_func(3, 18, CORE_PERIPH_PWM, 0);
				}
				if ( ctl_ptr->enabled_channels & (1<<3) ){
					_mcu_core_set_pinsel_func(3, 19, CORE_PERIPH_PWM, 0);
				}
				if ( ctl_ptr->enabled_channels & (1<<4) ){
					_mcu_core_set_pinsel_func(3, 20, CORE_PERIPH_PWM, 0);
				}
				if ( ctl_ptr->enabled_channels & (1<<5) ){
					_mcu_core_set_pinsel_func(3, 21, CORE_PERIPH_PWM, 0);
				}
				break;
			}
		}
		break;
#endif
	case 1:
		if(1){
			switch( ctl_ptr->pin_assign ){
			case 0:
				if ( ctl_ptr->enabled_channels & (1<<0) ){
					_mcu_core_set_pinsel_func(1, 18, CORE_PERIPH_PWM, 1);
				}
				if ( ctl_ptr->enabled_channels & (1<<1) ){
					_mcu_core_set_pinsel_func(1, 20, CORE_PERIPH_PWM, 1);
				}
				if ( ctl_ptr->enabled_channels & (1<<2) ){
					_mcu_core_set_pinsel_func(1, 21, CORE_PERIPH_PWM, 1);
				}
				if ( ctl_ptr->enabled_channels & (1<<3) ){
					_mcu_core_set_pinsel_func(1, 23, CORE_PERIPH_PWM, 1);
				}
				if ( ctl_ptr->enabled_channels & (1<<4) ){
					_mcu_core_set_pinsel_func(1, 24, CORE_PERIPH_PWM, 1);
				}
				if ( ctl_ptr->enabled_channels & (1<<5) ){
					_mcu_core_set_pinsel_func(1, 26, CORE_PERIPH_PWM, 1);
				}
				break;
			case 1:
				if ( ctl_ptr->enabled_channels & (1<<0) ){
					_mcu_core_set_pinsel_func(2, 0, CORE_PERIPH_PWM, 1);
				}
				if ( ctl_ptr->enabled_channels & (1<<1) ){
					_mcu_core_set_pinsel_func(2, 1, CORE_PERIPH_PWM, 1);
				}
				if ( ctl_ptr->enabled_channels & (1<<2) ){
					_mcu_core_set_pinsel_func(2, 2, CORE_PERIPH_PWM, 1);
				}
				if ( ctl_ptr->enabled_channels & (1<<3) ){
					_mcu_core_set_pinsel_func(2, 3, CORE_PERIPH_PWM, 1);
				}
				if ( ctl_ptr->enabled_channels & (1<<4) ){
					_mcu_core_set_pinsel_func(2, 4, CORE_PERIPH_PWM, 1);
				}
				if ( ctl_ptr->enabled_channels & (1<<5) ){
					_mcu_core_set_pinsel_func(2, 5, CORE_PERIPH_PWM, 1);
				}
				break;
#ifdef LPCXX7X_8X
			case 2:
				if ( ctl_ptr->enabled_channels & (1<<0) ){
					_mcu_core_set_pinsel_func(3, 24, CORE_PERIPH_PWM, 1);
				}
				if ( ctl_ptr->enabled_channels & (1<<1) ){
					_mcu_core_set_pinsel_func(3, 25, CORE_PERIPH_PWM, 1);
				}
				if ( ctl_ptr->enabled_channels & (1<<2) ){
					_mcu_core_set_pinsel_func(3, 26, CORE_PERIPH_PWM, 1);
				}
				if ( ctl_ptr->enabled_channels & (1<<3) ){
					_mcu_core_set_pinsel_func(3, 27, CORE_PERIPH_PWM, 1);
				}
				if ( ctl_ptr->enabled_channels & (1<<4) ){
					_mcu_core_set_pinsel_func(3, 28, CORE_PERIPH_PWM, 1);
				}
				if ( ctl_ptr->enabled_channels & (1<<5) ){
					_mcu_core_set_pinsel_func(3, 29, CORE_PERIPH_PWM, 1);
				}
				break;
#endif
			case MCU_GPIO_CFG_USER:
				break;
			default:
				errno = EINVAL;
				return -1 - offsetof(pwm_attr_t, pin_assign);
			}
		}
	}


	tmp = mcu_board_config.core_periph_freq / ctl_ptr->freq;
	if ( tmp > 0 ){
		tmp = tmp - 1;
	}

	regs->TCR = 0; //Disable the counter while the registers are being updated

	regs->PR = tmp;
	//Configure to reset on match0 in PWM Mode
	regs->MR0 = ctl_ptr->top;
	regs->LER |= (1<<0);
	regs->MCR = (1<<1); //enable the reset
	regs->TCR = (1<<3)|(1<<0); //Enable the counter in PWM mode
	regs->PCR = (ctl_ptr->enabled_channels & 0x3F) << 9;

	pwm_local[port].pin_assign = ctl_ptr->pin_assign;
	pwm_local[port].enabled_channels = ctl_ptr->enabled_channels;

	return 0;
}

int mcu_pwm_setaction(int port, void * ctl){
	mcu_action_t * action = (mcu_action_t*)ctl;
	LPC_PWM_Type * regs = pwm_regs_table[port];
	if( action->callback == 0 ){
		//cancel any ongoing operation
		if ( regs->MCR & (1<<0) ){ //If the interrupt is enabled--the pwm is busy
			exec_callback(port, regs, MCU_EVENT_SET_CODE(MCU_EVENT_OP_CANCELLED));
		}
	}


	if( _mcu_cortexm_priv_validate_callback(action->callback) < 0 ){
		return -1;
	}

	pwm_local[port].handler.callback = action->callback;
	pwm_local[port].handler.context = action->context;

	_mcu_cortexm_set_irq_prio(pwm_irqs[port], action->prio);

	//need to decode the event
	return 0;
}

int mcu_pwm_set(int port, void * ctl){
	pwm_reqattr_t * writep = (pwm_reqattr_t*)ctl;
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

	update_pwm(port, writep->channel, writep->duty);
	return 0;
}


int _mcu_pwm_dev_write(const device_cfg_t * cfg, device_transfer_t * wop){
	int port = DEVICE_GET_PORT(cfg);
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

	if( _mcu_cortexm_priv_validate_callback(wop->callback) < 0 ){
		return -1;
	}

	pwm_local[port].handler.callback = wop->callback;
	pwm_local[port].handler.context = wop->context;

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

void exec_callback(int port, LPC_PWM_Type * regs, void * data){
	//stop updating the duty cycle
	pwm_local[port].duty = NULL;

	//Disable the interrupt
	regs->MCR = (1<<1); //leave the reset on, but disable the interrupt

	//call the event handler
	_mcu_cortexm_execute_event_handler(&(pwm_local[port].handler), data);
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
		exec_callback(port, regs, 0);
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


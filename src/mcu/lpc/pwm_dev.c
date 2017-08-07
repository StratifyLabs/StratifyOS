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
#include "cortexm/cortexm.h"
#include "mcu/pwm.h"
#include "cortexm/cortexm.h"
#include "mcu/core.h"
#include "mcu/debug.h"

#if MCU_PWM_PORTS > 0


#define READ_OVERFLOW (1<<0)
#define WRITE_OVERFLOW (1<<1)

static void update_pwm(int port, int chan, int duty);
static void exec_callback(int port, LPC_PWM_Type * regs, u32 o_events);
static void force_latch(LPC_PWM_Type * regs);

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

static void configure_pin(const mcu_pin_t * pin, void * arg){
	u32 * enabled_channels = arg;
	//set the bit in enabled channels based on the pin number
	if( pin->port == 1 ){
		switch(pin->pin){
		case 18: *enabled_channels |= (1<<0); return;
		case 20: *enabled_channels |= (1<<1); return;
		case 21: *enabled_channels |= (1<<2); return;
		case 23: *enabled_channels |= (1<<3); return;
		case 24: *enabled_channels |= (1<<4); return;
		case 26: *enabled_channels |= (1<<5); return;
		}
	} else if(pin->port == 2 ){
		*enabled_channels |= (1<<(pin->pin));
	}
#ifdef LPCXX7X_8X
	else if( pin->port == 3 ){
		*enabled_channels |= (1<<(pin->pin-24)); //pins 24 to 29 inclusive
	}
#endif

}

void mcu_pwm_dev_power_on(const devfs_handle_t * handle){
	int port = handle->port;
	if ( pwm_local[port].ref_count == 0 ){
		switch(port){
#ifdef LPCXX7X_8X
		case 0:
			mcu_lpc_core_enable_pwr(PCPWM0);
			cortexm_enable_irq((void*)PWM0_IRQn);
			break;
#endif
		case 1:
			mcu_lpc_core_enable_pwr(PCPWM1);
			cortexm_enable_irq((void*)(u32)(pwm_irqs[port]));
			break;
		}

	}
	pwm_local[port].ref_count++;
}

void mcu_pwm_dev_power_off(const devfs_handle_t * handle){
	int port = handle->port;
	if ( pwm_local[port].ref_count > 0 ){
		if ( pwm_local[port].ref_count == 1 ){
			switch(port){
#ifdef LPCXX7X_8X
			case 0:
				cortexm_disable_irq((void*)(PWM0_IRQn));
				mcu_lpc_core_disable_pwr(PCPWM0);
				break;
#endif
			case 1:
				cortexm_disable_irq((void*)(u32)(pwm_irqs[port]));
				mcu_lpc_core_disable_pwr(PCPWM1);
				break;
			}
		}
		pwm_local[port].ref_count--;
	}
}

int mcu_pwm_dev_is_powered(const devfs_handle_t * handle){
	int port = handle->port;
	switch(port){
#ifdef LPCXX7X_8X
	case 0:
		return mcu_lpc_core_pwr_enabled(PCPWM0);
#endif
	case 1:
		return mcu_lpc_core_pwr_enabled(PCPWM1);
	}
	return 0;
}

int mcu_pwm_getinfo(const devfs_handle_t * handle, void * ctl){
	pwm_info_t * info = ctl;

#ifdef __lpc17xx
	int port = handle->port;
	if( port == 0 ){
		errno = ENODEV;
		return -1;
	}
#endif

	info->o_flags = PWM_FLAG_IS_ACTIVE_HIGH | PWM_FLAG_CLEAR_CHANNELS | PWM_FLAG_SET_TIMER | PWM_FLAG_IS_ENABLED;

	return 0;
}

int mcu_pwm_setattr(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	//check the GPIO configuration
	u32 tmp;
	u32 enabled_channels;
	u32 freq;
	u32 o_flags;
	u32 pcr;
	LPC_PWM_Type * regs = pwm_regs_table[port];

	const pwm_attr_t * attr = mcu_select_attr(handle, ctl);
	if( attr == 0 ){ return -1; }

	o_flags = attr->o_flags;

#ifdef __lpc17xx
	if( regs == 0 ){
		errno = ENODEV;
		return -1;
	}
#endif

	freq = attr->freq;
	if ( attr->freq == 0 ){
		freq = 1000000;
	}

	//Configure the GPIO
	enabled_channels = 0;

	pcr = 0;
	if( o_flags & PWM_FLAG_CLEAR_CHANNELS ){
		pcr = regs->PCR;
	}

	regs->PCR = 0;

	if( o_flags & (PWM_FLAG_SET_TIMER | PWM_FLAG_SET_CHANNELS) ){
		if( mcu_set_pin_assignment(
				&(attr->pin_assignment),
				MCU_CONFIG_PIN_ASSIGNMENT(pwm_config_t, handle),
				MCU_PIN_ASSIGNMENT_COUNT(pwm_pin_assignment_t),
				CORE_PERIPH_PWM, port, configure_pin, &enabled_channels) < 0 ){
			return -1;
		}
		regs->PCR |= (((enabled_channels & 0x3F) << 9) | pcr);
	}

	if( o_flags & PWM_FLAG_SET_TIMER ){
		tmp = mcu_board_config.core_periph_freq / freq;
		if ( tmp > 0 ){
			tmp = tmp - 1;
		}

		regs->TCR = 0; //Disable the counter while the registers are being updated

		regs->PR = tmp;
		//Configure to reset on match0 in PWM Mode
		regs->MCR = (1<<1); //reset the counter then it matches MR0
		regs->CTCR = 0;


		regs->TC = regs->MR0 - 1; //force the counter to latch right now
		regs->MR0 = attr->period;
		regs->LER |= (1<<0);

		force_latch(regs);

		if( (o_flags & PWM_FLAG_IS_ENABLED) == 0 ){
			regs->TCR = 0; //disable the counter
		}

	}


	return 0;
}

void force_latch(LPC_PWM_Type * regs){
	if( regs->LER ){
		regs->TCR = 0; //Disable the counter while the registers are being updated
		regs->TC = regs->MR0; //force the counter to latch right now
		regs->TCR = (1<<3)|(1<<0); //Enable the counter in PWM mode
		while( regs->LER ){ //give the regs time to latch
			;
		}
	}
}

int mcu_pwm_setaction(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	mcu_action_t * action = (mcu_action_t*)ctl;
	LPC_PWM_Type * regs = pwm_regs_table[port];
	if( action->handler.callback == 0 ){
		//cancel any ongoing operation
		if ( regs->MCR & (1<<0) ){ //If the interrupt is enabled--the pwm is busy
			exec_callback(port, regs, MCU_EVENT_FLAG_CANCELED);
		}
	}


	if( cortexm_validate_callback(action->handler.callback) < 0 ){
		return -1;
	}

	pwm_local[port].handler.callback = action->handler.callback;
	pwm_local[port].handler.context = action->handler.context;

	cortexm_set_irq_prio(pwm_irqs[port], action->prio);

	//need to decode the event
	return 0;
}

int mcu_pwm_setchannel(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
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

int mcu_pwm_getchannel(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	mcu_channel_t * channel = ctl;
	LPC_PWM_Type * regs = pwm_regs_table[port];

	switch(channel->loc){
	case 0: channel->value = regs->MR0; break;
	case 1: channel->value = regs->MR1; break;
	case 2: channel->value = regs->MR2; break;
	case 3: channel->value = regs->MR3; break;
	case 4: channel->value = regs->MR4; break;
	case 5: channel->value = regs->MR5; break;
	case 6: channel->value = regs->MR6; break;
	default:
		errno = EINVAL;
		return -1;
	}

	return 0;
}
int mcu_pwm_set(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	u32 value = (u32)ctl;
	LPC_PWM_Type * regs = pwm_regs_table[port];
	regs->TC = value;
	return 0;
}

int mcu_pwm_get(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	LPC_PWM_Type * regs = pwm_regs_table[port];
	return regs->TC;
}

int mcu_pwm_enable(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	LPC_PWM_Type * regs = pwm_regs_table[port];
	regs->TCR = (1<<3)|(1<<0); //Enable the counter in PWM mode
	return 0;
}

int mcu_pwm_disable(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	LPC_PWM_Type * regs = pwm_regs_table[port];
	regs->TCR = 0; //Enable the counter in PWM mode
	return 0;
}


int mcu_pwm_dev_write(const devfs_handle_t * handle, devfs_async_t * wop){
	int port = handle->port;
	LPC_PWM_Type * regs = pwm_regs_table[port];

#ifdef __lpc17xx
	if( regs == 0 ){
		errno = ENODEV;
		return -1;
	}
#endif

	if ( pwm_local[port].handler.callback ){ //If the interrupt is enabled--the pwm is busy
		errno = EBUSY;
		return -1;
	}

	pwm_local[port].pwm_nbyte_len = wop->nbyte >> 2;
	pwm_local[port].duty = (const uint32_t *)wop->buf;
	regs->MCR |= (1<<0); //enable the interrupt
	pwm_local[port].chan = wop->loc;

	if( cortexm_validate_callback(wop->handler.callback) < 0 ){
		return -1;
	}

	pwm_local[port].handler.callback = wop->handler.callback;
	pwm_local[port].handler.context = wop->handler.context;

	return 0;
}

void update_pwm(int port, int chan, int duty){
	LPC_PWM_Type * regs = pwm_regs_table[port];

	switch(chan){
	case 1:
		regs->MR1 = duty;
		break;
	case 2:
		regs->MR2 = duty;
		break;
	case 3:
		regs->MR3 = duty;
		break;
	case 4:
		regs->MR4 = duty;
		break;
	case 5:
		regs->MR5 = duty;
		break;
	case 6:
		regs->MR6 = duty;
		break;
	default:
		return;
	}

	regs->LER |= (1<<(chan));
}

void exec_callback(int port, LPC_PWM_Type * regs, u32 o_events){
	//stop updating the duty cycle
	pwm_local[port].duty = NULL;

	//call the event handler
	mcu_execute_event_handler(&(pwm_local[port].handler), o_events, 0);

	if( pwm_local[port].handler.callback == 0 ){
		//Disable the interrupt
		regs->MCR = (1<<1); //leave the reset on, but disable the interrupt
	}
}


static void mcu_core_pwm_isr(int port){
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
void mcu_core_pwm0_isr(){
	mcu_core_pwm_isr(0);
}
#endif

//This will execute when MR0 overflows
void mcu_core_pwm1_isr(){
	mcu_core_pwm_isr(1);
}

#endif


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
#include <stdbool.h>
#include "cortexm/cortexm.h"
#include "mcu/tmr.h"
#include "mcu/core.h"

#define NUM_TMRS MCU_TMR_PORTS
#define NUM_OCS 4
#define NUM_ICS 2


#define MR0_FLAG (1<<0)
#define MR1_FLAG (1<<1)
#define MR2_FLAG (1<<2)
#define MR3_FLAG (1<<3)
#define CR0_FLAG (1<<4)
#define CR1_FLAG (1<<5)

#if MCU_TMR_PORTS > 0

static LPC_TIM_Type * const tmr_regs_table[NUM_TMRS] = MCU_TMR_REGS;
static u8 const tmr_irqs[MCU_TMR_PORTS] = MCU_TMR_IRQS;

struct tmr_cfg {
	u8 pin_assign;
	u8 enabled_oc_chans;
	u8 enabled_ic_chans;
};

typedef struct MCU_PACK {
	mcu_event_handler_t handler[NUM_OCS+NUM_ICS];
	//struct tmr_cfg cfg;
	u8 ref_count;
} tmr_local_t;

static tmr_local_t m_tmr_local[NUM_TMRS];

static void clear_actions(int port);

static int execute_handler(mcu_event_handler_t * handler, u32 o_events, u32 channel, u32 value);

void clear_actions(int port){
	memset(m_tmr_local[port].handler, 0, (NUM_OCS+NUM_ICS)*sizeof(mcu_event_handler_t));
}

void mcu_tmr_dev_power_on(const devfs_handle_t * handle){
	int port = handle->port;
	if ( m_tmr_local[port].ref_count == 0 ){
		clear_actions(port);
		switch(port){
		case 0:
			mcu_lpc_core_enable_pwr(PCTIM0);
			break;
		case 1:
			mcu_lpc_core_enable_pwr(PCTIM1);
			break;
		case 2:
			mcu_lpc_core_enable_pwr(PCTIM2);
			break;
		case 3:
			mcu_lpc_core_enable_pwr(PCTIM3);
			break;
		}
		cortexm_enable_irq((void*)(u32)(tmr_irqs[port]));
	}
	m_tmr_local[port].ref_count++;
}


void mcu_tmr_dev_power_off(const devfs_handle_t * handle){
	int port = handle->port;
	if ( m_tmr_local[port].ref_count > 0 ){
		if ( m_tmr_local[port].ref_count == 1 ){
			clear_actions(port);
			cortexm_disable_irq((void*)(u32)(tmr_irqs[port]));
			switch(port){
			case 0:
				mcu_lpc_core_disable_pwr(PCTIM0);
				break;
			case 1:
				mcu_lpc_core_disable_pwr(PCTIM1);
				break;
			case 2:
				mcu_lpc_core_disable_pwr(PCTIM2);
				break;
			case 3:
				mcu_lpc_core_disable_pwr(PCTIM3);
				break;
			}
		}
		m_tmr_local[port].ref_count--;
	}
}

int mcu_tmr_dev_is_powered(const devfs_handle_t * handle){
	int port = handle->port;
	return ( m_tmr_local[port].ref_count != 0);
}

int mcu_tmr_getinfo(const devfs_handle_t * handle, void * ctl){
	tmr_info_t * info = ctl;
	int port = handle->port;

	// set supported flags and events
	info->freq = mcu_board_config.core_periph_freq / (tmr_regs_table[port]->PR+1);


	return 0;
}

int mcu_tmr_setattr(const devfs_handle_t * handle, void * ctl){
	LPC_TIM_Type * regs;
	int port = handle->port;
	int ctcr = 0;

	const tmr_attr_t * attr = mcu_select_attr(handle, ctl);
	if( attr == 0 ){
		return -1;
	}

	u32 o_flags = attr->o_flags;
	int chan = attr->channel.loc;
	regs = tmr_regs_table[port];


	if( o_flags & TMR_FLAG_SET_TIMER ){

		if( (o_flags & (TMR_FLAG_IS_SOURCE_CPU|TMR_FLAG_IS_SOURCE_IC0|TMR_FLAG_IS_SOURCE_IC1)) ){

			if( o_flags & TMR_FLAG_IS_SOURCE_CPU ){
				if( attr->freq == 0 ){
					errno = EINVAL;
					return -1 - offsetof(tmr_attr_t, freq);
				}
			} else {
				ctcr = 1; //default on the rising edge
				if( o_flags & TMR_FLAG_IS_SOURCE_EDGEFALLING ){
					ctcr = 2;
				} else if( o_flags & TMR_FLAG_IS_SOURCE_EDGEBOTH ){
					ctcr = 3;
				}
			}

			if( mcu_set_pin_assignment(
					&(attr->pin_assignment),
					MCU_CONFIG_PIN_ASSIGNMENT(tmr_config_t, handle),
					MCU_PIN_ASSIGNMENT_COUNT(tmr_pin_assignment_t),
					CORE_PERIPH_TMR, port, 0, 0) < 0 ){
				return -1;
			}


			if( o_flags & TMR_FLAG_IS_SOURCE_IC1 ){
				ctcr |= (1<<2);
			}

			//Set the prescalar so that the freq is correct
			if ( (attr->freq < mcu_board_config.core_periph_freq) && (attr->freq != 0) ){
				tmr_regs_table[port]->PR = ((mcu_board_config.core_periph_freq + attr->freq/2) / attr->freq);
				if (tmr_regs_table[port]->PR != 0 ){
					tmr_regs_table[port]->PR -= 1;
				}
			} else {
				regs->PR = 0;
			}

			regs->CTCR = ctcr;
		}

	}


	if( o_flags & TMR_FLAG_SET_CHANNEL ){
		//Check for reset action
		if ( o_flags & TMR_FLAG_IS_CHANNEL_RESET_ON_MATCH){ //reset on match
			regs->MCR |= ((1<<1) << (chan*3) );
		}

		//Check to see if the timer should stop on a match
		if ( o_flags & TMR_FLAG_IS_CHANNEL_STOP_ON_MATCH){
			regs->MCR |= ((1<<2) << (chan*3) );
		}

		if( chan <= TMR_ACTION_CHANNEL_OC3 ){
			regs->EMR &= ~(0x3<<(chan+4));
			if( o_flags & TMR_FLAG_IS_CHANNEL_SET_OUTPUT_ON_MATCH ){
				//set OC output on event
				regs->EMR |= (0x2<<(chan+4));
			}

			if( o_flags & TMR_FLAG_IS_CHANNEL_CLEAR_OUTPUT_ON_MATCH ){
				//clr OC output on event
				regs->EMR |= (0x1<<(chan+4));
			}

			if( o_flags & TMR_FLAG_IS_CHANNEL_TOGGLE_OUTPUT_ON_MATCH ){
				//toggle OC output on event
				regs->EMR |= (0x3<<(chan+4));
			}
		}
	}

	return 0;
}

int mcu_tmr_enable(const devfs_handle_t * handle, void * ctl){
	LPC_TIM_Type * regs;
	int port = handle->port;
	regs = tmr_regs_table[port];
	regs->TCR = 1;
	return 0;
}

int mcu_tmr_disable(const devfs_handle_t * handle, void * ctl){
	LPC_TIM_Type * regs;
	int port = handle->port;
	regs = tmr_regs_table[port];
	regs->TCR = 0;
	return 0;
}

int mcu_tmr_setchannel(const devfs_handle_t * handle, void * ctl){
	LPC_TIM_Type * regs;
	int port = handle->port;
	regs = tmr_regs_table[port];
	//Write the output compare value
	mcu_channel_t * req = (mcu_channel_t*)ctl;
	u8 chan;

	if( req->loc & MCU_CHANNEL_FLAG_IS_INPUT ){
		chan = req->loc & ~MCU_CHANNEL_FLAG_IS_INPUT;
		if ( chan > 1 ){
			errno = EINVAL;
			return -1;
		}
#if MCU_TMR_API == 1
		regs->CR[chan] = req->value;
#else
		((uint32_t*)&(regs->CR0))[ chan ] = req->value;
#endif

	} else {

		if ( req->loc > 3 ){
			errno = EINVAL;
			return -1;
		}

#if MCU_TMR_API == 1
		regs->MR[req->loc] = req->value;
#else
		((uint32_t*)&(regs->MR0))[ req->loc ] = req->value;
#endif
	}
	return 0;
}

int mcu_tmr_getchannel(const devfs_handle_t * handle, void * ctl){
	LPC_TIM_Type * regs;
	int port = handle->port;
	regs = tmr_regs_table[port];
	//Read the output compare channel
	mcu_channel_t * req = (mcu_channel_t*)ctl;
	u8 chan;

	if( req->loc & MCU_CHANNEL_FLAG_IS_INPUT ){
		chan = req->loc & ~MCU_CHANNEL_FLAG_IS_INPUT;
		if ( chan > 1 ){
			errno = EINVAL;
			return -1;
		}
#if MCU_TMR_API == 1
		req->value = regs->CR[chan];
#else
		req->value = ((uint32_t*)&(regs->CR0))[ chan ];
#endif
	} else {
		if ( req->loc > 3 ){
			errno = EINVAL;
			return -1;
		}
#if MCU_TMR_API == 1
		req->value = regs->MR[req->loc];
#else
		req->value = ((uint32_t*)&(regs->MR0))[ req->loc ];
#endif
	}
	return 0;
}

int mcu_tmr_dev_write(const devfs_handle_t * handle, devfs_async_t * wop){
	errno = ENOTSUP;
	return -1;
}


int mcu_tmr_setaction(const devfs_handle_t * handle, void * ctl){
	mcu_action_t * action = (mcu_action_t*)ctl;
	LPC_TIM_Type * regs;
	int port = handle->port;
	regs = tmr_regs_table[port];
	u32 chan;
	u32 o_events;

	o_events = action->o_events;
	chan = action->channel;

	if ( o_events == MCU_EVENT_FLAG_NONE ){ //Check to see if all actions are disabled
		regs->MCR &= ~(0x03 << (chan*3) );
		m_tmr_local[port].handler[chan].callback = 0;
	} else if( o_events & MCU_EVENT_FLAG_MATCH ){

		if( action->handler.callback != 0 ){
			regs->MCR |= ((1<<0) << (chan*3)); //set the interrupt on match flag
		}  else {
			regs->MCR &= ~((1<<0) << (chan*3)); //disable the interrupt on match
		}

		m_tmr_local[port].handler[chan] = action->handler;
	}

	return 0;
}

int mcu_tmr_dev_read(const devfs_handle_t * handle, devfs_async_t * rop){
	errno = ENOTSUP;
	return -1;
}

int mcu_tmr_set(const devfs_handle_t * handle, void * ctl){
	LPC_TIM_Type * regs;
	int port = handle->port;
	regs = tmr_regs_table[port];
	regs->TC = (uint32_t)ctl;
	return 0;
}

int mcu_tmr_get(const devfs_handle_t * handle, void * ctl){
	LPC_TIM_Type * regs;
	int port = handle->port;
	regs = tmr_regs_table[port];
	return regs->TC;
}

static void tmr_isr(int port); //This is speed optimized
//void tmr_isr(int port); //This is size optimized

int execute_handler(mcu_event_handler_t * handler, u32 o_events, u32 channel, u32 value){
	tmr_event_t event;
	event.channel.loc = channel;
	event.channel.value = value;
	return mcu_execute_event_handler(handler, o_events, &event);
}

//Four timers with 4 OC's and 2 IC's each
void tmr_isr(int port){
	int flags;
	LPC_TIM_Type * regs;
	regs = tmr_regs_table[port];
	flags = (regs->IR & 0x3F);
	regs->IR = flags; //clear the flags
	//execute the callbacks
#if MCU_TMR_API == 1
	if( flags & MR0_FLAG ){
		execute_handler(&(m_tmr_local[port].handler[0]), MCU_EVENT_FLAG_MATCH, 0, regs->MR[0]);
	}
	if( flags & MR1_FLAG ){
		execute_handler(&(m_tmr_local[port].handler[0]), MCU_EVENT_FLAG_MATCH, 1, regs->MR[1]);
	}
	if( flags & MR2_FLAG ){
		execute_handler(&(m_tmr_local[port].handler[2]), MCU_EVENT_FLAG_MATCH, 2, regs->MR[2]);
	}
	if( flags & MR3_FLAG ){
		execute_handler(&(m_tmr_local[port].handler[3]), MCU_EVENT_FLAG_MATCH, 3, regs->MR[3]);
	}
	if( flags & CR0_FLAG ){
		execute_handler(&(m_tmr_local[port].handler[4]), MCU_EVENT_FLAG_MATCH, 4, regs->CR[0]);
	}
	if( flags & CR1_FLAG ){
		execute_handler(&(m_tmr_local[port].handler[5]), MCU_EVENT_FLAG_MATCH, 5, regs->CR[1]);
	}
#else
	if( flags & MR0_FLAG ){
		execute_handler(&(m_tmr_local[port].handler[0]), MCU_EVENT_FLAG_MATCH, 0, regs->MR0);
	}
	if( flags & MR1_FLAG ){
		execute_handler(&(m_tmr_local[port].handler[1]), MCU_EVENT_FLAG_MATCH, 1, regs->MR1);
	}
	if( flags & MR2_FLAG ){
		execute_handler(&(m_tmr_local[port].handler[2]), MCU_EVENT_FLAG_MATCH, 2, regs->MR2);
	}
	if( flags & MR3_FLAG ){
		execute_handler(&(m_tmr_local[port].handler[3]), MCU_EVENT_FLAG_MATCH, 3, regs->MR3);
	}
	if( flags & CR0_FLAG ){
		execute_handler(&(m_tmr_local[port].handler[4]), MCU_EVENT_FLAG_MATCH, 4, regs->CR0);
	}
	if( flags & CR1_FLAG ){
		execute_handler(&(m_tmr_local[port].handler[5]), MCU_EVENT_FLAG_MATCH, 5, regs->CR1);
	}
#endif
}

void mcu_core_tmr0_isr(){
	tmr_isr(0);
}

void mcu_core_tmr1_isr(){
	tmr_isr(1);
}

void mcu_core_tmr2_isr(){
	tmr_isr(2);
}

void mcu_core_tmr3_isr(){
	tmr_isr(3);
}

#endif


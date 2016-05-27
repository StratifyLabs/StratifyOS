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
	uint8_t pin_assign;
	uint8_t enabled_oc_chans;
	uint8_t enabled_ic_chans;
};

typedef struct MCU_PACK {
	mcu_event_handler_t handler[NUM_OCS+NUM_ICS];
	struct tmr_cfg cfg;
	uint8_t ref_count;
} tmr_local_t;

static tmr_local_t _mcu_tmr_local[NUM_TMRS];

static void _mcu_tmr_clear_actions(int port);

void _mcu_tmr_clear_actions(int port){
	memset(_mcu_tmr_local[port].handler, 0, (NUM_OCS+NUM_ICS)*sizeof(mcu_event_handler_t));
}

void _mcu_tmr_dev_power_on(int port){
	if ( _mcu_tmr_local[port].ref_count == 0 ){
		_mcu_tmr_clear_actions(port);
		switch(port){
		case 0:
			_mcu_lpc_core_enable_pwr(PCTIM0);
			break;
		case 1:
			_mcu_lpc_core_enable_pwr(PCTIM1);
			break;
		case 2:
			_mcu_lpc_core_enable_pwr(PCTIM2);
			break;
		case 3:
			_mcu_lpc_core_enable_pwr(PCTIM3);
			break;
		}
		_mcu_core_priv_enable_irq((void*)(u32)(tmr_irqs[port]));
	}
	_mcu_tmr_local[port].ref_count++;
}


void _mcu_tmr_dev_power_off(int port){
	if ( _mcu_tmr_local[port].ref_count > 0 ){
		if ( _mcu_tmr_local[port].ref_count == 1 ){
			_mcu_tmr_clear_actions(port);
			_mcu_core_priv_disable_irq((void*)(u32)(tmr_irqs[port]));
			switch(port){
			case 0:
				_mcu_lpc_core_disable_pwr(PCTIM0);
				break;
			case 1:
				_mcu_lpc_core_disable_pwr(PCTIM1);
				break;
			case 2:
				_mcu_lpc_core_disable_pwr(PCTIM2);
				break;
			case 3:
				_mcu_lpc_core_disable_pwr(PCTIM3);
				break;
			}
		}
		_mcu_tmr_local[port].ref_count--;
	}
}

int _mcu_tmr_dev_powered_on(int port){
	return ( _mcu_tmr_local[port].ref_count != 0);
}

int mcu_tmr_getattr(int port, void * ctl){
	uint32_t ctcr;
	tmr_attr_t * ctlp;
	ctlp = (tmr_attr_t *)ctl;

	ctcr = tmr_regs_table[port]->CTCR;

	//GPIO CFG
	ctlp->pin_assign = _mcu_tmr_local[port].cfg.pin_assign;


	//Clock source and counter mode
	if ( (ctcr & 0x03) == 0 ){
		ctlp->clksrc = TMR_CLKSRC_CPU;
	} else {

		if ( ((ctcr >> 2) & 0x01) == 0 ){
			ctlp->clksrc = TMR_CLKSRC_IC0;
		} else {
			ctlp->clksrc = TMR_CLKSRC_IC1;
		}

		if ( (ctcr & 0x03) == 1 ){
			ctlp->clksrc |= TMR_CLKSRC_EDGERISING;
		} else if ( (ctcr & 0x03) == 2 ){
			ctlp->clksrc |= TMR_CLKSRC_EDGEFALLING;
		} else if ( (ctcr & 0x03) == 3 ){
			ctlp->clksrc |= TMR_CLKSRC_EDGEBOTH;
		}
	}

	//Frequency
	ctlp->freq = mcu_board_config.core_periph_freq / (tmr_regs_table[port]->PR+1);

	//Enabled Channels
	ctlp->enabled_oc_chans = _mcu_tmr_local[port].cfg.enabled_oc_chans;
	ctlp->enabled_ic_chans = _mcu_tmr_local[port].cfg.enabled_ic_chans;
	return 0;
}

int mcu_tmr_setattr(int port, void * ctl){
	tmr_attr_t * ctlp;
	ctlp = (tmr_attr_t *)ctl;
	int ctcr;
	int clksrc;

	clksrc = ctlp->clksrc & TMR_CLKSRC_MASK;
	//Check for errors in the IOCTL settings
	if ( clksrc == TMR_CLKSRC_CPU ){
		//Check for a valid target frequency
		if ( ctlp->freq == 0 ){
			errno = EINVAL;
			return -1 - offsetof(tmr_attr_t, freq);
		}
	}

	//Check for a valid clock source
	if ( (clksrc != TMR_CLKSRC_CPU) &&
			(clksrc != TMR_CLKSRC_IC0) &&
			(clksrc != TMR_CLKSRC_IC1) ){
		errno = EINVAL;
		return -1 - offsetof(tmr_attr_t, clksrc);
	}

	//Check for a valid counter mode
	if ( clksrc != TMR_CLKSRC_CPU ){
		switch(ctlp->clksrc & TMR_CLKSRC_EDGEMASK){
		case TMR_CLKSRC_EDGERISING:
			ctcr = 1;
			break;
		case TMR_CLKSRC_EDGEFALLING:
			ctcr = 2;
			break;
		case TMR_CLKSRC_EDGEBOTH:
			ctcr = 3;
			break;
		default:
			errno = EINVAL;
			return -1 - offsetof(tmr_attr_t, clksrc);
		}
	} else {
		ctcr = 0;
	}

	//configure the GPIO for OC channels
	if ( ctlp->enabled_oc_chans != 0 ){
		if (ctlp->pin_assign == 0){
			switch(port){
			case 0:
				if ( ctlp->enabled_oc_chans & (1<<0) ){
					_mcu_core_set_pinsel_func(1, 28, CORE_PERIPH_TMR, 0);
				}
				if ( ctlp->enabled_oc_chans & (1<<1) ){
					_mcu_core_set_pinsel_func(1, 29, CORE_PERIPH_TMR, 0);
				}
				break;
			case 1:
				if ( ctlp->enabled_oc_chans & (1<<0) ){
					_mcu_core_set_pinsel_func(1, 22, CORE_PERIPH_TMR, 1);
				}
				if ( ctlp->enabled_oc_chans & (1<<1) ){
					_mcu_core_set_pinsel_func(1, 25, CORE_PERIPH_TMR, 1);
				}
				break;
			case 2:
				if ( ctlp->enabled_oc_chans & (1<<0) ){
					_mcu_core_set_pinsel_func(0, 6, CORE_PERIPH_TMR, 2);
				}
				if ( ctlp->enabled_oc_chans & (1<<1) ){
					_mcu_core_set_pinsel_func(0, 7, CORE_PERIPH_TMR, 2);
				}
				if ( ctlp->enabled_oc_chans & (1<<2) ){
					_mcu_core_set_pinsel_func(0, 8, CORE_PERIPH_TMR, 2);
				}
				if ( ctlp->enabled_oc_chans & (1<<3) ){
					_mcu_core_set_pinsel_func(0, 9, CORE_PERIPH_TMR, 2);
				}
				break;
			case 3:
				if ( ctlp->enabled_oc_chans & (1<<0) ){
					_mcu_core_set_pinsel_func(0, 10, CORE_PERIPH_TMR, 3);
				}
				if ( ctlp->enabled_oc_chans & (1<<1) ){
					_mcu_core_set_pinsel_func(0, 11, CORE_PERIPH_TMR, 3);
				}
				break;
			}
		} else if (ctlp->pin_assign == 1){
			switch(port){
			case 0:
				if ( ctlp->enabled_oc_chans & (1<<0) ){
					_mcu_core_set_pinsel_func(3, 25, CORE_PERIPH_TMR, 0);
				}
				if ( ctlp->enabled_oc_chans & (1<<1) ){
					_mcu_core_set_pinsel_func(3, 26, CORE_PERIPH_TMR, 0);
				}
				break;
			case 1:
				errno = EINVAL;
				return -1 - offsetof(tmr_attr_t, pin_assign);
			case 2:
				if ( ctlp->enabled_oc_chans & (1<<0) ){
					_mcu_core_set_pinsel_func(4, 28, CORE_PERIPH_TMR, 2);
				}
				if ( ctlp->enabled_oc_chans & (1<<1) ){
					_mcu_core_set_pinsel_func(4, 29, CORE_PERIPH_TMR, 2);
				}
				break;
			case 3:
				errno = EINVAL;
				return -1 - offsetof(tmr_attr_t, pin_assign);
			}
		} else if ( ctlp->pin_assign != MCU_GPIO_CFG_USER ){
			errno = EINVAL;
			return -1 - offsetof(tmr_attr_t, pin_assign);
		}
	}

	//configure the GPIO for IC channels
	if ( ctlp->enabled_ic_chans != 0 ){
		//Each timer has two channels that are connected to external IO
		if (ctlp->pin_assign == 0){
			switch(port){
			case 0:
				if ( ctlp->enabled_ic_chans & (1<<0) ){
					_mcu_core_set_pinsel_func(1, 26, CORE_PERIPH_TMR, 0);
				}
				if ( ctlp->enabled_ic_chans & (1<<1) ){
					_mcu_core_set_pinsel_func(1, 27, CORE_PERIPH_TMR, 0);
				}
				break;
			case 1:
				if ( ctlp->enabled_ic_chans & (1<<0) ){
					_mcu_core_set_pinsel_func(1, 18, CORE_PERIPH_TMR, 1);
				}
				if ( ctlp->enabled_ic_chans & (1<<1) ){
					_mcu_core_set_pinsel_func(1, 19, CORE_PERIPH_TMR, 1);
				}
				break;
			case 2:
				if ( ctlp->enabled_ic_chans & (1<<0) ){
					_mcu_core_set_pinsel_func(0, 4, CORE_PERIPH_TMR, 2);
				}
				if ( ctlp->enabled_ic_chans & (1<<1) ){
					_mcu_core_set_pinsel_func(0, 5, CORE_PERIPH_TMR, 2);
				}
				break;
			case 3:
				if ( ctlp->enabled_ic_chans & (1<<0) ){
					_mcu_core_set_pinsel_func(0, 23, CORE_PERIPH_TMR, 3);
				}
				if ( ctlp->enabled_ic_chans & (1<<1) ){
					_mcu_core_set_pinsel_func(0, 24, CORE_PERIPH_TMR, 3);
				}
				break;

			}
		} else if ( ctlp->pin_assign != MCU_GPIO_CFG_USER ){
			errno = EINVAL;
			return -1 - offsetof(tmr_attr_t, pin_assign);
		}
	}

	//Configure the GPIO for counter mode
	if ( clksrc == TMR_CLKSRC_IC0){
		switch(port){
		case 0:
			_mcu_core_set_pinsel_func(1, 26, CORE_PERIPH_TMR, 0);
			break;
		case 1:
			_mcu_core_set_pinsel_func(1, 18, CORE_PERIPH_TMR, 1);
			break;
		case 2:
			_mcu_core_set_pinsel_func(0, 4, CORE_PERIPH_TMR, 2);
			break;
		case 3:
			_mcu_core_set_pinsel_func(0, 23, CORE_PERIPH_TMR, 3);
			break;
		}
	} else if ( clksrc == TMR_CLKSRC_IC1 ){
		ctcr |= (1<<2);
		switch(port){
		case 0:
			_mcu_core_set_pinsel_func(1, 27, CORE_PERIPH_TMR, 0);
			break;
		case 1:
			_mcu_core_set_pinsel_func(1, 19, CORE_PERIPH_TMR, 1);
			break;
		case 2:
			_mcu_core_set_pinsel_func(0, 5, CORE_PERIPH_TMR, 2);
			break;
		case 3:
			_mcu_core_set_pinsel_func(0, 24, CORE_PERIPH_TMR, 3);
			break;
		}
	}


	//Set the prescalar so that the freq is correct
	if ( (ctlp->freq < mcu_board_config.core_periph_freq) && (ctlp->freq != 0) ){
		tmr_regs_table[port]->PR = ((mcu_board_config.core_periph_freq + ctlp->freq/2) / ctlp->freq);
		if (tmr_regs_table[port]->PR != 0 ){
			tmr_regs_table[port]->PR -= 1;
		}
	} else {
		tmr_regs_table[port]->PR = 0;
	}

	tmr_regs_table[port]->CTCR = ctcr;

	_mcu_tmr_local[port].cfg.enabled_oc_chans = ctlp->enabled_oc_chans;
	_mcu_tmr_local[port].cfg.enabled_ic_chans = ctlp->enabled_ic_chans;
	_mcu_tmr_local[port].cfg.pin_assign = ctlp->pin_assign;
	return 0;
}

int mcu_tmr_on(int port, void * ctl){
	LPC_TIM_Type * regs;
	regs = tmr_regs_table[port];
	regs->TCR = 1;
	return 0;
}

int mcu_tmr_off(int port, void * ctl){
	LPC_TIM_Type * regs;
	regs = tmr_regs_table[port];
	regs->TCR = 0;
	return 0;
}

int mcu_tmr_setoc(int port, void * ctl){
	LPC_TIM_Type * regs;
	regs = tmr_regs_table[port];
	//Write the output compare value
	tmr_reqattr_t * req = (tmr_reqattr_t*)ctl;
	if ( req->channel > 3 ){
		errno = EINVAL;
		return -1;
	}

#if MCU_TMR_API == 1
	regs->MR[req->channel] = req->value;
#else
	((uint32_t*)&(regs->MR0))[ req->channel ] = req->value;
#endif
	return 0;
}

int mcu_tmr_getoc(int port, void * ctl){
	LPC_TIM_Type * regs;
	regs = tmr_regs_table[port];
	//Read the output compare channel
	tmr_reqattr_t * req = (tmr_reqattr_t*)ctl;
	if ( req->channel > 3 ){
		errno = EINVAL;
		return -1;
	}
#if MCU_TMR_API == 1
	req->value = regs->MR[req->channel];
#else
	req->value = ((uint32_t*)&(regs->MR0))[ req->channel ];
#endif
	return 0;
}

int mcu_tmr_setic(int port, void * ctl){
	LPC_TIM_Type * regs;
	regs = tmr_regs_table[port];
	unsigned int chan;
	tmr_reqattr_t * req = (tmr_reqattr_t*)ctl;
	chan = req->channel - TMR_ACTION_CHANNEL_IC0;
	if ( chan > 1 ){
		errno = EINVAL;
		return -1;
	}
#if MCU_TMR_API == 1
	regs->CR[chan] = req->value;
#else
	((uint32_t*)&(regs->CR0))[ req->channel ] = req->value;
#endif
	return 0;
}

int mcu_tmr_getic(int port, void * ctl){
	LPC_TIM_Type * regs;
	unsigned int chan;
	regs = tmr_regs_table[port];
	tmr_reqattr_t * req = (tmr_reqattr_t*)ctl;
	chan = req->channel - TMR_ACTION_CHANNEL_IC0;
	if ( chan > 1 ){
		errno = EINVAL;
		return -1;
	}
#if MCU_TMR_API == 1
	req->value = regs->CR[chan];
#else
	req->value = ((uint32_t*)&(regs->CR0))[ chan ];
#endif
	return 0;
}

int _mcu_tmr_dev_write(const device_cfg_t * cfg, device_transfer_t * wop){
	int port;
	mcu_action_t * action;
	int chan;
	action = (mcu_action_t*)wop->buf;

	if( wop->nbyte != sizeof(mcu_action_t) ){
		errno = EINVAL;
		return -1;
	}

	port = cfg->periph.port;
	chan = action->channel;
	if ( _mcu_tmr_local[port].handler[chan].callback != 0 ){
		//The interrupt is on -- port is busy
		errno = EAGAIN;
		return -1;
	}

	action = (mcu_action_t*)wop->buf;
	action->callback = wop->callback;
	action->context = wop->context;

	_mcu_core_setirqprio(tmr_irqs[port], action->prio);

	return mcu_tmr_setaction(port, action);
}


int mcu_tmr_setaction(int port, void * ctl){
	tmr_action_t * action = (tmr_action_t*)ctl;
	LPC_TIM_Type * regs;
	regs = tmr_regs_table[port];
	int chan;
	int event;

	event = action->event;
	chan = action->channel;

	if ( event == TMR_ACTION_EVENT_NONE ){ //Check to see if all actions are disabled
		regs->MCR &= ~(0x03 << (chan*3) );
		_mcu_tmr_local[port].handler[chan].callback = NULL;
	} else {


		//Check for an interrupt action with a callback
		if ( event & TMR_ACTION_EVENT_INTERRUPT){
			regs->MCR |= ((1<<0) << (chan*3) );

			_mcu_tmr_local[port].handler[chan].callback = action->callback;
			_mcu_tmr_local[port].handler[chan].context = action->context;

		}

		//Check for reset action
		if ( event & TMR_ACTION_EVENT_RESET){
			regs->MCR |= ((1<<1) << (chan*3) );
		}

		//Check to see if the timer should stop on a match
		if ( event & TMR_ACTION_EVENT_STOP){
			regs->MCR |= ((1<<2) << (chan*3) );
		}

		if( chan <= TMR_ACTION_CHANNEL_OC3 ){
			regs->EMR &= ~(0x3<<(chan+4));
			if( event & TMR_ACTION_EVENT_SETOC ){
				//set OC output on event
				regs->EMR |= (0x2<<(chan+4));
			}

			if( event & TMR_ACTION_EVENT_CLROC ){
				//clr OC output on event
				regs->EMR |= (0x1<<(chan+4));
			}

			if( event & TMR_ACTION_EVENT_TOGGLEOC ){
				//toggle OC output on event
				regs->EMR |= (0x3<<(chan+4));
			}
		}
	}

	return 0;
}

int _mcu_tmr_dev_read(const device_cfg_t * cfg, device_transfer_t * rop){
	int port = DEVICE_GET_PORT(cfg);
	int chan = rop->loc;

	if( _mcu_core_priv_validate_callback(rop->callback) < 0 ){
		return -1;
	}

	_mcu_tmr_local[port].handler[chan].callback = rop->callback;
	_mcu_tmr_local[port].handler[chan].context = rop->context;
	return 0;
}

int mcu_tmr_set(int port, void * ctl){
	LPC_TIM_Type * regs;
	regs = tmr_regs_table[port];
	regs->TC = (uint32_t)ctl;
	return 0;
}

int mcu_tmr_get(int port, void * ctl){
	LPC_TIM_Type * regs;
	regs = tmr_regs_table[port];
	return regs->TC;
}

static void tmr_isr(int port); //This is speed optimized
//void tmr_isr(int port); //This is size optimized

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
		_mcu_core_exec_event_handler(&(_mcu_tmr_local[port].handler[0]), (mcu_event_t)regs->MR[0]);
	}
	if( flags & MR1_FLAG ){
		_mcu_core_exec_event_handler(&(_mcu_tmr_local[port].handler[1]), (mcu_event_t)regs->MR[1]);
	}
	if( flags & MR2_FLAG ){
		_mcu_core_exec_event_handler(&(_mcu_tmr_local[port].handler[2]), (mcu_event_t)regs->MR[2]);
	}
	if( flags & MR3_FLAG ){
		_mcu_core_exec_event_handler(&(_mcu_tmr_local[port].handler[3]), (mcu_event_t)regs->MR[3]);
	}
	if( flags & CR0_FLAG ){
		_mcu_core_exec_event_handler(&(_mcu_tmr_local[port].handler[4]), (mcu_event_t)regs->CR[0]);
	}
	if( flags & CR1_FLAG ){
		_mcu_core_exec_event_handler(&(_mcu_tmr_local[port].handler[5]), (mcu_event_t)regs->CR[1]);
	}
#else
	if( flags & MR0_FLAG ){
		_mcu_core_exec_event_handler(&(_mcu_tmr_local[port].handler[0]), (mcu_event_t)regs->MR0);
	}
	if( flags & MR1_FLAG ){
		_mcu_core_exec_event_handler(&(_mcu_tmr_local[port].handler[1]), (mcu_event_t)regs->MR1);
	}
	if( flags & MR2_FLAG ){
		_mcu_core_exec_event_handler(&(_mcu_tmr_local[port].handler[2]), (mcu_event_t)regs->MR2);
	}
	if( flags & MR3_FLAG ){
		_mcu_core_exec_event_handler(&(_mcu_tmr_local[port].handler[3]), (mcu_event_t)regs->MR3);
	}
	if( flags & CR0_FLAG ){
		_mcu_core_exec_event_handler(&(_mcu_tmr_local[port].handler[4]), (mcu_event_t)regs->CR0);
	}
	if( flags & CR1_FLAG ){
		_mcu_core_exec_event_handler(&(_mcu_tmr_local[port].handler[5]), (mcu_event_t)regs->CR1);
	}
#endif
}

void _mcu_core_tmr0_isr(){
	tmr_isr(0);
}

void _mcu_core_tmr1_isr(){
	tmr_isr(1);
}

void _mcu_core_tmr2_isr(){
	tmr_isr(2);
}

void _mcu_core_tmr3_isr(){
	tmr_isr(3);
}

#endif


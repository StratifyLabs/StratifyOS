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
#include "mcu/tmr.h"
#include "mcu/core.h"

#define NUM_OCS 4
#define NUM_ICS 2

#define MR0_FLAG (1<<0)
#define MR1_FLAG (1<<1)
#define MR2_FLAG (1<<2)
#define MR3_FLAG (1<<3)
#define CR0_FLAG (1<<4)
#define CR1_FLAG (1<<5)

LPC_TMR_Type * const tmr_regs_table[MCU_TMR_PORTS] = MCU_TMR_REGS;

struct tmr_cfg {
	uint8_t pin_assign;
	uint8_t enabled_oc_chans;
	uint8_t enabled_ic_chans;
};

typedef struct {
	mcu_callback_t callback[NUM_OCS+NUM_ICS];
	void * context[NUM_OCS+NUM_ICS];
	struct tmr_cfg cfg;
} tmr_local_t;

static tmr_local_t _mcu_tmr_local[MCU_TMR_PORTS];

static void clear_actions(int port);
static void input_pin_assign(int port, tmr_attr_t * ctlp);

void clear_actions(int port){
	int i;
	for(i=0; i < NUM_OCS+NUM_ICS; i++){
		_mcu_tmr_local[port].callback[i] = NULL;
	}
}

int _mcu_tmr_dev_port_is_invalid(int port){
	if ( (uint32_t)port > 3 ){
		return 1;
	}
	return 0;
}

void _mcu_tmr_dev_power_on(int port){

	clear_actions(port);

	switch(port){
	case 0:
		_mcu_lpc_core_enable_pwr(PCCT16B0);
		break;
	case 1:
		_mcu_lpc_core_enable_pwr(PCCT16B1);
		break;
	case 2:
		_mcu_lpc_core_enable_pwr(PCCT32B0);
		break;
	case 3:
		_mcu_lpc_core_enable_pwr(PCCT32B1);
		break;
	}

	_mcu_core_priv_enable_irq((void*)TIMER_16_0_IRQn + port);
}


void _mcu_tmr_dev_power_off(int port){

	_mcu_core_priv_disable_irq((void*)TIMER_16_0_IRQn + port);

	switch(port){
	case 0:
		_mcu_lpc_core_disable_pwr(PCCT16B0);
		break;
	case 1:
		_mcu_lpc_core_disable_pwr(PCCT16B1);
		break;
	case 2:
		_mcu_lpc_core_disable_pwr(PCCT32B0);
		break;
	case 3:
		_mcu_lpc_core_disable_pwr(PCCT32B1);
		break;
	}
}

int _mcu_tmr_dev_powered_on(int port){
	switch(port){
	case 0:
		if ( _mcu_lpc_core_pwr_enabled(PCCT16B0) ) {
			return 1;
		} else {
			return 0;
		}
		break;
	case 1:
		if ( _mcu_lpc_core_pwr_enabled(PCCT16B1) ) {
			return 1;
		} else {
			return 0;
		}
		break;
	case 2:
		if ( _mcu_lpc_core_pwr_enabled(PCCT32B0) ) {
			return 1;
		} else {
			return 0;
		}
		break;
	case 3:
		if ( _mcu_lpc_core_pwr_enabled(PCCT32B1) ) {
			return 1;
		} else {
			return 0;
		}
		break;
	}
	return -1;
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
	ctlp->freq = _mcu_core_getclock() / (tmr_regs_table[port]->PR+1);

	//Enabled Channels
	ctlp->enabled_oc_chans = _mcu_tmr_local[port].cfg.enabled_oc_chans;
	ctlp->enabled_ic_chans = _mcu_tmr_local[port].cfg.enabled_ic_chans;
	return 0;
}

void input_pin_assign(int port, tmr_attr_t * ctlp){
	switch(port){
	case 0:
		if ( ctlp->enabled_ic_chans & (1<<0) ){
			_mcu_core_set_pinsel_func(0, 2, CORE_PERIPH_TMR, 0);
		}
		break;
	case 1:
		if ( ctlp->enabled_ic_chans & (1<<0) ){
			_mcu_core_set_pinsel_func(1, 8, CORE_PERIPH_TMR, 1);
		}
		break;
	case 2:
		if ( ctlp->enabled_ic_chans & (1<<0) ){
			_mcu_core_set_pinsel_func(1, 5, CORE_PERIPH_TMR, 2);
		}
		break;
	case 3:
		if ( ctlp->enabled_ic_chans & (1<<0) ){
			_mcu_core_set_pinsel_func(1, 0, CORE_PERIPH_TMR, 3);
		}
		break;
	}
}


int mcu_tmr_setattr(int port, void * ctl){
	tmr_attr_t * ctlp;
	ctlp = (tmr_attr_t *)ctl;
	int ctcr;

	//Check for errors in the IOCTL settings
	if ( ctlp->clksrc == TMR_CLKSRC_CPU ){
		//Check for a valid target frequency
		if ( ctlp->freq == 0 ){
			errno = EINVAL;
			return -1;
		}
	}

	//Check for a valid clock source
	if ( (ctlp->clksrc != TMR_CLKSRC_CPU) &&
			(ctlp->clksrc != TMR_CLKSRC_IC0) &&
			(ctlp->clksrc != TMR_CLKSRC_IC1) ){
		errno = EINVAL;
		return -1;
	}

	//Check for a valid counter mode
	if ( ctlp->clksrc != TMR_CLKSRC_CPU ){
		switch(ctlp->counter_mode & TMR_CLKSRC_EDGEMASK){
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
			return -1;
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
					_mcu_core_set_pinsel_func(0, 8, CORE_PERIPH_TMR, 0);
				}
				if ( ctlp->enabled_oc_chans & (1<<1) ){
					_mcu_core_set_pinsel_func(0, 9, CORE_PERIPH_TMR, 0);
				}
				break;
			case 1:
				if ( ctlp->enabled_oc_chans & (1<<0) ){
					_mcu_core_set_pinsel_func(1, 9, CORE_PERIPH_TMR, 1);
				}
				if ( ctlp->enabled_oc_chans & (1<<1) ){
					_mcu_core_set_pinsel_func(1, 10, CORE_PERIPH_TMR, 1);
				}
				if ( ctlp->enabled_oc_chans & (1<<2) ){
					_mcu_core_set_pinsel_func(0, 10, CORE_PERIPH_TMR, 1);
				}
				break;
			case 2:
				if ( ctlp->enabled_oc_chans & (1<<0) ){
					_mcu_core_set_pinsel_func(1, 6, CORE_PERIPH_TMR, 2);
				}
				if ( ctlp->enabled_oc_chans & (1<<1) ){
					_mcu_core_set_pinsel_func(1, 7, CORE_PERIPH_TMR, 2);
				}
				if ( ctlp->enabled_oc_chans & (1<<2) ){
					_mcu_core_set_pinsel_func(0, 1, CORE_PERIPH_TMR, 2);
				}
				if ( ctlp->enabled_oc_chans & (1<<3) ){
					_mcu_core_set_pinsel_func(0, 11, CORE_PERIPH_TMR, 2);
				}
				break;
			case 3:
				if ( ctlp->enabled_oc_chans & (1<<0) ){
					_mcu_core_set_pinsel_func(1, 1, CORE_PERIPH_TMR, 3);
				}
				if ( ctlp->enabled_oc_chans & (1<<1) ){
					_mcu_core_set_pinsel_func(1, 2, CORE_PERIPH_TMR, 3);
				}
				if ( ctlp->enabled_oc_chans & (1<<2) ){
					_mcu_core_set_pinsel_func(1, 3, CORE_PERIPH_TMR, 3);
				}
				if ( ctlp->enabled_oc_chans & (1<<3) ){
					_mcu_core_set_pinsel_func(1, 4, CORE_PERIPH_TMR, 3);
				}
				break;
			}
		} else if ( ctlp->pin_assign != MCU_GPIO_CFG_USER ){
			errno = EINVAL;
			return -1;
		}
	}

	//configure the GPIO for IC channels
	if ( ctlp->enabled_ic_chans != 0 ){
		//Each timer has two channels that are connected to external IO
		if (ctlp->pin_assign == 0){
			input_pin_assign(port, ctlp);
		} else if ( ctlp->pin_assign != MCU_GPIO_CFG_USER ){
			errno = EINVAL;
			return -1;
		}
	}


	//Configure the GPIO for counter mode
	if ( ctlp->clksrc == TMR_CLKSRC_IC0){
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
	} else if ( ctlp->clksrc == TMR_CLKSRC_IC1 ){
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
	if ( (ctlp->freq < _mcu_core_getclock()) && (ctlp->freq != 0) ){
		tmr_regs_table[port]->PR = ((_mcu_core_getclock() + ctlp->freq/2) / ctlp->freq);
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
	LPC_TMR_Type * regs;
	regs = tmr_regs_table[port];
	regs->TCR = 1;
	return 0;
}

int mcu_tmr_off(int port, void * ctl){
	LPC_TMR_Type * regs;
	regs = tmr_regs_table[port];
	regs->TCR = 0;
	return 0;
}

int mcu_tmr_setoc(int port, void * ctl){
	LPC_TMR_Type * regs;
	regs = tmr_regs_table[port];
	//Write the output compare value
	tmr_reqattr_t * req = (tmr_reqattr_t*)ctl;
	if ( req->channel > 3 ){
		errno = EINVAL;
		return -1;
	}
	((uint32_t*)&(regs->MR0))[ req->channel ] = req->value;
	return 0;
}

int mcu_tmr_getoc(int port, void * ctl){
	LPC_TMR_Type * regs;
	regs = tmr_regs_table[port];
	//Read the output compare channel
	tmr_reqattr_t * req = (tmr_reqattr_t*)ctl;
	if ( req->channel > 3 ){
		errno = EINVAL;
		return -1;
	}
	req->value = ((uint32_t*)&(regs->MR0))[ req->channel ];
	return 0;
}

int mcu_tmr_setic(int port, void * ctl){
	LPC_TMR_Type * regs;
	regs = tmr_regs_table[port];
	unsigned int chan;
	tmr_reqattr_t * req = (tmr_reqattr_t*)ctl;
	chan = req->channel - TMR_ACTION_CHANNEL_IC0;
	if ( chan > 1 ){
		errno = EINVAL;
		return -1;
	}
	((uint32_t*)&(regs->CR0))[ req->channel ] = req->value;
	return 0;
}

int mcu_tmr_getic(int port, void * ctl){
	LPC_TMR_Type * regs;
	unsigned int chan;
	regs = tmr_regs_table[port];
	tmr_reqattr_t * req = (tmr_reqattr_t*)ctl;
	chan = req->channel - TMR_ACTION_CHANNEL_IC0;
	if ( chan > 1 ){
		errno = EINVAL;
		return -1;
	}
	req->value = ((uint32_t*)&(regs->CR0))[ chan ];
	return 0;
}

int _mcu_tmr_dev_write(const device_cfg_t * cfg, device_transfer_t * wop){
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
	return mcu_tmr_setaction(port, &action);
}

int mcu_tmr_setaction(int port, void * ctl){
	tmr_action_t * action = (tmr_action_t*)ctl;
	LPC_TMR_Type * regs;
	regs = tmr_regs_table[port];
	int chan;
	int event;

	event = action->event;
	chan = action->channel;

	if ( event == TMR_ACTION_EVENT_NONE ){ //Check to see if all actions are disabled
		regs->MCR &= ~(0x03 << (chan*3) );
		_mcu_tmr_local[port].callback[chan] = NULL;
	} else {


		//Check for an interrupt action with a callback
		if ( event & TMR_ACTION_EVENT_INTERRUPT){
			regs->MCR |= ((1<<0) << (chan*3) );
			_mcu_tmr_local[port].callback[chan] = action->callback;
			_mcu_tmr_local[port].context[chan] = action->context;
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
			if( event & TMR_ACTION_EVENT_PWMMODE ){
				regs->PWMC |= (1<<chan);
			} else {
				regs->PWMC &= ~(1<<chan);
			}


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
	_mcu_tmr_local[port].callback[chan] = rop->callback;
	_mcu_tmr_local[port].context[chan] = rop->context;
	return 0;
}

int mcu_tmr_set(int port, void * ctl){
	LPC_TMR_Type * regs;
	regs = tmr_regs_table[port];
	regs->TC = (uint32_t)ctl;
	return 0;
}

int mcu_tmr_get(int port, void * ctl){
	LPC_TMR_Type * regs;
	regs = tmr_regs_table[port];
	return regs->TC;
}

static void tmr_isr(int port); //This is speed optimized
//void tmr_isr(int port); //This is size optimized

//Four timers with 4 OC's and 2 IC's each
void tmr_isr(int port){
	int flags;
	LPC_TMR_Type * regs;
	regs = tmr_regs_table[port];
	flags = (regs->IR & 0x3F);
	regs->IR = flags; //clear the flags
	//execute the callbacks
	if ( (flags & MR0_FLAG) && (_mcu_tmr_local[port].callback[0]) ){
		if ( _mcu_tmr_local[port].callback[0](_mcu_tmr_local[port].context[0], (const void*)regs->MR0) == 0 ){
			_mcu_tmr_local[port].callback[0] = NULL;
		}
	}
	if ( (flags & MR1_FLAG) && (_mcu_tmr_local[port].callback[1]) ){
		if ( _mcu_tmr_local[port].callback[1](_mcu_tmr_local[port].context[1], (const void*)regs->MR1) == 0 ){
			_mcu_tmr_local[port].callback[1] = NULL;
		}
	}
	if ( (flags & MR2_FLAG) && (_mcu_tmr_local[port].callback[2]) ){
		if ( _mcu_tmr_local[port].callback[2](_mcu_tmr_local[port].context[2], (const void*)regs->MR2) == 0){
			_mcu_tmr_local[port].callback[2] = NULL;
		}
	}
	if ( (flags & MR3_FLAG) && (_mcu_tmr_local[port].callback[3]) ){
		if ( _mcu_tmr_local[port].callback[3](_mcu_tmr_local[port].context[3], (const void*)regs->MR3) == 0 ){
			_mcu_tmr_local[port].callback[3] = NULL;
		}
	}
	if ( (flags & CR0_FLAG) && (_mcu_tmr_local[port].callback[4]) ){
		if( _mcu_tmr_local[port].callback[4](_mcu_tmr_local[port].context[4], (const void*)regs->CR0) == 0 ){
			_mcu_tmr_local[port].callback[4] = NULL;
		}
	}
}

void _mcu_core_tmr0_isr(void){
	tmr_isr(0);
}

void _mcu_core_tmr1_isr(void){
	tmr_isr(1);
}

void _mcu_core_tmr2_isr(void){
	tmr_isr(2);
}

void _mcu_core_tmr3_isr(void){
	tmr_isr(3);
}


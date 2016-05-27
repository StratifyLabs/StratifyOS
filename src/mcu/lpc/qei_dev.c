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
#include "mcu/qei.h"
#include "mcu/core.h"

#if MCU_QEI_PORTS > 0

typedef struct {
	mcu_event_handler_t handler;
	uint8_t ref_count;
} qei_local_t;

static qei_local_t qei_local[MCU_QEI_PORTS] MCU_SYS_MEM;
static LPC_QEI_Type * const qei_regs[MCU_QEI_PORTS] = MCU_QEI_REGS;
static u8 const qei_irqs[MCU_QEI_PORTS] = MCU_QEI_IRQS;

void _mcu_qei_dev_power_on(int port){
	if ( qei_local[port].ref_count == 0 ){
		_mcu_lpc_core_enable_pwr(PCQEI);
		_mcu_core_priv_enable_irq((void*)(u32)(qei_irqs[port]));
	}
	qei_local[port].ref_count++;
}

void _mcu_qei_dev_power_off(int port){
	if ( qei_local[port].ref_count > 0 ){
		if ( qei_local[port].ref_count == 1 ){
			_mcu_core_priv_disable_irq((void*)(u32)(qei_irqs[port]));
			_mcu_lpc_core_disable_pwr(PCQEI);
		}
		qei_local[port].ref_count--;
	}
}

int _mcu_qei_dev_powered_on(int port){
	return _mcu_lpc_core_pwr_enabled(PCQEI);
}

int mcu_qei_setattr(int port, void * ctl){

	LPC_QEI_Type * regs = qei_regs[port];
	qei_attr_t * ctlp;
	ctlp = (qei_attr_t*)ctl;

	if ( ctlp->vfreq == 0 ){
		errno = EINVAL;
		return -1 - offsetof(qei_attr_t, vfreq);
	}

	if ( ctlp->vfreq > mcu_board_config.core_periph_freq ){
		errno = EINVAL;
		return -1 - offsetof(qei_attr_t, vfreq);
	}

	//configure the GPIO
	if ( ctlp->pin_assign == 0 ){
		_mcu_core_set_pinsel_func(1, 20, CORE_PERIPH_QEI, 0);
		_mcu_core_set_pinsel_func(1, 23, CORE_PERIPH_QEI, 0);
		_mcu_core_set_pinsel_func(1, 24, CORE_PERIPH_QEI, 0);
	} else {
		errno = EINVAL;
		return -1 - offsetof(qei_attr_t, pin_assign);
	}

	regs->MAXPOS = ctlp->max_pos;
	regs->LOAD = mcu_board_config.core_periph_freq / ctlp->vfreq;
#ifdef __lpc17xx
	regs->FILTER = ctlp->filter;
#endif

	regs->CONF = 0;
	if( ctlp->mode & QEI_MODE_INVERT_DIR ){
		regs->CONF |= (1<<0);
	}

	if( ctlp->mode & QEI_MODE_SIGNAL_MODE ){
		regs->CONF |= (1<<1);
	}

	if( ctlp->mode & QEI_MODE_DOUBLE_EDGE ){
		regs->CONF |= (1<<2);
	}

	if( ctlp->mode & QEI_MODE_INVERT_INDEX ){
		regs->CONF |= (1<<3);
	}
	return 0;
}

int mcu_qei_getattr(int port, void * ctl){
	LPC_QEI_Type * regs = qei_regs[port];

	qei_attr_t * ctlp;
	ctlp = (qei_attr_t*)ctl;

	ctlp->max_pos = regs->MAXPOS;
	if ( regs->LOAD == 0 ){
		errno = EINVAL;
		return -1;
	} else {
		ctlp->vfreq = mcu_board_config.core_periph_freq / (regs->LOAD);
	}

#ifdef __lpc17xx
	ctlp->filter = regs->FILTER;
#endif

	ctlp->mode = 0;
	if ( regs->CONF & (1<<0) ){
		ctlp->mode |= QEI_MODE_INVERT_DIR;
	}

	if ( regs->CONF & (1<<1) ){
		ctlp->mode |= QEI_MODE_SIGNAL_MODE;
	}

	if ( regs->CONF & (1<<2) ){
		ctlp->mode |= QEI_MODE_DOUBLE_EDGE;
	}

	if ( regs->CONF & (1<<3) ){
		ctlp->mode |= QEI_MODE_INVERT_INDEX;
	}

	return 0;
}

int mcu_qei_setaction(int port, void * ctl){
	LPC_QEI_Type * regs = qei_regs[port];

	mcu_action_t * action = (mcu_action_t*)ctl;
	regs->IEC = 0x1FFF;
	if( action->event & QEI_ACTION_EVENT_INDEX ){
		regs->IES = (1<<0);
	}

	if( action->event & QEI_ACTION_EVENT_DIRCHANGE ){
		regs->IES = (1<<4);
	}

	if( _mcu_core_priv_validate_callback(action->callback) < 0 ){
		return -1;
	}

	qei_local[port].handler.callback = action->callback;
	qei_local[port].handler.context = action->context;

	_mcu_core_setirqprio(qei_irqs[port], action->prio);


	return 0;
}

int _mcu_qei_dev_read(const device_cfg_t * cfg, device_transfer_t * rop){
	const int port = cfg->periph.port;
	if( _mcu_core_priv_validate_callback(rop->callback) < 0 ){
		return -1;
	}

	qei_local[port].handler.callback = rop->callback;
	qei_local[port].handler.context = rop->context;
	return 0;
}

int mcu_qei_get(int port, void * ctl){
	LPC_QEI_Type * regs = qei_regs[port];

	return regs->POS;
}

int mcu_qei_getvelocity(int port, void * ctl){
	LPC_QEI_Type * regs = qei_regs[port];

	return regs->CAP;
}

int mcu_qei_getindex(int port, void * ctl){
	LPC_QEI_Type * regs = qei_regs[port];

	return regs->INXCNT;
}

int mcu_qei_reset(int port, void * ctl){
	LPC_QEI_Type * regs = qei_regs[port];

	int mask = (int)ctl;
	if ( mask & QEI_RESET_POS ){
		regs->CON |= (1<<0);
	}

	if ( mask & QEI_RESET_VELOCITY ){
		regs->CON |= (1<<2);
	}

	if ( mask & QEI_RESET_INDEX ){
		regs->CON |= (1<<3);
	}

	if ( mask & QEI_RESET_POS_ONINDEX ){
		regs->CON |= (1<<1);
	}
	return 0;
}

void _mcu_core_qei0_isr(){
	int flags;
	const int port = 0;
	LPC_QEI_Type * regs = qei_regs[port];

	flags = regs->INTSTAT;
	regs->CLR = 0x1FFF;
	_mcu_core_exec_event_handler(&(qei_local[port].handler), (mcu_event_t)flags);
}

#endif





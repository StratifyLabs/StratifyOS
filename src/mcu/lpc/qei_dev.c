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

static qei_local_t qei_local MCU_SYS_MEM;

void _mcu_qei_dev_power_on(int port){
	if ( qei_local.ref_count == 0 ){
		_mcu_lpc_core_enable_pwr(PCQEI);
		_mcu_core_priv_enable_irq((void*)QEI_IRQn);
	}
	qei_local.ref_count++;
}

void _mcu_qei_dev_power_off(int port){
	if ( qei_local.ref_count > 0 ){
		if ( qei_local.ref_count == 1 ){
			_mcu_core_priv_disable_irq((void*)(QEI_IRQn));
			_mcu_lpc_core_disable_pwr(PCQEI);
		}
		qei_local.ref_count--;
	}
}

int _mcu_qei_dev_powered_on(int port){
	return _mcu_lpc_core_pwr_enabled(PCQEI);
}

int mcu_qei_setattr(int port, void * ctl){
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

	LPC_QEI->MAXPOS = ctlp->max_pos;
	LPC_QEI->LOAD = mcu_board_config.core_periph_freq / ctlp->vfreq;
#ifdef __lpc17xx
	LPC_QEI->FILTER = ctlp->filter;
#endif

	LPC_QEI->CONF = 0;
	if( ctlp->mode & QEI_MODE_INVERT_DIR ){
		LPC_QEI->CONF |= (1<<0);
	}

	if( ctlp->mode & QEI_MODE_SIGNAL_MODE ){
		LPC_QEI->CONF |= (1<<1);
	}

	if( ctlp->mode & QEI_MODE_DOUBLE_EDGE ){
		LPC_QEI->CONF |= (1<<2);
	}

	if( ctlp->mode & QEI_MODE_INVERT_INDEX ){
		LPC_QEI->CONF |= (1<<3);
	}
	return 0;
}

int mcu_qei_getattr(int port, void * ctl){
	qei_attr_t * ctlp;
	ctlp = (qei_attr_t*)ctl;

	ctlp->max_pos = LPC_QEI->MAXPOS;
	if ( LPC_QEI->LOAD == 0 ){
		errno = EINVAL;
		return -1;
	} else {
		ctlp->vfreq = mcu_board_config.core_periph_freq / (LPC_QEI->LOAD);
	}

#ifdef __lpc17xx
	ctlp->filter = LPC_QEI->FILTER;
#endif

	ctlp->mode = 0;
	if ( LPC_QEI->CONF & (1<<0) ){
		ctlp->mode |= QEI_MODE_INVERT_DIR;
	}

	if ( LPC_QEI->CONF & (1<<1) ){
		ctlp->mode |= QEI_MODE_SIGNAL_MODE;
	}

	if ( LPC_QEI->CONF & (1<<2) ){
		ctlp->mode |= QEI_MODE_DOUBLE_EDGE;
	}

	if ( LPC_QEI->CONF & (1<<3) ){
		ctlp->mode |= QEI_MODE_INVERT_INDEX;
	}

	return 0;
}

int mcu_qei_setaction(int port, void * ctl){
	mcu_action_t * action = (mcu_action_t*)ctl;
	LPC_QEI->IEC = 0x1FFF;
	if( action->event & QEI_ACTION_EVENT_INDEX ){
		LPC_QEI->IES = (1<<0);
	}

	if( action->event & QEI_ACTION_EVENT_DIRCHANGE ){
		LPC_QEI->IES = (1<<4);
	}

	if( _mcu_core_priv_validate_callback(action->callback) < 0 ){
		return -1;
	}

	qei_local.handler.callback = action->callback;
	qei_local.handler.context = action->context;

	return 0;
}

int _mcu_qei_dev_read(const device_cfg_t * cfg, device_transfer_t * rop){
	if( _mcu_core_priv_validate_callback(rop->callback) < 0 ){
		return -1;
	}

	qei_local.handler.callback = rop->callback;
	qei_local.handler.context = rop->context;
	return 0;
}

int mcu_qei_get(int port, void * ctl){
	return LPC_QEI->POS;
}

int mcu_qei_getvelocity(int port, void * ctl){
	return LPC_QEI->CAP;
}

int mcu_qei_getindex(int port, void * ctl){
	return LPC_QEI->INXCNT;
}

int mcu_qei_reset(int port, void * ctl){
	int mask = (int)ctl;
	if ( mask & QEI_RESET_POS ){
		LPC_QEI->CON |= (1<<0);
	}

	if ( mask & QEI_RESET_VELOCITY ){
		LPC_QEI->CON |= (1<<2);
	}

	if ( mask & QEI_RESET_INDEX ){
		LPC_QEI->CON |= (1<<3);
	}

	if ( mask & QEI_RESET_POS_ONINDEX ){
		LPC_QEI->CON |= (1<<1);
	}
	return 0;
}

void _mcu_core_qei_isr(void){
	int flags;
	flags = LPC_QEI->INTSTAT;
	LPC_QEI->CLR = 0x1FFF;
	_mcu_core_exec_event_handler(&(qei_local.handler), (mcu_event_t)flags);
}

#endif





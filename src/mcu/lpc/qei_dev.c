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
#include "mcu/cortexm.h"
#include "mcu/core.h"

#if MCU_QEI_PORTS > 0

typedef struct {
	mcu_event_handler_t handler;
	uint8_t ref_count;
} qei_local_t;

static qei_local_t qei_local[MCU_QEI_PORTS] MCU_SYS_MEM;
static LPC_QEI_Type * const qei_regs[MCU_QEI_PORTS] = MCU_QEI_REGS;
static u8 const qei_irqs[MCU_QEI_PORTS] = MCU_QEI_IRQS;

void mcu_qei_dev_power_on(const devfs_handle_t * handle){
	int port = handle->port;
	if ( qei_local[port].ref_count == 0 ){
		mcu_lpc_core_enable_pwr(PCQEI);
		mcu_cortexm_priv_enable_irq((void*)(u32)(qei_irqs[port]));
	}
	qei_local[port].ref_count++;
}

void mcu_qei_dev_power_off(const devfs_handle_t * handle){
	int port = handle->port;
	if ( qei_local[port].ref_count > 0 ){
		if ( qei_local[port].ref_count == 1 ){
			mcu_cortexm_priv_disable_irq((void*)(u32)(qei_irqs[port]));
			mcu_lpc_core_disable_pwr(PCQEI);
		}
		qei_local[port].ref_count--;
	}
}

int mcu_qei_dev_is_powered(const devfs_handle_t * handle){
	return mcu_lpc_core_pwr_enabled(PCQEI);
}

int mcu_qei_setattr(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;

	LPC_QEI_Type * regs = qei_regs[port];
	const qei_attr_t * attr = ctl;
	u32 o_flags = attr->o_flags;

	if( o_flags & QEI_FLAG_RESET ){
		if ( o_flags & QEI_FLAG_IS_RESET_POS ){
			regs->CON |= (1<<0);
		}

		if ( o_flags & QEI_FLAG_IS_RESET_VELOCITY ){
			regs->CON |= (1<<2);
		}

		if ( o_flags & QEI_FLAG_IS_RESET_INDEX ){
			regs->CON |= (1<<3);
		}

		if ( o_flags & QEI_FLAG_IS_RESET_POS_ONINDEX ){
			regs->CON |= (1<<1);
		}
	}

	if( o_flags & QEI_FLAG_SET ){


		if ( attr->velocity_freq == 0 ){
			errno = EINVAL;
			return -1 - offsetof(qei_attr_t, velocity_freq);
		}

		if ( attr->velocity_freq > mcu_board_config.core_periph_freq ){
			errno = EINVAL;
			return -1 - offsetof(qei_attr_t, velocity_freq);
		}

		if( mcu_core_set_pin_assignment(&(attr->pin_assignment),
				MCU_PIN_ASSIGNMENT_COUNT(qei_pin_assignment_t),
				CORE_PERIPH_QEI,
				port) < 0 ){
			errno = EINVAL;
			return -1 - offsetof(qei_attr_t, pin_assignment);
		}

		regs->MAXPOS = attr->max_position;
		regs->LOAD = mcu_board_config.core_periph_freq / attr->velocity_freq;
#ifdef __lpc17xx
		regs->FILTER = attr->filter;
#endif

		regs->CONF = 0;
		if( o_flags & QEI_FLAG_IS_INVERT_DIR ){
			regs->CONF |= (1<<0);
		}

		if( o_flags & QEI_FLAG_IS_SIGNAL_MODE ){
			regs->CONF |= (1<<1);
		}

		if( o_flags & QEI_FLAG_IS_DOUBLE_EDGE ){
			regs->CONF |= (1<<2);
		}

		if( o_flags & QEI_FLAG_IS_INVERT_INDEX ){
			regs->CONF |= (1<<3);
		}
	}
	return 0;
}

int mcu_qei_getinfo(const devfs_handle_t * handle, void * ctl){
	return 0;
}

int mcu_qei_setaction(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	LPC_QEI_Type * regs = qei_regs[port];

	mcu_action_t * action = (mcu_action_t*)ctl;
	regs->IEC = 0x1FFF;
	if( action->o_events & MCU_EVENT_FLAG_INDEX ){
		regs->IES = (1<<0);
	}

	if( action->o_events & MCU_EVENT_FLAG_DIRECTION_CHANGED ){
		regs->IES = (1<<4);
	}

	if( mcu_cortexm_priv_validate_callback(action->handler.callback) < 0 ){
		return -1;
	}

	qei_local[port].handler.callback = action->handler.callback;
	qei_local[port].handler.context = action->handler.context;

	mcu_cortexm_set_irq_prio(qei_irqs[port], action->prio);


	return 0;
}

int mcu_qei_dev_read(const devfs_handle_t * handle, devfs_async_t * rop){
	const int port = handle->port;
	if( mcu_cortexm_priv_validate_callback(rop->handler.callback) < 0 ){
		return -1;
	}

	qei_local[port].handler.callback = rop->handler.callback;
	qei_local[port].handler.context = rop->handler.context;
	return 0;
}

int mcu_qei_get(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	LPC_QEI_Type * regs = qei_regs[port];

	return regs->POS;
}

int mcu_qei_getvelocity(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	LPC_QEI_Type * regs = qei_regs[port];

	return regs->CAP;
}

int mcu_qei_getindex(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	LPC_QEI_Type * regs = qei_regs[port];

	return regs->INXCNT;
}

void mcu_core_qei0_isr(){
	const int port = 0;
	u32 o_events = 0;
	LPC_QEI_Type * regs = qei_regs[port];

	o_events = regs->INTSTAT;
	regs->CLR = 0x1FFF;
	mcu_execute_event_handler(&(qei_local[port].handler), o_events, 0);
}

#endif





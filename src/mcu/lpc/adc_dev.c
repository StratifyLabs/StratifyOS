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

#include <stddef.h>
#include <errno.h>
#include <fcntl.h>
#include "mcu/cortexm.h"
#include "mcu/adc.h"
#include "mcu/core.h"
#include "mcu/pio.h"
#include "mcu/debug.h"

#if MCU_ADC_PORTS > 0

#define ADC_PDN 21
#define ADC_START 24
#define ADC_DONE 31

typedef struct {
	mcu_event_handler_t handler;
	adc_sample_t * bufp;
	int len;
	uint8_t ref_count;
	uint8_t enabled_channels;
} adc_local_t;

static adc_local_t adc_local[MCU_ADC_PORTS] MCU_SYS_MEM;
static void exec_callback(int port, u32 o_events) MCU_PRIV_CODE;

static LPC_ADC_Type * const adc_regs[MCU_ADC_PORTS] = MCU_ADC_REGS;
static u8 const adc_irqs[MCU_ADC_PORTS] = MCU_ADC_IRQS;

static int enable_pin(int pio_port, int pio_pin, int adc_port) MCU_PRIV_CODE;
int enable_pin(int pio_port, int pio_pin, int adc_port){
	pio_attr_t pattr;
	pattr.o_pinmask = (1<<pio_pin);
	pattr.o_flags = PIO_FLAG_SET_INPUT | PIO_FLAG_IS_FLOAT | PIO_FLAG_IS_ANALOG;
	mcu_pio_setattr(pio_port, &pattr);
	return _mcu_core_set_pinsel_func(pio_port,pio_pin,CORE_PERIPH_ADC,adc_port);
}


void _mcu_adc_dev_power_on(int port){
	LPC_ADC_Type * regs = adc_regs[port];
	if ( adc_local[port].ref_count == 0 ){
		_mcu_lpc_core_enable_pwr(PCADC);
		_mcu_cortexm_priv_enable_irq((void*)(u32)(adc_irqs[port]));
		regs->INTEN = 0;
		memset(&adc_local, 0, sizeof(adc_local_t));
#if defined __lpc13xx || defined __lpc13uxx
		LPC_SYSCON->PDRUNCFG &= ~(1 << 4);  //Power up the ADC
		LPC_SYSCON->SYSAHBCLKCTRL |= (SYSAHBCLKCTRL_ADC);
#endif
	}

	adc_local[port].ref_count++;

}

void _mcu_adc_dev_power_off(int port){
	LPC_ADC_Type * regs = adc_regs[port];

	if ( adc_local[port].ref_count > 0 ){
		if ( adc_local[port].ref_count == 1 ){
			regs->CR = 0; //reset the control -- clear the PDN bit
			_mcu_cortexm_priv_disable_irq((void*)(u32)(adc_irqs[port]));
			_mcu_lpc_core_disable_pwr(PCADC);
		}
		adc_local[port].ref_count--;
	}
}

int _mcu_adc_dev_powered_on(int port){
	return (adc_local[port].ref_count != 0);
}

int _mcu_adc_dev_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	const int port = cfg->port;
	LPC_ADC_Type * regs = adc_regs[cfg->port];

	if ( (uint8_t)rop->loc > 7 ){
		errno = EINVAL;
		return -1;
	}

	if ( regs->INTEN & 0xFF ){
		//The interrupt is on -- port is busy
		errno = EAGAIN;
		return -1;
	}

	if( _mcu_cortexm_priv_validate_callback(rop->handler.callback) < 0 ){
		return -1;
	}

	adc_local[port].handler.callback = rop->handler.callback;
	adc_local[port].handler.context = rop->handler.context;
	adc_local[port].bufp = rop->buf;
	adc_local[port].len = rop->nbyte & ~(sizeof(adc_sample_t)-1);
	rop->nbyte = adc_local[port].len;

	regs->INTEN = (1<<8);
	regs->CR |= ((1<<ADC_START)|(1<<rop->loc)); //start the first conversion

	return 0;
}

void exec_callback(int port, u32 o_events){
	LPC_ADC_Type * regs = adc_regs[port];
	adc_local[port].bufp = NULL;
	//Disable the interrupt
	regs->INTEN = 0;
	regs->CR &= ((1<<ADC_PDN)|(0xFF00)); //leave the clock div bits in place and PDN
	mcu_execute_event_handler(&(adc_local[port].handler), o_events, 0);
}

//! \todo Should this use DMA instead of this interrupt?
void _mcu_core_adc0_isr();
void _mcu_core_adc0_isr(){
	const int port = 0;
	LPC_ADC_Type * regs = adc_regs[0];

	if ( adc_local[port].len > 0 ){
		*adc_local[port].bufp++ = (adc_sample_t)(regs->GDR & 0xFFFF);
		adc_local[port].len = adc_local[port].len - sizeof(adc_sample_t);
	}

	if ( adc_local[port].len > 0 ){
		regs->CR |= (1<<ADC_START); //set the start bit
	} else {
		exec_callback(0, MCU_EVENT_FLAG_DATA_READY);
	}
}


int mcu_adc_getinfo(int port, void * ctl){
	adc_info_t * info = ctl;

	info->freq = ADC_MAX_FREQ;
	info->o_flags = (ADC_FLAG_LEFT_JUSTIFIED|ADC_FLAG_RIGHT_JUSTIFIED);
	info->resolution = 12;

	return 0;
}

int mcu_adc_setattr(int port, void * ctl){
	int i;
	LPC_ADC_Type * regs = adc_regs[port];

	adc_attr_t * attr = ctl;
	u16 clk_div;

	if ( attr->freq == 0 ){
		errno = EINVAL;
		return -1 - offsetof(adc_attr_t, freq);
	}

	if ( attr->freq > ADC_MAX_FREQ ){
		attr->freq = ADC_MAX_FREQ;
	}

	for(i=0; i < ADC_PIN_ASSIGNMENT_COUNT; i++){
		if( mcu_is_port_valid(attr->pin_assignment[i].port) ){
			if ( enable_pin(attr->pin_assignment[i].port, attr->pin_assignment[i].pin, 0) ){
				return -1;  //pin failed to allocate as a UART pin
			}
		}
	}


	//Calculate the clock setting
#ifdef __lpc17xx
	clk_div = mcu_board_config.core_periph_freq/(attr->freq * 65);
#endif
#ifdef LPCXX7X_8X
	clk_div = mcu_board_config.core_periph_freq/(attr->freq * 31);
#endif
	if ( clk_div > 0 ){
		clk_div = clk_div - 1;
	}
	clk_div = clk_div > 255 ? 255 : clk_div;

	regs->CR = (1<<ADC_PDN)|(clk_div<<8); //Set the clock bits
	regs->INTEN = 0;
	return 0;
}

int mcu_adc_setaction(int port, void * ctl){
	LPC_ADC_Type * regs = adc_regs[port];

	mcu_action_t * action = (mcu_action_t*)ctl;
	if( action->handler.callback == 0 ){
		if ( regs->INTEN & 0xFF ){
			exec_callback(port, MCU_EVENT_FLAG_CANCELED);
		}
	}

	if( _mcu_cortexm_priv_validate_callback(action->handler.callback) < 0 ){
		return -1;
	}

	adc_local[port].handler.callback = action->handler.callback;
	adc_local[port].handler.context = action->handler.context;

	_mcu_cortexm_set_irq_prio(adc_irqs[port], action->prio);

	return 0;
}

#endif


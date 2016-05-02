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
#include "mcu/pio.h"
#include "mcu/adc.h"



#define ADC_PDN 21
#define ADC_START 24
#define ADC_DONE 31

typedef struct {
	mcu_callback_t callback;
	void * context;
	uint8_t enabled_channels;
	uint8_t ref_count;
	adc_sample_t * bufp;
	int len;
} adc_local_t;

static adc_local_t adc_local MCU_SYS_MEM;

//Register access functions
int _mcu_adc_dev_port_is_invalid(int port){
	if ( port != 0 ){
		return -1;
	}
	return 0;
}

void _mcu_adc_dev_power_on(int port){
	if ( adc_local.ref_count == 0 ){
		_mcu_lpc_core_enable_pwr(PCADC);
		_mcu_core_priv_enable_irq((void*)ADC_IRQn);
		LPC_ADC->INTEN = 0;
		LPC_SYSCON->PDRUNCFG &= ~(1 << 4);  //Power up the ADC
		LPC_SYSCON->SYSAHBCLKCTRL |= (SYSAHBCLKCTRL_ADC);
		memset(&adc_local, 0, sizeof(adc_local_t));
	}
	adc_local.ref_count++;
}

void _mcu_adc_dev_power_off(int port){
	if( adc_local.ref_count > 0 ){
		adc_local.ref_count--;
		if ( adc_local.ref_count == 0 ){
			LPC_ADC->CR = 0;
			_mcu_core_priv_disable_irq((void*)ADC_IRQn);
			_mcu_lpc_core_disable_pwr(PCADC);
			LPC_SYSCON->SYSAHBCLKCTRL &= ~(SYSAHBCLKCTRL_ADC);
			LPC_SYSCON->PDRUNCFG |= (1 << 4);  //Power down the ADC
		}
	}
}

int _mcu_adc_dev_powered_on(int port){
	return _mcu_lpc_core_pwr_enabled(PCADC);
}

int _mcu_adc_dev_read(const device_cfg_t * cfg, device_transfer_t * rop){
	if ( (uint8_t)rop->loc > 7 ){
		errno = EINVAL;
		return -1;
	}

	if ( LPC_ADC->INTEN & 0xFF ){
		//The interrupt is on -- port is busy
		errno = EAGAIN;
		return -1;
	}

	adc_local.callback = rop->callback;
	adc_local.context = rop->context;
	adc_local.bufp = rop->buf;
	adc_local.len = rop->nbyte & ~(sizeof(adc_sample_t)-1);
	rop->nbyte = adc_local.len;
	LPC_ADC->INTEN = (1<<8);
	LPC_ADC->CR |= ((1<<ADC_START)|(1<<rop->loc)); //start the first conversion

	return 0;
}

void _mcu_core_adc_isr(void){
	if ( adc_local.len > 0 ){
		*adc_local.bufp++ = (adc_sample_t)LPC_ADC->GDR;
		adc_local.len = adc_local.len - sizeof(adc_sample_t);
	}

	if ( adc_local.len > 0 ){
		LPC_ADC->CR |= (1<<ADC_START); //set the start bit
	} else {
		adc_local.bufp = NULL;
		//Disable the interrupt
		LPC_ADC->INTEN = 0;
		LPC_ADC->CR &= (0xFF00); //leave the clock div bits in place
		if ( adc_local.callback != NULL ){
			if ( adc_local.callback(adc_local.context, 0) == 0 ){
				adc_local.callback = NULL;
			}
		}
	}
}


int mcu_adc_getattr(int port, void * ctl){
	adc_attr_t * ctlp;
	uint16_t clk_div;
	ctlp = (adc_attr_t*)ctl;

	ctlp->pin_assign = 0; //GPIO config is always 0
	ctlp->enabled_channels = adc_local.enabled_channels;
	clk_div = (LPC_ADC->CR >> 8) & 0xFF;
	ctlp->freq = _mcu_core_getclock() / ((clk_div + 1) * 65); //calculate the truncated frequency value
	return 0;
}

int mcu_adc_setattr(int port, void * ctl){
	adc_attr_t * ctlp;
	int freq;
	ctlp = (adc_attr_t*)ctl;
	uint16_t clk_div;
	pio_attr_t pattr;

	if ( ctlp->freq == 0 ){
		errno = EINVAL;
		return -1;
	}

	if ( ctlp->pin_assign != 0 ){
		errno = EINVAL;
		return -1;
	}

	if ( ctlp->enabled_channels & ~(0xFF) ){
		errno = EINVAL;
		return -1;
	}

	//Calculate the clock setting
	if( ctlp->freq > ADC_MAX_FREQ ){
		freq = ADC_MAX_FREQ;
	} else {
		ctlp->freq = ADC_MAX_FREQ;
	}

	clk_div = (_mcu_core_getclock() + freq*6)/(freq * 11);
	if ( clk_div > 0 ){
		clk_div = clk_div - 1;
	}
	clk_div = clk_div > 255 ? 255 : clk_div;

	//Update the channels
	adc_local.enabled_channels = ctlp->enabled_channels;


	pattr.mode = PIO_MODE_INPUT | PIO_MODE_ANALOG | PIO_MODE_FLOAT;
	//Enable the pins to use the ADC and Disable pull-up/down resistors
	if ( adc_local.enabled_channels & (1<<0) ){
		pattr.mask = (1<<11);
		mcu_pio_setattr(0, &pattr);
		_mcu_core_set_pinsel_func(0,11,CORE_PERIPH_ADC,0);
	}

	if ( adc_local.enabled_channels & (1<<1) ){
		pattr.mask = (1<<0);
		mcu_pio_setattr(1, &pattr);
		_mcu_core_set_pinsel_func(1,0,CORE_PERIPH_ADC,0);
	}

	if ( adc_local.enabled_channels & (1<<2) ){
		pattr.mask = (1<<1);
		mcu_pio_setattr(1, &pattr);
		_mcu_core_set_pinsel_func(1,1,CORE_PERIPH_ADC,0);
	}

	if ( adc_local.enabled_channels & (1<<3) ){
		pattr.mask = (1<<2);
		mcu_pio_setattr(1, &pattr);
		_mcu_core_set_pinsel_func(1,2,CORE_PERIPH_ADC,0);
	}

	if ( adc_local.enabled_channels & (1<<4) ){
		pattr.mask = (1<<3);
		mcu_pio_setattr(1, &pattr);
		_mcu_core_set_pinsel_func(1,3,CORE_PERIPH_ADC,0);
	}

	if ( adc_local.enabled_channels & (1<<5) ){
		pattr.mask = (1<<4);
		mcu_pio_setattr(1, &pattr);
		_mcu_core_set_pinsel_func(1,4,CORE_PERIPH_ADC,0);
	}

	if ( adc_local.enabled_channels & (1<<6) ){
		pattr.mask = (1<<10);
		mcu_pio_setattr(1, &pattr);
		_mcu_core_set_pinsel_func(1,10,CORE_PERIPH_ADC,0);
	}

	if ( adc_local.enabled_channels & (1<<7) ){
		pattr.mask = (1<<11);
		mcu_pio_setattr(1, &pattr);
		_mcu_core_set_pinsel_func(1,11,CORE_PERIPH_ADC,0);
	}

	LPC_ADC->CR = (clk_div<<8); //Set the clock bits
	LPC_ADC->INTEN = 0;
	return 0;
}

int mcu_adc_setaction(int port, void * ctl){
	mcu_action_t * action = (mcu_action_t*)ctl;
	adc_local.callback = action->callback;
	adc_local.callback = action->context;
	return 0;
}


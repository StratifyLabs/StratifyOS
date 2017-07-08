/*
 * emc_dev.c
 *
 *  Created on: Sep 27, 2013
 *      Author: tgil
 */

#include <errno.h>
#include "mcu/core.h"
#include "mcu/emc.h"

#if MCU_EMC_PORTS > 0


LPC_EMC_Static_Type * const mcu_static_regs[MCU_EMC_CHANS] = MCU_EMC_STATIC_REGS;
LPC_EMC_Dynamic_Type * const mcu_dynamic_regs[MCU_EMC_CHANS] = MCU_EMC_DYNAMIC_REGS;

void _mcu_emc_dev_power_on(int port){

	//turn on the clock

	LPC_EMC->Control = 1;
	_mcu_lpc_core_enable_pwr(PCEMC);

}

void _mcu_emc_dev_power_off(int port){
	//EMC stays on after it has been enabled

	//_mcu_lpc_core_disable_pwr(PCEMC);
}

int _mcu_emc_dev_powered_on(int port){
	return _mcu_lpc_core_pwr_enabled(PCEMC);
}

int mcu_emc_getinfo(int port, void * ctl){
	errno = ENOTSUP;
	return -1;
}

int mcu_emc_setattr(int port, void * ctl){
	emc_attr_t * attr = (emc_attr_t*)ctl;
	uint32_t tmp;
	int width;
	int i;

	if( attr->channel >= MCU_EMC_CHANS ){
		errno = EINVAL;
		return -1 - offsetof(emc_attr_t, channel);
	}

	if( attr->pin_assign != 0 ){
		errno = EINVAL;
		return -1 - offsetof(emc_attr_t, pin_assign);
	}

	if( attr->addr_width > 26 ){
		errno = EINVAL;
		return -1 - offsetof(emc_attr_t, addr_width);
	}


	if( attr->clock < mcu_board_config.core_cpu_freq ){
		LPC_SC->EMCCLKSEL = 1;
	} else {
		LPC_SC->EMCCLKSEL = 0;
	}

	width = 8;

	if( attr->mode & EMC_MODE_DYNAMIC ){
		//dynamic configuration
		LPC_EMC_Dynamic_Type * dyn_regs = mcu_dynamic_regs[attr->channel];

		dyn_regs->DynamicConfig = 0;

	} else {
		//static configuration
		LPC_EMC_Static_Type * static_regs = mcu_static_regs[attr->channel];

		//check static configuration
		tmp = 0;
		if( attr->mode & EMC_MODE_PAGEMODE ){
			tmp |= (1<<3);
		}

		if( attr->mode & EMC_MODE_CSPOL_HIGH ){
			tmp |= (1<<6);
		}

		if( attr->mode & EMC_MODE_BYTELANE_LOW ){
			tmp |= (1<<7);
		}

		if( attr->mode & EMC_MODE_EXTWAIT ){
			tmp |= (1<<8);
		}

		if( attr->mode & EMC_MODE_BUFFER ){
			tmp |= (1<<19);
		}

		if( attr->mode & EMC_MODE_WP ){
			tmp |= (1<<20);
		}

		static_regs->StaticConfig = tmp;
		static_regs->StaticWaitOen = attr->wait_output_enable;
		static_regs->StaticWaitPage = attr->wait_page;
		static_regs->StaticWaitRd = attr->wait_rd;
		static_regs->StaticWaitTurn = attr->wait_turn;
		static_regs->StaticWaitWen = attr->wait_write_enable;
		static_regs->StaticWaitWr = attr->wait_wr;

		_mcu_core_set_pinsel_func(4,24,CORE_PERIPH_EMC,0); //OE

		if( attr->channel == 0 ){
			_mcu_core_set_pinsel_func(4,30,CORE_PERIPH_EMC,0); //CS0
			if( attr->mode & EMC_MODE_USEBLS ){
				_mcu_core_set_pinsel_func(4,26,CORE_PERIPH_EMC,0); //BLS0
			}
		} else if( attr->channel == 1 ){
			_mcu_core_set_pinsel_func(4,31,CORE_PERIPH_EMC,0); //CS1
			if( attr->mode & EMC_MODE_USEBLS ){
				_mcu_core_set_pinsel_func(4,27,CORE_PERIPH_EMC,0); //BLS1
			}
		} else if( attr->channel == 2 ){
			_mcu_core_set_pinsel_func(2,14,CORE_PERIPH_EMC,0); //CS2
			if( attr->mode & EMC_MODE_USEBLS ){
				_mcu_core_set_pinsel_func(4,28,CORE_PERIPH_EMC,0); //BLS2
			}
		} else if( attr->channel == 3 ){
			_mcu_core_set_pinsel_func(2,15,CORE_PERIPH_EMC,0); //CS3
			if( attr->mode & EMC_MODE_USEBLS ){
				_mcu_core_set_pinsel_func(4,29,CORE_PERIPH_EMC,0); //BLS3
			}
		}

		//set the pin assignments
		if( attr->mode & EMC_MODE_BUS8 ){
			width = 8;
		} else if (attr->mode & EMC_MODE_BUS16 ){
			static_regs->StaticConfig |= 1;
			width = 16;
		} else {
			width = 32;
			static_regs->StaticConfig |= 2;
		}


	}


	//Initialize data bus pins
	for(i=0; i < width; i++){
		_mcu_core_set_pinsel_func(3,i,CORE_PERIPH_EMC,0);
	}

	//initialize addr bus pins
	width = attr->addr_width;
	if( attr->addr_width > 24 ){
		width = 24;
	}

	for(i=0; i < width; i++){
		_mcu_core_set_pinsel_func(4,i,CORE_PERIPH_EMC,0);
	}

	if( attr->addr_width > 24 ){
		_mcu_core_set_pinsel_func(5,0,CORE_PERIPH_EMC,0);
	}

	if( attr->addr_width > 25 ){
		_mcu_core_set_pinsel_func(5,1,CORE_PERIPH_EMC,0);
	}

	_mcu_core_set_pinsel_func(4,25,CORE_PERIPH_EMC,0); //WE


	return 0;
}

int mcu_emc_setaction(int port, void * ctl){
	errno = ENOTSUP;
	return -1;
}

#endif

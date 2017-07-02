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

#include "config.h"
#include <errno.h>
#include "mcu/cortexm.h"
#include "mcu/eeprom.h"
#include "mcu/debug.h"
#include "mcu/core.h"

#if MCU_EEPROM_PORTS > 0

#define EEPROM_PAGES (MCU_EEPROM_SIZE/MCU_EEPROM_PAGE_SIZE)

typedef struct MCU_PACK {
	uint8_t * buf;
	mcu_event_handler_t handler;
	uint16_t len;
	uint8_t isread;
	uint8_t offset;
	uint8_t page;
	uint8_t ref_count;
} eeprom_local_t;

static eeprom_local_t eeprom_local[MCU_EEPROM_PORTS];
LPC_EEPROM_Type * const eeprom_regs[MCU_EEPROM_PORTS] = MCU_EEPROM_REGS;
u8 const eeprom_irqs[MCU_EEPROM_PORTS] = MCU_EEPROM_IRQS;


static void exec_callback(int port, void * data);

static int calc_offset(int loc){
	return loc % MCU_EEPROM_PAGE_SIZE;
}

static int calc_page(int loc){
	return loc / MCU_EEPROM_PAGE_SIZE;
}


void _mcu_eeprom_dev_power_on(int port){
	uint8_t phase[3];
	int cpu_mhz;
	LPC_EEPROM_Type * regs = eeprom_regs[port];
	if( eeprom_local[port].ref_count == 0 ){
		regs->PWRDWN = 0;

		//enable the interrupt
		if( eeprom_irqs[port] != 0xFF ){
			_mcu_cortexm_priv_enable_irq((void*)(u32)(eeprom_irqs[port]));
		}

		//initialize the EEPROM clock
		regs->CLKDIV = (mcu_board_config.core_cpu_freq / 375000) - 1;

		//initialize the STATE register
		cpu_mhz = mcu_board_config.core_cpu_freq / 1000000;
		phase[0] = (((cpu_mhz*15) + 500) / 1000) - 1;
		phase[1] = (((cpu_mhz*55) + 500) / 1000) - 1;
		phase[2] = (((cpu_mhz*35) + 500) / 1000) - 1;
		regs->WSTATE = phase[0] | (phase[1]<<8) | (phase[2]<<16);

	}

	eeprom_local[port].ref_count++;
}

void _mcu_eeprom_dev_power_off(int port){
	LPC_EEPROM_Type * regs = eeprom_regs[port];

	if ( eeprom_local[port].ref_count > 0 ){
		if ( eeprom_local[port].ref_count == 1 ){
			//disable the interrupt
			_mcu_cortexm_priv_disable_irq((void*)(u32)(eeprom_irqs[port]));

			//power down
			regs->PWRDWN = 1;
		}
		eeprom_local[port].ref_count--;
	}

}

int _mcu_eeprom_dev_powered_on(int port){
	LPC_EEPROM_Type * regs = eeprom_regs[port];
	return ( regs->PWRDWN & (1<<0) ) == 0;
}


int mcu_eeprom_getattr(int port, void * ctl){
	eeprom_attr_t * attr = ctl;
	attr->size = 4032;
	return 0;
}
int mcu_eeprom_setattr(int port, void * ctl){
	return 0;
}

int mcu_eeprom_setaction(int port, void * ctl){
	mcu_action_t * action = (mcu_action_t *)ctl;
	if( action->callback == 0 ){
		if( eeprom_local[port].buf != 0 ){
			exec_callback(port, MCU_EVENT_SET_CODE(MCU_EVENT_OP_CANCELLED));
		}
	}

	if( _mcu_cortexm_priv_validate_callback(action->callback) < 0 ){
		return -1;
	}

	eeprom_local[port].handler.callback = action->callback;
	eeprom_local[port].handler.context = action->context;
	return -1;
}


int _mcu_eeprom_dev_write(const device_cfg_t * cfg, device_transfer_t * wop){
	int port = cfg->periph.port;
	if ( wop->nbyte == 0 ){
		return 0;
	}

	//Check to see if the port is busy
	if ( eeprom_local[port].buf != 0 ){
		errno = EAGAIN;
		return -1;
	}

	//check for a valid wop->loc value
	if( ((wop->loc + wop->nbyte) > MCU_EEPROM_SIZE) || (wop->loc < 0) ){
		errno = EINVAL;
		return -1;
	}

	//Initialize variables
	eeprom_local[port].buf = wop->buf;
	eeprom_local[port].len = wop->nbyte;
	eeprom_local[port].isread = 0;

	eeprom_local[port].page = calc_page(wop->loc);
	eeprom_local[port].offset = calc_offset(wop->loc);

	if( _mcu_cortexm_priv_validate_callback(wop->callback) < 0 ){
		return -1;
	}

	eeprom_local[port].handler.callback = wop->callback;
	eeprom_local[port].handler.context = wop->context;

	//fill the page
	LPC_EEPROM_Type * regs = eeprom_regs[port];
	regs->ADDR = eeprom_local[port].offset;
	regs->CMD = 3;
	do {
		regs->WDATA = *eeprom_local[port].buf;
		//wait until the previous data has written
		while( (regs->INTSTAT & (1<<26)) == 0 ){
			;
		}
		regs->INTSTATCLR = (1<<26);
		eeprom_local[port].len--;
		eeprom_local[port].offset++;
		eeprom_local[port].buf++;
	} while( (eeprom_local[port].offset < MCU_EEPROM_PAGE_SIZE) && (eeprom_local[port].len > 0) );


	regs->ADDR = eeprom_local[port].page << 6;
	eeprom_local[port].page++;
	eeprom_local[port].offset = 0;
	regs->INT_SET_ENABLE = (1<<28);

	regs->CMD = 6; //erase/program page


	return 0;

}

int _mcu_eeprom_dev_read(const device_cfg_t * cfg, device_transfer_t * rop){
	int port = cfg->periph.port;

	if ( rop->nbyte == 0 ){
		return 0;
	}

	//Check to see if the port is busy
	if ( eeprom_local[port].buf != 0 ){
		errno = EAGAIN;
		return -1;
	}

	//check for a valid rop->loc value
	if( ((rop->loc + rop->nbyte) > MCU_EEPROM_SIZE) || (rop->loc < 0) ){
		errno = EINVAL;
		return -1;
	}


	//Initialize variables
	eeprom_local[port].buf = rop->buf;
	eeprom_local[port].len = rop->nbyte;
	eeprom_local[port].page = calc_page(rop->loc);
	eeprom_local[port].offset = calc_offset(rop->loc);


	LPC_EEPROM_Type * regs = eeprom_regs[port];
	regs->INTSTATCLR = (1<<26) | (1<<28);


	regs->ADDR = eeprom_local[port].offset | (eeprom_local[port].page << 6);
	regs->CMD = 0 | (1<<3);
	do {
		*eeprom_local[port].buf = regs->RDATA;
		//wait until the previous data has been read
		while( (regs->INTSTAT & (1<<26)) == 0 ){
			;
		}
		regs->INTSTATCLR = (1<<26);
		eeprom_local[port].len--;
		eeprom_local[port].offset++;
		eeprom_local[port].buf++;

		if( eeprom_local[port].offset == MCU_EEPROM_PAGE_SIZE ){
			eeprom_local[port].offset = 0;
			eeprom_local[port].page++;
			regs->ADDR = (eeprom_local[port].page << 6);
			regs->CMD = 0 | (1<<3);
		}

	} while( eeprom_local[port].len > 0 );
	eeprom_local[port].buf = 0;
	return rop->nbyte;
}

void exec_callback(int port, void * data){
	LPC_EEPROM_Type * regs = eeprom_regs[port];
	eeprom_local[port].buf = 0;
	regs->INTENCLR = (1<<26)|(1<<28); //disable the interrupts
	_mcu_cortexm_execute_event_handler(&(eeprom_local[port].handler), 0);
}

void _mcu_core_eeprom0_isr(){
	const int port = 0;
	LPC_EEPROM_Type * regs = eeprom_regs[port];
	uint32_t status = regs->INTSTAT;
	regs->INTSTATCLR = status;
	if( status & (1<<28) ){
		//this was a program/erase action
		if( eeprom_local[port].len > 0 ){
			regs->ADDR = eeprom_local[port].offset;
			regs->CMD = 3;
			do {
				regs->WDATA = *eeprom_local[port].buf;
				//wait until the previous data has written
				while( (regs->INTSTAT & (1<<26)) == 0 ){
					;
				}
				regs->INTSTATCLR = (1<<26);
				eeprom_local[port].len--;
				eeprom_local[port].offset++;
				eeprom_local[port].buf++;
			} while( (eeprom_local[port].offset < MCU_EEPROM_PAGE_SIZE) && (eeprom_local[port].len > 0) );

			regs->ADDR = eeprom_local[port].page << 6;
			regs->CMD = 6; //erase/program page
			eeprom_local[port].page++;
			eeprom_local[port].offset = 0;
			return;
		}
	}

	if( eeprom_local[port].len == 0 ){
		exec_callback(0, 0);
	}
}

#endif


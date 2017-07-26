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
#include <fcntl.h>
#include "cortexm/cortexm.h"
#include "mcu/mci.h"
#include "mcu/pio.h"
#include "mcu/debug.h"
#include "mcu/core.h"

#if MCU_MCI_PORTS > 0

#define WRITE_OP 0
#define READ_OP 1

static LPC_MCI_Type * const mci_regs_table[MCU_MCI_PORTS] = MCU_MCI_REGS;

static void exec_callback(int port, void * data);


static int mcu_mci_write_fifo(LPC_MCI_Type * regs, uint32_t * src, int nbyte);

typedef union {
	uint16_t ptr16;
	uint8_t ptr8[2];
} i2c_ptr_t;

typedef struct {
	volatile char * data;
	volatile uint16_t size;
	volatile i2c_ptr_t ptr;
	char addr;
	uint8_t ref_count;
	volatile uint8_t state;
	volatile int err;
	int * ret;
	mcu_event_handler_t handler;
} mci_local_t;

mci_attr_t mci_local_attr[MCU_MCI_PORTS] MCU_SYS_MEM;
mci_local_t mci_local[MCU_MCI_PORTS] MCU_SYS_MEM;

void mcu_mci_dev_power_on(const devfs_handle_t * handle){
	int port = handle->port;
	if ( mci_local[port].ref_count == 0 ){
		mcu_lpc_core_enable_pwr(PCMCI);
		cortexm_enable_irq((void*)MCI_IRQn);
		mci_local[port].handler.callback = NULL;
	}
	mci_local[port].ref_count++;
}

void mcu_mci_dev_power_off(const devfs_handle_t * handle){
	int port = handle->port;
	if ( mci_local[port].ref_count > 0 ){
		if ( mci_local[port].ref_count == 1 ){
			cortexm_disable_irq((void*)MCI_IRQn);
			mcu_lpc_core_enable_pwr(PCMCI);
		}
		mci_local[port].ref_count--;
	}
}

int mcu_mci_dev_is_powered(const devfs_handle_t * handle){
	int port = handle->port;
	return ( mci_local[port].ref_count != 0 );
}

int mcu_mci_getinfo(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	memcpy(ctl, &(mci_local_attr[port]), sizeof(mci_attr_t));
	return 0;
}

int mcu_mci_setattr(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	mci_attr_t * attr = ctl;
	LPC_MCI_Type * regs = mci_regs_table[port];

	int32_t clkdiv;


	if( attr->freq == 0 ){
		attr->freq = 100000000; //set to max freq
	}

	clkdiv = (mcu_board_config.core_periph_freq + attr->freq) / (2 * attr->freq) - 1;

	if( clkdiv > 255 ){
		clkdiv = 255;
	}

	if( attr->pin_assign > 3 ){
		errno = EINVAL;
		return (-1 - offsetof(mci_attr_t, pin_assign));
	}

	if( attr->mode & (MCI_MODE_WIDEBUS) ){

	}

	//setup the pin selection
	switch(attr->pin_assign){
	case 1:
		mcu_core_set_pinsel_func(1, 5, CORE_PERIPH_MCI, port); //SD_PWR
		/* no break */
	case 0:
		//use pins with port 1
		mcu_core_set_pinsel_func(1,  6, CORE_PERIPH_MCI, port); //SD_DAT0
		if( attr->mode & (MCI_MODE_WIDEBUS) ){
			mcu_core_set_pinsel_func(1,  7, CORE_PERIPH_MCI, port); //SD_DAT1
			mcu_core_set_pinsel_func(1, 11, CORE_PERIPH_MCI, port); //SD_DAT2
			mcu_core_set_pinsel_func(1, 12, CORE_PERIPH_MCI, port); //SD_DAT3
		}
		mcu_core_set_pinsel_func(1,  2, CORE_PERIPH_MCI, port); //SD_CLK
		mcu_core_set_pinsel_func(1,  3, CORE_PERIPH_MCI, port); //SD_CMD
		break;
	case 3:
		//use the alternate setting with SD PWR
		/* no break */
	case 2:
		//use alternate setting
		break;

	}


	regs->CLOCK = (1<<8)|(1<<9);
	if( attr->mode & MCI_MODE_WIDEBUS ){
		regs->CLOCK |= (1<<11);
	}
	if( clkdiv < 0 ){
		regs->CLOCK |= (1<<10); //by-pass divider
	}



	return -1;
}


int mcu_mci_setaction(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	mcu_action_t * action = (mcu_action_t*)ctl;
	if( action->handler.callback == 0 ){


	}

	if( cortexm_validate_callback(action->handler.callback) < 0 ){
		return -1;
	}

	mci_local[port].handler.callback = action->handler.callback;
	mci_local[port].handler.context = action->handler.context;

	return 0;
}

static int mcu_mci_write_fifo(LPC_MCI_Type * regs, uint32_t * src, int nbyte){
	return nbyte;
}


int mcu_mci_dev_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	int port = cfg->port;
	LPC_MCI_Type * regs = mci_regs_table[port];

	//int num_blocks;

	//num_blocks = wop->nbyte / mci_local_attr[port].block_size;



	regs->DATATMR = 5000; //set the timeout
	regs->DATALEN = wop->nbyte;

	//bit 2 is MODE (block or stream?)
	regs->DATACTRL = (1<<0); //enable from controller to card

	//enable the interrupt

	//load up the TXFIFO
	mcu_mci_write_fifo(0,0,0);

	return -1;
}

int mcu_mci_dev_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	return -1;
}


void mcu_mci0_isr(int port) {
	exec_callback(port, 0);
}

void exec_callback(int port, void * data){
	/*
	mci_local[port].state |= I2C_DONE_FLAG;
	if ( mci_local[port].err != 0 ){
		*(mci_local[port].ret) = -1;
	}


	*/

	mcu_execute_event_handler(&(mci_local[port].handler), MCU_EVENT_FLAG_WRITE_COMPLETE, 0);

}

#endif



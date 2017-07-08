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
#include "mcu/cortexm.h"
#include "mcu/spi.h"
#include "mcu/pio.h"
#include "spi_flags.h"
#include "mcu/core.h"
#include "mcu/debug.h"

#if MCU_SPI_PORTS > 0

typedef struct {
	char * volatile rx_buf;
	char * volatile tx_buf;
	volatile int size;
	void * duplex_mem;
	int ret;
	u8 pin_assign;
	u8 width;
	u8 ref_count;
	mcu_callback_t callback;
	void * context;
} spi_local_t;

static spi_local_t spi_local[MCU_SPI_PORTS] MCU_SYS_MEM;

static LPC_SPI_Type * const spi_regs[MCU_SPI_PORTS] = MCU_SPI_REGS;
static u8 const spi_irqs[MCU_SPI_PORTS] = MCU_SPI_IRQS;

static void exec_callback(int port, void * data);

static int spi_port_transfer(int port, int is_read, devfs_async_t * dop);
static int byte_swap(int port, int byte);

void _mcu_spi_dev_power_on(int port){
	if ( spi_local[port].ref_count == 0 ){
		_mcu_lpc_core_enable_pwr(PCSPI);
		_mcu_cortexm_priv_enable_irq((void*)(u32)(spi_irqs[port]));
		spi_local[port].duplex_mem = NULL;
		spi_local[port].callback = NULL;
	}
	spi_local[port].ref_count++;

}

void _mcu_spi_dev_power_off(int port){
	if ( spi_local[port].ref_count > 0 ){
		if ( spi_local[port].ref_count == 1 ){
			_mcu_cortexm_priv_disable_irq((void*)(u32)(spi_irqs[port]));
			_mcu_lpc_core_disable_pwr(PCSPI);
		}
		spi_local[port].ref_count--;
	}
}

int _mcu_spi_dev_powered_on(int port){
	return ( spi_local[port].ref_count != 0 );
}


int mcu_spi_getinfo(int port, void * ctl){
	LPC_SPI_Type * regs = spi_regs[port];
	spi_info_t * info = ctl;

	//set flags

	return 0;
}

int mcu_spi_setattr(int port, void * ctl){
	LPC_SPI_Type * regs = spi_regs[port];
	uint32_t cr0, cpsr;
	uint32_t tmp;
	spi_attr_t * attr = (spi_attr_t*)ctl;
	u32 mode;
	u32 o_flags = attr->o_flags;
	int i;


	if( o_flags & SPI_FLAG_SET_MASTER ){

		if( attr->freq == 0 ){
			errno = EINVAL;
			return -1 - offsetof(spi_attr_t, freq);
		}

		if( attr->width > 8 ){
			errno = EINVAL;
			return -1 - offsetof(spi_attr_t, width);
		}

		mode = 0;
		if( o_flags & SPI_FLAG_MODE1 ){
			mode = 1;
		} else if( o_flags & SPI_FLAG_MODE2 ){
			mode = 2;
		} else if( o_flags & SPI_FLAG_MODE3 ){
			mode = 3;
		}


		//2 uses the SPI
		cr0 = (1<<2);

		tmp = mcu_board_config.core_periph_freq / attr->freq;
		tmp = ( tmp > 255 ) ? 254 : tmp;
		tmp = ( tmp < 8 ) ? 8 : tmp;
		if ( tmp & 0x01 ){
			tmp++; //round the divisor up so that actual is less than the target
		}
		cpsr = (tmp & 0xFE);

		cr0 |= (mode << 3);
		cr0 |= (1<<5);  //must be a master


		if ( attr->width >= 8 && attr->width <= 16 ){
			cr0 |= (( attr->width & 0x0F ) << 8);
		} else {
			errno = EINVAL;
			return -1;
		}

		if( mcu_core_set_pin_assignment(attr->pin_assignment, SPI_PIN_ASSIGNMENT_COUNT, CORE_PERIPH_SPI, port) < 0 ){
			return -1;
		}

		regs->CCR = cpsr & 0xFE;
		regs->CR = cr0;

	}


	return 0;
}

int mcu_spi_swap(int port, void * ctl){
	return byte_swap(port, (int)ctl);
}

int mcu_spi_setduplex(int port, void * ctl){
	spi_local[port].duplex_mem = (void * volatile)ctl;
	return 0;
}

void exec_callback(int port, void * data){
	LPC_SPI_Type * regs = spi_regs[port];
	regs->CR &= ~(SPIE); //disable the interrupt
	if ( spi_local[port].callback != NULL ){
		if( spi_local[port].callback(spi_local[port].context, data) == 0 ){
			spi_local[port].callback = NULL;
		}
	}
}

int mcu_spi_setaction(int port, void * ctl){
	LPC_SPI_Type * regs = spi_regs[port];

	mcu_action_t * action = (mcu_action_t*)ctl;
	if( action->handler.callback == 0 ){
		//cancel any ongoing operation
		if ( regs->CR & (SPIE) ){
			exec_callback(port, MCU_EVENT_SET_CODE(MCU_EVENT_OP_CANCELLED));
		}
	}

	spi_local[port].callback = action->handler.callback;
	spi_local[port].context = action->handler.context;
	_mcu_cortexm_set_irq_prio(spi_irqs[port], action->prio);

	return 0;
}


int byte_swap(int port, int byte){
	LPC_SPI_Type * regs = spi_regs[port];
	regs->DR = byte; //start the next transfer
	while( !(regs->SR) ); //wait for transfer to complete
	return regs->DR;
}

int _mcu_spi_dev_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	return spi_port_transfer(cfg->port, 0, wop);
}

int _mcu_spi_dev_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	return spi_port_transfer(cfg->port, 1, rop);
}

void _mcu_core_spi0_isr(){
	int tmp;
	const int port = 0;
	LPC_SPI_Type * regs = spi_regs[port];


	(volatile int)regs->SR;
	//! \todo Check for errors

	tmp = regs->DR;
	if ( spi_local[port].rx_buf != NULL ){
		*spi_local[port].rx_buf++ = tmp;
	}

	if ( spi_local[port].size ){
		spi_local[port].size--;
	}

	if ( spi_local[port].size ){
		if ( spi_local[port].tx_buf != NULL ){
			tmp = *spi_local[port].tx_buf++;
		} else {
			tmp = 0xFF;
		}

		regs->DR = tmp;
	}

	regs->INT |= (SPIF_INT); //clear the interrupt flag

	if ( spi_local[port].size == 0 ){
		exec_callback(0, 0);
	}
}

int spi_port_transfer(int port, int is_read, devfs_async_t * dop){
	LPC_SPI_Type * regs = spi_regs[port];
	int size;
	size = dop->nbyte;

	if ( regs->CR & (SPIE) ){
		errno = EAGAIN;
		return -1;
	}

	if ( size == 0 ){
		return 0;
	}

	if ( is_read ){
		spi_local[port].rx_buf = dop->buf;
		spi_local[port].tx_buf = spi_local[port].duplex_mem;
	} else {
		spi_local[port].tx_buf = dop->buf;
		spi_local[port].rx_buf = spi_local[port].duplex_mem;
	}
	spi_local[port].size = size;
	spi_local[port].callback = dop->handler.callback;
	spi_local[port].context = dop->handler.context;
	regs->CR |= SPIE; //enable the interrupt
	if ( spi_local[port].tx_buf ){
		//! \todo This won't handle spi widths other than 8 bits
		regs->DR = *spi_local[port].tx_buf++;
	} else {
		regs->DR = 0xFF;
	}

	spi_local[port].ret = size;

	return 0;
}

#endif


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
	mcu_event_handler_t handler;
} spi_local_t;

static spi_local_t spi_local[MCU_SPI_PORTS] MCU_SYS_MEM;

static LPC_SPI_Type * const spi_regs[MCU_SPI_PORTS] = MCU_SPI_REGS;
static u8 const spi_irqs[MCU_SPI_PORTS] = MCU_SPI_IRQS;

static void exec_callback(int port, u32 o_events);

static int spi_port_transfer(const devfs_handle_t * handle, int is_read, devfs_async_t * dop);
static int byte_swap(int port, int byte);

void mcu_spi_dev_power_on(const devfs_handle_t * handle){
	int port = handle->port;
	if ( spi_local[port].ref_count == 0 ){
		mcu_lpc_core_enable_pwr(PCSPI);
		cortexm_enable_irq((void*)(u32)(spi_irqs[port]));
		spi_local[port].duplex_mem = NULL;
		spi_local[port].handler.callback = NULL;
	}
	spi_local[port].ref_count++;

}

void mcu_spi_dev_power_off(const devfs_handle_t * handle){
	int port = handle->port;
	if ( spi_local[port].ref_count > 0 ){
		if ( spi_local[port].ref_count == 1 ){
			cortexm_disable_irq((void*)(u32)(spi_irqs[port]));
			mcu_lpc_core_disable_pwr(PCSPI);
		}
		spi_local[port].ref_count--;
	}
}

int mcu_spi_dev_is_powered(const devfs_handle_t * handle){
	int port = handle->port;
	return ( spi_local[port].ref_count != 0 );
}


int mcu_spi_getinfo(const devfs_handle_t * handle, void * ctl){
	spi_info_t * info = ctl;

	//set flags
	info->o_flags = 0;

	return 0;
}

int mcu_spi_setattr(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	LPC_SPI_Type * regs = spi_regs[port];
	u32 cr0, cpsr;
	u32 tmp;
	u32 mode;

	const spi_attr_t * attr = mcu_select_attr(handle, ctl);
	if( attr == 0 ){
		return -1;
	}

	u32 o_flags = attr->o_flags;


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
		if( o_flags & SPI_FLAG_IS_MODE1 ){
			mode = 1;
		} else if( o_flags & SPI_FLAG_IS_MODE2 ){
			mode = 2;
		} else if( o_flags & SPI_FLAG_IS_MODE3 ){
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


		if ( (attr->width >= 8) && (attr->width <= 16) ){
			cr0 |= (( attr->width & 0x0F ) << 8);
		} else {
			errno = EINVAL;
			return -1;
		}

		if( mcu_set_pin_assignment(
				&(attr->pin_assignment),
				MCU_CONFIG_PIN_ASSIGNMENT(spi_config_t, handle),
				MCU_PIN_ASSIGNMENT_COUNT(spi_pin_assignment_t),
				CORE_PERIPH_SPI, port, 0, 0) < 0 ){
			return -1;
		}

		regs->CCR = cpsr & 0xFE;
		regs->CR = cr0;

	}


	return 0;
}

int mcu_spi_swap(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	return byte_swap(port, (int)ctl);
}

int mcu_spi_setduplex(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	spi_local[port].duplex_mem = (void * volatile)ctl;
	return 0;
}

void exec_callback(int port, u32 o_events){
	LPC_SPI_Type * regs = spi_regs[port];
	regs->CR &= ~(SPIE); //disable the interrupt

	mcu_execute_event_handler(&(spi_local[port].handler), o_events, 0);
}

int mcu_spi_setaction(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;

	mcu_action_t * action = (mcu_action_t*)ctl;

	if ( (action->handler.callback == 0) && (action->o_events & (MCU_EVENT_FLAG_DATA_READY|MCU_EVENT_FLAG_WRITE_COMPLETE)) ){
		exec_callback(port, MCU_EVENT_FLAG_CANCELED);
	}

	spi_local[port].handler = action->handler;
	cortexm_set_irq_prio(spi_irqs[port], action->prio);
	return 0;
}


int byte_swap(int port, int byte){
	LPC_SPI_Type * regs = spi_regs[port];
	regs->DR = byte; //start the next transfer
	while( !(regs->SR) ); //wait for transfer to complete
	return regs->DR;
}

int mcu_spi_dev_write(const devfs_handle_t * handle, devfs_async_t * wop){
	return spi_port_transfer(handle, 0, wop);
}

int mcu_spi_dev_read(const devfs_handle_t * handle, devfs_async_t * rop){
	return spi_port_transfer(handle, 1, rop);
}

void mcu_core_spi0_isr(){
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
		exec_callback(0, MCU_EVENT_FLAG_WRITE_COMPLETE|MCU_EVENT_FLAG_DATA_READY);
	}
}

int spi_port_transfer(const devfs_handle_t * handle, int is_read, devfs_async_t * dop){
	int port = handle->port;
	LPC_SPI_Type * regs = spi_regs[port];
	int size;
	size = dop->nbyte;

	if ( spi_local[port].handler.callback ){
		errno = EBUSY;
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
	spi_local[port].handler = dop->handler;
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


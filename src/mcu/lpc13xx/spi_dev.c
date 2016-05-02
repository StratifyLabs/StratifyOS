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
#include "mcu/arch.h"
#include "mcu/spi.h"
#include "mcu/pio.h"
#include "ssp_flags.h"

typedef struct {
	mcu_callback_t callback;
	void * context;
	volatile int size;
	char * volatile rx_buf;
	char * volatile tx_buf;
	void * duplex_mem;
	int ret;
	uint8_t pin_assign;
	uint8_t width;
} spi_local_t;

static spi_local_t spi_local[MCU_SSP_PORTS] MCU_SYS_MEM;

static int ssp_port_transfer(int port, int is_read, device_transfer_t * dop);
static void ssp_fill_tx_fifo(int port);
static void ssp_empty_rx_fifo(int port);
static int byte_swap(int port, int byte);

LPC_SSP_TypeDef * const ssp_regs_table[MCU_SSP_PORTS] = MCU_SSP_REGS;

int _mcu_spi_dev_port_is_invalid(int port){
	if ( (uint32_t)port >= MCU_SSP_PORTS ){
		return 1;
	}
	return 0;
}

void _mcu_spi_dev_power_on(int port){
	spi_local[port].duplex_mem = NULL;
	spi_local[port].callback = NULL;
	switch(port){
	case 0:
		LPC_SYSCON->SSP0CLKDIV = 1;
		LPC_SYSCON->PRESETCTRL |= (1<<0);
		_mcu_lpc_core_enable_pwr(PCSSP0); //Enable clock to SSP module (including the register block)
		_mcu_core_priv_enable_irq((void*)SSP0_IRQn);
		break;
#if MCU_SSP_PORTS > 1
	case 1:
		LPC_SYSCON->SSP1CLKDIV = 1;
		LPC_SYSCON->PRESETCTRL |= (1<<2);
		_mcu_lpc_core_enable_pwr(PCSSP1);
		_mcu_core_priv_enable_irq((void*)SSP1_IRQn);
		break;
#endif
	}
}

void _mcu_spi_dev_power_off(int port){
	switch(port){
	case 0:
		LPC_SYSCON->PRESETCTRL &= ~(1<<0);
		_mcu_core_priv_disable_irq((void*)SSP0_IRQn);
		_mcu_lpc_core_disable_pwr(PCSSP0);
		break;
#if MCU_SSP_PORTS > 1
	case 1:
		LPC_SYSCON->PRESETCTRL &= ~(1<<2);
		_mcu_core_priv_disable_irq((void*)SSP1_IRQn);
		_mcu_lpc_core_disable_pwr(PCSSP1);
		break;
#endif
	}

}

int _mcu_spi_dev_powered_on(int port){
	switch(port){
		case 0:
			return _mcu_lpc_core_pwr_enabled(PCSSP0);
#if MCU_SSP_PORTS > 1
		case 1:
			return _mcu_lpc_core_pwr_enabled(PCSSP1);
#endif
	}
	return 1;
}


int mcu_spi_getattr(int port, void * ctl){
	LPC_SSP_TypeDef * regs;
	spi_attr_t * ctlp = (spi_attr_t*)ctl;
	uint8_t tmp;

	ctlp->pin_assign = spi_local[port].pin_assign;
	regs = ssp_regs_table[port];
	//Master
	if ( regs->CR1 & (1<<2) ){
		//Slave
		ctlp->master = SPI_ATTR_SLAVE;
	} else {
		ctlp->master = SPI_ATTR_MASTER;
	}

	//width
	ctlp->width = (regs->CR0 & 0x0F) + 1;

	//format
	tmp = (regs->CR0 >> 4) & 0x03;
	if ( tmp == 0 ){
		ctlp->format = SPI_ATTR_FORMAT_SPI;
	} else if ( tmp == 1 ){
		ctlp->format = SPI_ATTR_FORMAT_TI;
	} else if ( tmp == 2 ){
		ctlp->format = SPI_ATTR_FORMAT_MICROWIRE;
	}

	//Mode
	ctlp->mode = 0;
	if ( regs->CR0 & (1<<6) ){
		ctlp->mode |= 0x02;
	}
	if ( regs->CR0 & (1<<7) ){
		ctlp->mode |= 0x01;
	}

	//bitrate
	if ( regs->CPSR != 0){
		ctlp->bitrate = _mcu_core_getclock() / (regs->CPSR);
	} else {
		ctlp->bitrate = -1;
	}


	return 0;
}

int mcu_spi_setattr(int port, void * ctl){
	LPC_SSP_TypeDef * regs;
	uint32_t cr0, cr1, cpsr;
	uint32_t tmp;
	spi_attr_t * ctlp = (spi_attr_t*)ctl;
	regs = ssp_regs_table[port];

	if ( ctlp->bitrate == 0 ) {
		errno = EINVAL;
		return -1;
	}

	if ( ctlp->mode >= 4 ) {
		errno = EINVAL;
		return -1;
	}



	if( ctlp->width > 8 ){
		errno = EINVAL;
		return -1;
	}

	cr0 = 0;
	cr1 = (1<<1); //set the enable
	if ( ctlp->master != SPI_ATTR_MASTER ){
		cr1 |= (1<<2);
	}

	tmp = _mcu_core_getclock() / ctlp->bitrate;
	tmp = ( tmp > 255 ) ? 254 : tmp;
	tmp = ( tmp < 2 ) ? 2 : tmp;
	if ( tmp & 0x01 ){
		tmp++; //round the divisor up so that actual is less than the target
	}
	cpsr = tmp;

	if ( ctlp->mode & 0x01 ){
		cr0 |= (1<<7);
	}

	if ( ctlp->mode & 0x02 ){
		cr0 |= (1<<6);
	}

	if ( ctlp->width >= 4 && ctlp->width <= 16 ){
		cr0 |= ( ctlp->width - 1);
	} else {
		errno = EINVAL;
		return -1;
	}

	if ( ctlp->format == SPI_ATTR_FORMAT_SPI ){
		;
	} else if ( ctlp->format == SPI_ATTR_FORMAT_TI ){
		cr0 |= (1<<4);
	} else if ( ctlp->format == SPI_ATTR_FORMAT_MICROWIRE ){
		cr0 |= (1<<5);
	} else {
		errno = EINVAL;
		return -1;
	}


	if ( ctlp->pin_assign != MCU_GPIO_CFG_USER ){

		switch(port){
		case 0:
			if (1){
				switch(ctlp->pin_assign){
				case 0:
					if ( _mcu_spi_cfg_pio(0, 0, 9, 8, 6) < 0 ){
						errno = ENODEV;
						return -1;
					}
					LPC_IOCON->SCK_LOC = 2;
					break;
				case 1:
					if( _mcu_spi_cfg_pio(0, 0, 9, 8, 10) < 0 ){
						errno = ENODEV;
						return -1;
					}
					LPC_IOCON->SCK_LOC = 0;
					break;
				case 2:
					if ( _mcu_core_set_pinsel_func(0, 9, CORE_PERIPH_SPI, port) < 0 ) return -1;  //Use pin for SPI
					if ( _mcu_core_set_pinsel_func(0, 8, CORE_PERIPH_SPI, port) < 0 ) return -1;  //Use pin for SPI
					if ( _mcu_core_set_pinsel_func(2, 11, CORE_PERIPH_SPI, port) < 0 ) return -1;  //Use pin for SPI
					LPC_IOCON->SCK_LOC = 1;
					break;
				default:
					errno = EINVAL;
					return -1;
				}

				if( ctlp->master == SPI_ATTR_SLAVE ){
					if ( _mcu_core_set_pinsel_func(0, 2, CORE_PERIPH_SPI, port) ) return -1;  //Use SSEL
				}

			}
			break;
#if MCU_SSP_PORTS > 1
		case 1:
			if (1){
				switch(ctlp->pin_assign){
				case 0:
					if ( _mcu_spi_cfg_pio(1, 0, 9, 8, 7) < 0 ){
						errno = ENODEV;
						return -1;
					}
					break;
				default:
					errno = EINVAL;
					return -1;
				}
			}
			break;
#endif
		}
	}

	regs->CR0 = cr0;
	regs->CR1 = cr1;
	regs->CPSR = cpsr;

	spi_local[port].pin_assign = ctlp->pin_assign;

	return 0;
}

int mcu_spi_swap(int port, void * ctl){
	return byte_swap(port, (int)ctl);
}

int mcu_spi_setduplex(int port, void * ctl){
	spi_local[port].duplex_mem = (void * volatile)ctl;
	return 0;
}

int mcu_spi_setaction(int port, void * ctl){
	mcu_action_t * action = (mcu_action_t*)ctl;
	spi_local[port].callback = action->callback;
	spi_local[port].context = action->context;
	return 0;
}


int byte_swap(int port, int byte){
	LPC_SSP_TypeDef * regs;
	regs = ssp_regs_table[port];
	regs->DR = byte;
	while ( regs->SR & SSPSR_BSY ){
		;
	}
	byte = regs->DR; //read the byte to empty the RX FIFO
	return byte;
}

int _mcu_spi_dev_write(const device_cfg_t * cfg, device_transfer_t * wop){
	int port = DEVICE_GET_PORT(cfg);
	return ssp_port_transfer(port, 0, wop);
}

int _mcu_spi_dev_read(const device_cfg_t * cfg, device_transfer_t * rop){
	int port = DEVICE_GET_PORT(cfg);
	return ssp_port_transfer(port, 1, rop);
}

void ssp_fill_tx_fifo(int port){
	LPC_SSP_TypeDef * regs;
	regs = ssp_regs_table[port];
	int size = 0;
	while ( (regs->SR & SSPSR_TNF) && spi_local[port].size && (size < 8) ){
		if ( spi_local[port].tx_buf != NULL ){
			//! \todo This won't handle spi widths other than 8 bits -- need to read the transmit width
			regs->DR = *spi_local[port].tx_buf++;
		} else {
			regs->DR = 0xFF;
		}
		spi_local[port].size--;
		size++;
	}
}

void ssp_empty_rx_fifo(int port){
	LPC_SSP_TypeDef * regs;
	regs = ssp_regs_table[port];
	while ( regs->SR & SSPSR_RNE ){
		if ( spi_local[port].rx_buf != NULL ){
			*spi_local[port].rx_buf++ = regs->DR;
		} else {
			regs->DR;
		}
	}
}

void _mcu_core_ssp_isr(int port){
	LPC_SSP_TypeDef * regs;
	regs = ssp_regs_table[port];
	ssp_fill_tx_fifo(port);
	ssp_empty_rx_fifo(port);
	if ( (regs->SR & (SSPSR_TFE)) && !(regs->SR & (SSPSR_RNE)) ){
		regs->IMSC &= ~(SSPIMSC_RXIM|SSPIMSC_RTIM); //Kill the interrupts

		if ( (spi_local[port].size == 0) && (spi_local[port].callback != NULL) ){
			if ( spi_local[port].callback(spi_local[port].context, 0) == 0 ){
				spi_local[port].callback = NULL;
			}
		}
	}
}

void _mcu_core_ssp0_isr(void){
	_mcu_core_ssp_isr(0);
}

void _mcu_core_ssp1_isr(void){
	_mcu_core_ssp_isr(1);
}


int ssp_port_transfer(int port, int is_read, device_transfer_t * dop){
	int size;
	LPC_SSP_TypeDef * regs;
	size = dop->nbyte;
	if ( size == 0 ){
		return 0;
	}

	regs = ssp_regs_table[port];

	//Check to see if SSP port is busy
	if ( regs->IMSC & (SSPIMSC_RXIM|SSPIMSC_RTIM) ){
		errno = EAGAIN;
		return -1;
	}

	if ( is_read ){
		spi_local[port].rx_buf = dop->buf;
		spi_local[port].tx_buf = spi_local[port].duplex_mem;
	} else {
		spi_local[port].tx_buf = dop->buf;
		spi_local[port].rx_buf = spi_local[port].duplex_mem;
	}
	spi_local[port].size = size;
	spi_local[port].callback = dop->callback;
	spi_local[port].context = dop->context;
	ssp_fill_tx_fifo(port);

	//! \todo Use DMA for SSP spi_local??
	regs->IMSC |= (SSPIMSC_RXIM|SSPIMSC_RTIM); //when RX is half full or a timeout, get the bytes
	spi_local[port].ret = size;

	return 0;
}




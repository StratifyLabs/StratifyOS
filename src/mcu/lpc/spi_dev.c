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
#include "mcu/spi.h"
#include "mcu/pio.h"
#include "spi_flags.h"
#include "ssp_flags.h"
#include "mcu/core.h"

#include "mcu/debug.h"

#if MCU_SPI_PORTS > 0

extern void _mcu_ssp_dev_power_on(int port);
extern void _mcu_ssp_dev_power_off(int port);
extern int _mcu_ssp_dev_powered_on(int port);
extern int _mcu_ssp_dev_read(const device_cfg_t * cfg, device_transfer_t * rop);
extern int _mcu_ssp_dev_write(const device_cfg_t * cfg, device_transfer_t * wop);

typedef struct {
	char * volatile rx_buf;
	char * volatile tx_buf;
	volatile int size;
	void * duplex_mem;
	int ret;
	uint8_t pin_assign;
	uint8_t width;
	uint8_t ref_count;
	mcu_callback_t callback;
	void * context;
} spi_local_t;



static spi_local_t spi_local MCU_SYS_MEM;
static void exec_callback(void * data);

static int spi_port_transfer(int is_read, device_transfer_t * dop);
static int byte_swap(int port, int byte);

void _mcu_spi_dev_power_on(int port){
	if( port != 2 ){
		_mcu_ssp_dev_power_on(port);
		return;
	}
	if ( spi_local.ref_count == 0 ){
		switch(port){
		case 2:
			_mcu_lpc_core_enable_pwr(PCSPI);
			_mcu_core_priv_enable_irq((void*)SPI_IRQn);
			break;
		}
		spi_local.duplex_mem = NULL;
		spi_local.callback = NULL;
	}
	spi_local.ref_count++;

}

void _mcu_spi_dev_power_off(int port){
	if( port != 2 ){
		_mcu_ssp_dev_power_off(port);
		return;
	}
	if ( spi_local.ref_count > 0 ){
		if ( spi_local.ref_count == 1 ){
			_mcu_core_priv_disable_irq((void*)SPI_IRQn);
			_mcu_lpc_core_disable_pwr(PCSPI);
		}
		spi_local.ref_count--;
	}
}

int _mcu_spi_dev_powered_on(int port){
	if( port != 2 ){
		return _mcu_ssp_dev_powered_on(port);
	}
	return ( spi_local.ref_count != 0 );
}


int mcu_spi_getattr(int port, void * ctl){
	if( port != 2 ){
		return mcu_ssp_getattr(port, ctl);
	}

	spi_attr_t * ctlp = (spi_attr_t*)ctl;
	ctlp->pin_assign = spi_local.pin_assign;


	//Master
	if ( LPC_SPI->SPCR & (1<<5) ){
		//Slave
		ctlp->master = SPI_ATTR_MASTER;
	} else {
		ctlp->master = SPI_ATTR_SLAVE;
	}

	//width
	ctlp->width = (LPC_SPI->SPCR >> 8) & 0x0F;
	if ( ctlp->width == 0 ){
		ctlp->width = 16;
	}

	//format
	ctlp->format = SPI_ATTR_FORMAT_SPI;

	//Mode
	ctlp->mode = (LPC_SPI->SPCR >> 3) & 0x03;

	//bitrate
	if ( LPC_SPI->SPCCR < 8 ){
		ctlp->bitrate = -1;
	} else {
		ctlp->bitrate = mcu_board_config.core_periph_freq / (LPC_SPI->SPCCR);
	}


	return 0;
}

int mcu_spi_setattr(int port, void * ctl){
	if( port != 2 ){
		return mcu_ssp_setattr(port, ctl);
	}
	uint32_t cr0, cpsr;
	uint32_t tmp;
	spi_attr_t * ctlp = (spi_attr_t*)ctl;


	if ( ctlp->bitrate == 0 ) {
		errno = EINVAL;
		return -1 - offsetof(spi_attr_t, bitrate);
	}

	if ( ctlp->mode >= 4 ) {
		errno = EINVAL;
		return -1 - offsetof(spi_attr_t, mode);
	}

	if ( ctlp->master != SPI_ATTR_MASTER ){
		errno = EINVAL;
		return -1 - offsetof(spi_attr_t, master);
	}

	if( ctlp->width > 8 ){
		errno = EINVAL;
		return -1 - offsetof(spi_attr_t, width);
	}


	//2 uses the SPI
	cr0 = (1<<2);

	tmp = mcu_board_config.core_periph_freq / ctlp->bitrate;
	tmp = ( tmp > 255 ) ? 254 : tmp;
	tmp = ( tmp < 8 ) ? 8 : tmp;
	if ( tmp & 0x01 ){
		tmp++; //round the divisor up so that actual is less than the target
	}
	cpsr = (tmp & 0xFE);

	cr0 |= ( ctlp->mode << 3);
	cr0 |= (1<<5);  //must be a master


	if ( ctlp->width >= 8 && ctlp->width <= 16 ){
		cr0 |= (( ctlp->width & 0x0F ) << 8);
	} else {
		errno = EINVAL;
		return -1;
	}

	if ( ctlp->format != SPI_ATTR_FORMAT_SPI ){
		errno = EINVAL;
		return -1;
	}

	switch(ctlp->pin_assign){
	case 0:
		_mcu_spi_cfg_pio(2, 0, 18, 17, 15);
		break;
	case MCU_GPIO_CFG_USER:
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	LPC_SPI->SPCCR = cpsr & 0xFE;
	LPC_SPI->SPCR = cr0;


	spi_local.pin_assign = ctlp->pin_assign;

	return 0;
}

int mcu_spi_swap(int port, void * ctl){
	if( port != 2 ){
		return mcu_ssp_swap(port, ctl);
	}
	return byte_swap(port, (int)ctl);
}

int mcu_spi_setduplex(int port, void * ctl){
	if( port != 2 ){
		return mcu_ssp_setduplex(port, ctl);
	}
	spi_local.duplex_mem = (void * volatile)ctl;
	return 0;
}

void exec_callback(void * data){
	LPC_SPI->SPCR &= ~(SPIE); //disable the interrupt
	if ( spi_local.callback != NULL ){
		if( spi_local.callback(spi_local.context, data) == 0 ){
			spi_local.callback = NULL;
		}
	}
}

int mcu_spi_setaction(int port, void * ctl){
	if( port != 2 ){
		return mcu_ssp_setaction(port, ctl);
	}
	mcu_action_t * action = (mcu_action_t*)ctl;
	if( action->callback == 0 ){
		//cancel any ongoing operation
		if ( LPC_SPI->SPCR & (SPIE) ){
			exec_callback(DEVICE_OP_CANCELLED);
		}
	}

	spi_local.callback = action->callback;
	spi_local.context = action->context;
	return 0;
}


int byte_swap(int port, int byte){
	LPC_SPI->SPDR = byte; //start the next transfer
	while( !(LPC_SPI->SPSR) ); //wait for transfer to complete
	return LPC_SPI->SPDR;
}

int _mcu_spi_dev_write(const device_cfg_t * cfg, device_transfer_t * wop){
	if( cfg->periph.port != 2 ){
		return _mcu_ssp_dev_write(cfg, wop);
	}
	return spi_port_transfer(0, wop);
}

int _mcu_spi_dev_read(const device_cfg_t * cfg, device_transfer_t * rop){
	if( cfg->periph.port != 2 ){
		return _mcu_ssp_dev_read(cfg, rop);
	}
	return spi_port_transfer(1, rop);
}

void _mcu_core_spi_isr(void){
	int tmp;

	(volatile int)LPC_SPI->SPSR;
	//! \todo Check for errors

	tmp = LPC_SPI->SPDR;
	if ( spi_local.rx_buf != NULL ){
		*spi_local.rx_buf++ = tmp;
	}

	if ( spi_local.size ){
		spi_local.size--;
	}

	if ( spi_local.size ){
		if ( spi_local.tx_buf != NULL ){
			tmp = *spi_local.tx_buf++;
		} else {
			tmp = 0xFF;
		}

		LPC_SPI->SPDR = tmp;
	}

	LPC_SPI->SPINT |= (SPIF_INT); //clear the interrupt flag

	if ( spi_local.size == 0 ){
		exec_callback(0);
	}
}

int spi_port_transfer(int is_read, device_transfer_t * dop){
	int size;
	size = dop->nbyte;

	if ( LPC_SPI->SPCR & (SPIE) ){
		errno = EAGAIN;
		return -1;
	}

	if ( size == 0 ){
		return 0;
	}

	if ( is_read ){
		spi_local.rx_buf = dop->buf;
		spi_local.tx_buf = spi_local.duplex_mem;
	} else {
		spi_local.tx_buf = dop->buf;
		spi_local.rx_buf = spi_local.duplex_mem;
	}
	spi_local.size = size;
	spi_local.callback = dop->callback;
	spi_local.context = dop->context;
	LPC_SPI->SPCR |= SPIE; //enable the interrupt
	if ( spi_local.tx_buf ){
		//! \todo This won't handle spi widths other than 8 bits
		LPC_SPI->SPDR = *spi_local.tx_buf++;
	} else {
		LPC_SPI->SPDR = 0xFF;
	}

	spi_local.ret = size;

	return 0;
}

#endif


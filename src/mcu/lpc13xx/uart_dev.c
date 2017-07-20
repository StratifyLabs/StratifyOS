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
#include "mcu/uart.h"
#include "mcu/gpio.h"
#include "mcu/circ_buf.h"

static void write_tx_data(int port);
static void read_rx_data(int port);

LPC_UART_Type * const uart_regs_table[MCU_UART_PORTS] = MCU_UART_REGS;

// Interrupt Enable Register bit definitions
#define UIER_RBRIE			(1 << 0) 	// Enable data received interrupt
#define UIER_ERBFI          (1 << 0)    // Enable Receive Data Available Interrupt
#define UIER_ETBEI          (1 << 1)    // Enable Transmit Holding Register Empty Interrupt
#define UIER_ELSI           (1 << 2)    // Enable Receive Line Status Interrupt

// Interrupt ID Register bit definitions
#define UIIR_NO_INT         (1 << 0)    // NO INTERRUPTS PENDING
#define UIIR_MS_INT         (0 << 1)    // MODEM Status
#define UIIR_THRE_INT       (1 << 1)    // Transmit Holding Register Empty
#define UIIR_RDA_INT        (2 << 1)    // Receive Data Available
#define UIIR_RLS_INT        (3 << 1)    // Receive Line Status
#define UIIR_CTI_INT        (6 << 1)    // Character Timeout Indicator
#define UIIR_ID_MASK        0x0E

// FIFO Control Register bit definitions
#define UFCR_FIFO_ENABLE    (1 << 0)    // FIFO Enable
#define UFCR_RX_FIFO_RESET  (1 << 1)    // Reset Receive FIFO
#define UFCR_TX_FIFO_RESET  (1 << 2)    // Reset Transmit FIFO
#define UFCR_FIFO_TRIG1     (0 << 6)    // Trigger @ 1 character in FIFO
#define UFCR_FIFO_TRIG4     (1 << 6)    // Trigger @ 4 characters in FIFO
#define UFCR_FIFO_TRIG8     (2 << 6)    // Trigger @ 8 characters in FIFO
#define UFCR_FIFO_TRIG14    (3 << 6)    // Trigger @ 14 characters in FIFO

// Line Control Register bit definitions
#define ULCR_CHAR_5         (0 << 0)    // 5-bit character length
#define ULCR_CHAR_6         (1 << 0)    // 6-bit character length
#define ULCR_CHAR_7         (2 << 0)    // 7-bit character length
#define ULCR_CHAR_8         (3 << 0)    // 8-bit character length
#define ULCR_STOP_1         (0 << 2)    // 1 stop bit
#define ULCR_STOP_2         (1 << 2)    // 2 stop bits
#define ULCR_PAR_NO         (0 << 3)    // No Parity
#define ULCR_PAR_ODD        (1 << 3)    // Odd Parity
#define ULCR_PAR_EVEN       (3 << 3)    // Even Parity
#define ULCR_PAR_MARK       (5 << 3)    // MARK "1" Parity
#define ULCR_PAR_SPACE      (7 << 3)    // SPACE "0" Paruty
#define ULCR_BREAK_ENABLE   (1 << 6)    // Output BREAK line condition
#define ULCR_DLAB_ENABLE    (1 << 7)    // Enable Divisor Latch Access
#define ULCR_RESET_RX		(1 << 1) 	//Reset the RX fifo
#define ULCR_RESET_TX		(1 << 2) 	//Reset the RX fifo

// Modem Control Register bit definitions
#define UMCR_DTR            (1 << 0)    // Data Terminal Ready
#define UMCR_RTS            (1 << 1)    // Request To Send
#define UMCR_LB             (1 << 4)    // Loopback

// Line Status Register bit definitions
#define ULSR_RDR            (1 << 0)    // Receive Data Ready
#define ULSR_OE             (1 << 1)    // Overrun Error
#define ULSR_PE             (1 << 2)    // Parity Error
#define ULSR_FE             (1 << 3)    // Framing Error
#define ULSR_BI             (1 << 4)    // Break Interrupt
#define ULSR_THRE           (1 << 5)    // Transmit Holding Register Empty
#define ULSR_TEMT           (1 << 6)    // Transmitter Empty
#define ULSR_RXFE           (1 << 7)    // Error in Receive FIFO
#define ULSR_ERR_MASK       0x1E

// Modem Status Register bit definitions
#define UMSR_DCTS           (1 << 0)    // Delta Clear To Send
#define UMSR_DDSR           (1 << 1)    // Delta Data Set Ready
#define UMSR_TERI           (1 << 2)    // Trailing Edge Ring Indicator
#define UMSR_DDCD           (1 << 3)    // Delta Data Carrier Detect
#define UMSR_CTS            (1 << 4)    // Clear To Send
#define UMSR_DSR            (1 << 5)    // Data Set Ready
#define UMSR_RI             (1 << 6)    // Ring Indicator
#define UMSR_DCD            (1 << 7)    // Data Carrier Detect

#define UFIFOLVL_TX_MASK (0x0F00)
#define UFIFOLVL_TX(x) ( x >> 8)

#define UART_TX_FIFO_SIZE 16


//Transmit Enable
#define UTER_TXEN			(1 << 7) 	//Transmit Enable

typedef struct {
	mcu_callback_t write_callback;
	void * write_context;
	mcu_callback_t read_callback;
	void * read_context;
	uart_attr_t attr;
	char * rx_bufp;
	int * rx_nbyte;
	volatile int rx_len;
	const char * tx_bufp;
	volatile int tx_len;
	uint8_t ref_count;
} uart_local_t;

static uart_local_t uart_local[MCU_UART_PORTS] MCU_SYS_MEM;

int _mcu_uart_dev_port_is_invalid(int port){
	if ( (uint32_t)port > MCU_UART_PORTS ){
		return 1;
	}
	return 0;
}

void _mcu_uart_dev_power_on(int port){
	if( uart_local[port].ref_count == 0 ){
		_mcu_lpc_core_enable_pwr(PCUART);
		uart_local[port].rx_bufp = NULL;
		uart_local[port].tx_bufp = NULL;
		mcu_uart_clear(port, NULL);
	}
	uart_local[port].ref_count++;
}

void _mcu_uart_dev_power_off(int port){
	if( uart_local[port].ref_count > 0 ){
		uart_local[port].ref_count--;
		if( uart_local[port].ref_count == 0 ){
			_mcu_cortexm_priv_disable_irq((void*)UART_IRQn);
			_mcu_lpc_core_disable_pwr(PCUART);
		}
	}
}

int _mcu_uart_dev_powered_on(int port){
	if ( uart_local[port].ref_count > 0 ){ return 1; }
	return 0;
}

int mcu_uart_getinfo(int port, void * ctl){
	memcpy(ctl, &(uart_local[port].attr), sizeof(uart_attr_t));
	return 0;
}

int mcu_uart_setattr(int port, void * ctl){
	uint32_t baud_rate;
	uint8_t baud_low;
	uint8_t baud_high;
	uint8_t lcr;
	uint32_t f_div;
	LPC_UART_Type * uart_regs;
	uart_attr_t * ctl_ptr = (uart_attr_t*)ctl;
	uart_regs = uart_regs_table[port];

	if ( ctl_ptr->baudrate != 0 ){
		baud_rate = ctl_ptr->baudrate;
	} else {
		errno = EINVAL;
		return -1;
	}

	lcr = 0;

	switch(ctl_ptr->stop){
	case UART_ATTR_STOP_BITS_1:
		lcr |= ULCR_STOP_1;
		break;
	case UART_ATTR_STOP_BITS_2:
		lcr |= ULCR_STOP_2;
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	if ( (ctl_ptr->width > 8) || (ctl_ptr->width < 5) ){
		errno = EINVAL;
		return -1;
	} else {
		lcr |= (ctl_ptr->width - 5);
	}


	switch(ctl_ptr->parity){
	case UART_PARITY_NONE:
		lcr |= ULCR_PAR_NO;
		break;
	case UART_PARITY_ODD:
		lcr |= ULCR_PAR_ODD;
		break;
	case UART_PARITY_EVEN:
		lcr |= ULCR_PAR_EVEN;
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	//configure pins
	if( ctl_ptr->pin_assign != MCU_GPIO_CFG_USER ){
		uart_cfg_gpio(0, 1, 6, 7); //pins must be configured before enabling the clock
	}

	LPC_SYSCON->SYSAHBCLKCTRL |= (SYSAHBCLKCTRL_UART);
	LPC_SYSCON->UARTCLKDIV = 1;

	_mcu_cortexm_priv_disable_irq((void*)(UART_IRQn));

	uart_regs->RBR;
	uart_regs->IIR;

	uart_regs->LCR = ULCR_DLAB_ENABLE;
	f_div =  ( _mcu_core_getclock() + 8*baud_rate) / ( baud_rate * 16) ;	// calculate the divisor
	baud_low = f_div & 0xFF;
	baud_high = (f_div >> 8) & 0xFF;
	uart_regs->DLM = baud_high;
	uart_regs->DLL = baud_low;

	//LCR controls width, parity, and stop bits
	uart_regs->LCR = lcr;

	//configure the TX/RX FIFOs
	uart_regs->FCR = UFCR_FIFO_TRIG14|
			UFCR_FIFO_ENABLE|
			ULCR_RESET_RX|
			ULCR_RESET_TX;


	uart_regs->LSR; //read the LSR

	//wait for data pending transmit
	while (( uart_regs->LSR & (ULSR_THRE|ULSR_TEMT)) != (ULSR_THRE|ULSR_TEMT) );
	while ( uart_regs->LSR & ULSR_RDR ){
		uart_regs->RBR;	// Dump data from RX FIFO
	}

	uart_regs->IER = 0; //disable the TX/RX interrupts

	memcpy(&(uart_local[port].attr), ctl, sizeof(uart_attr_t));
	//! \todo need to store actual baud rate not target baud rate

	_mcu_cortexm_priv_enable_irq((void*)UART_IRQn);

	return 0;
}

int mcu_uart_setaction(int port, void * ctl){
	mcu_action_t * action = (mcu_action_t*)ctl;
	LPC_UART_Type * uart_regs = uart_regs_table[port];
	if( action->o_events == MCU_EVENT_FLAG_DATA_READY ){
		uart_local[port].read_callback = action->handler.callback;
		uart_local[port].read_context = action->handler.context;
		if ( uart_local[port].read_callback != NULL ){
			uart_regs->IER |= (UIER_RBRIE);  //enable the receiver interrupt
			uart_local[port].rx_bufp = NULL;
		} else {
			uart_regs->IER &= (~UIER_RBRIE); //disable the receive interrupt
		}
	} else if ( action->o_events == MCU_EVENT_FLAG_WRITE_COMPLETE ){
		uart_local[port].write_callback = action->handler.callback;
		uart_local[port].write_context = action->handler.context;
	} else {
		errno = EINVAL;
		return -1;
	}
	return 0;
}

int mcu_uart_clear(int port, void * ctl){
	return 0;
}

int mcu_uart_flush(int port, void * ctl){
	LPC_UART_Type * uart_regs = uart_regs_table[port];
	uart_regs->RBR;
	uart_regs->FCR = UFCR_RX_FIFO_RESET|UFCR_TX_FIFO_RESET;
	return 0;
}


int mcu_uart_get(int port, void * ctl){
	char * dest;
	LPC_UART_Type * uart_regs = uart_regs_table[port];
	if( uart_regs->LSR & ULSR_RDR ){ //check to see if a byte is available
		dest = ctl;
		*dest = uart_regs->RBR;
		return 0;
	}
	return -1;
}


int _mcu_uart_dev_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	int len;
	int port;
	LPC_UART_Type * uart_regs;

	//grab the port and registers
	port = DEVFS_GET_PORT(cfg);
	uart_regs = uart_regs_table[port];

	if ( uart_regs->IER & UIER_RBRIE ){
		errno = EAGAIN;
		return -1;
	}

	//initialize the transfer
	uart_local[port].rx_bufp = rop->buf;
	uart_local[port].rx_len = rop->nbyte;
	uart_local[port].rx_nbyte = &rop->nbyte;

	//Check the local buffer for bytes that are immediately available
	read_rx_data(port);
	len = rop->nbyte - uart_local[port].rx_len;
	if ( len == 0 ){  //nothing available to read
		if ( rop->flags & O_NONBLOCK ){
			uart_local[port].read_callback = NULL;
			uart_local[port].rx_bufp = NULL;
			rop->nbyte = 0;
			errno = EAGAIN;
			len = -1;
		} else {
			uart_local[port].read_callback = rop->handler.callback;
			uart_local[port].read_context = rop->handler.context;
			uart_regs->IER |= UIER_RBRIE;  //enable the receiver interrupt
		}
	}
	return len;
}

int _mcu_uart_dev_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	LPC_UART_Type * uart_regs;
	int port;
	port = DEVFS_GET_PORT(cfg);

	if ( wop->nbyte == 0 ){
		return 0;
	}

	//Grab the registers
	uart_regs = uart_regs_table[port];

	//Check to see if the port is busy
	if ( uart_regs->IER & UIER_ETBEI ){
		errno = EAGAIN;
		return -1;
	}

	//Initialize variables
	uart_local[port].tx_bufp = wop->buf;
	uart_local[port].tx_len = wop->nbyte;

	uart_regs->TER = 0; //disable the transmitter
	write_tx_data(port);
	uart_local[port].write_callback = wop->handler.callback;
	uart_local[port].write_context = wop->handler.context;
	uart_regs->IER |= UIER_ETBEI;  //enable the transmit interrupt
	uart_regs->TER = UTER_TXEN; //enable the transmitter

	return 0;
}

void read_rx_data(int port){
	LPC_UART_Type * uart_regs = uart_regs_table[port];
	while( (uart_local[port].rx_len) && (uart_regs->LSR & ULSR_RDR) ){
		*(uart_local[port].rx_bufp)++ = uart_regs->RBR;
		uart_local[port].rx_len--;
	}
}

void write_tx_data(int port){
	int fifo_cnt;
	fifo_cnt = UART_TX_FIFO_SIZE;
	LPC_UART_Type * uart_regs = uart_regs_table[port];
	while( (uart_local[port].tx_len > 0) && (fifo_cnt > 0) ){
		uart_regs->THR = *(uart_local[port].tx_bufp)++;
		fifo_cnt--;
		uart_local[port].tx_len--;
	}
}

void _mcu_core_uart_isr(){
	const int port = 0;
	//first determine if this is a UART0 interrupt
	LPC_UART_Type * uart_regs = uart_regs_table[port];
	uart_regs->IIR;

	//Check for incoming bytes
	if( uart_regs->LSR & ULSR_RDR ){ //this is the RX FIFO ready flag -- read all the bytes in the FIFO before exiting
		//todo check for errors with this byte

		if ( uart_local[port].rx_bufp != NULL ){
			read_rx_data(port);
			*(uart_local[port].rx_nbyte) = *(uart_local[port].rx_nbyte) - uart_local[port].rx_len;
			uart_local[port].rx_bufp = NULL;
		}

		if ( uart_local[port].read_callback != NULL ){
			if( uart_local[port].read_callback(uart_local[port].read_context, NULL) == 0 ){
				uart_local[port].read_callback = NULL;
			}
		}

		//if the callback is NULL now, disable the interrupt
		if( uart_local[port].read_callback == NULL ){
			uart_regs->IER &= ~UIER_RBRIE; //disable the receive interrupt
		}
	}


	if( ( (uart_regs->LSR & ULSR_THRE)) && (uart_regs->IER & UIER_ETBEI) ){ //THR is empty and interrupt enabled
		uart_regs->TER = 0; //disable the transmitter
		//check for outgoing bytes
		if ( uart_local[port].tx_len == 0 ){

			//no more outgoing bytes so operation is complete
			uart_local[port].tx_bufp = NULL;
			//call the write callback
			if ( uart_local[port].write_callback != NULL ){
				if( uart_local[port].write_callback(uart_local[port].write_context, NULL) == 0 ){
					uart_local[port].write_callback = NULL;
				}
			}

			//if the callback is NULL now, disable the interrupt
			if( uart_local[port].write_callback == NULL ){
				uart_regs->IER &= ~UIER_ETBEI; //disable the transmit interrupt
			}
		} else if( uart_local[port].tx_bufp != NULL ){
			write_tx_data(port);
		}

		uart_regs->TER = UTER_TXEN; //enable the transmitter
	}
}












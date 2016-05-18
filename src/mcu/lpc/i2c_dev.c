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
#include "mcu/i2c.h"
#include "mcu/debug.h"
#include "mcu/core.h"
#include "mcu/pio.h"

#if MCU_I2C_PORTS > 0

#define WRITE_OP 0
#define READ_OP 1

static void enable_opendrain_pin(int pio_port, int pio_pin) MCU_PRIV_CODE;
void enable_opendrain_pin(int pio_port, int pio_pin){
	pio_attr_t pattr;
	pattr.mask = (1<<pio_pin);
	pattr.mode = PIO_MODE_OUTPUT | PIO_MODE_OPENDRAIN;
	mcu_pio_setattr(pio_port, &pattr);
}

static LPC_I2C_Type * const i2c_regs_table[MCU_I2C_PORTS] = MCU_I2C_REGS;

static u8 const i2c_irqs[MCU_I2C_PORTS] = MCU_I2C_IRQS;

static inline LPC_I2C_Type * i2c_get_regs(int port) MCU_ALWAYS_INLINE;
LPC_I2C_Type * i2c_get_regs(int port){
	return i2c_regs_table[port];
}

static void exec_callback(int port, void * data);

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
	i2c_reqattr_t transfer;
	mcu_event_handler_t handler;
} i2c_local_t;

i2c_attr_t i2c_local_attr[MCU_I2C_PORTS] MCU_SYS_MEM;
i2c_local_t i2c_local[MCU_I2C_PORTS] MCU_SYS_MEM;

#define AA (1<<2) //assert acknowledge flag
#define SI (1<<3) //I2C Interrupt flag
#define STO (1<<4) //stop condition
#define STA (1<<5) //start condition
#define I2EN (1<<6) //interface enable

#define I2C_DONE_FLAG (1<<7)


static inline char i2c_tst_done(int port) MCU_ALWAYS_INLINE;
char i2c_tst_done(int port){
	return i2c_local[port].state & I2C_DONE_FLAG;
}

static inline void i2c_clr_done(int port) MCU_ALWAYS_INLINE;
void i2c_clr_done(int port){
	i2c_local[port].state  &= ~(I2C_DONE_FLAG);
}

static int i2c_transfer(int port, int op, device_transfer_t * dop);

#define i2c_slave_ack(port) (LPC_I2C[port].CONSET = I2CONSET_AA)
#define i2c_slave_nack(port) (LPC_I2C[port].CONCLR = I2CONCLR_AAC)
#define i2c_slave_clr_int(port) (LPC_I2C[port].CONCLR = I2CONCLR_SIC)

void _mcu_i2c_dev_power_on(int port){
	if ( i2c_local[port].ref_count == 0 ){
		switch(port){
		case 0:
			_mcu_lpc_core_enable_pwr(PCI2C0);
#if defined __lpc13uxx || __lpc13xx
			LPC_SYSCON->PRESETCTRL |= (1<<1); //de-assert the I2C peripheral reset--applies to I2C and SSP0/1
			LPC_SYSCON->SYSAHBCLKCTRL |= (SYSAHBCLKCTRL_I2C);
#endif
			break;
#if MCU_I2C_PORTS > 1
		case 1:
			_mcu_lpc_core_enable_pwr(PCI2C1);
			break;
#endif
#if MCU_I2C_PORTS > 2
		case 2:
			_mcu_lpc_core_enable_pwr(PCI2C2);
			break;
#endif
		}
		_mcu_core_priv_enable_irq((void*)(u32)(i2c_irqs[port]));
		i2c_local[port].handler.callback = NULL;
	}
	i2c_local[port].ref_count++;
}

void _mcu_i2c_dev_power_off(int port){
	if ( i2c_local[port].ref_count > 0 ){
		if ( i2c_local[port].ref_count == 1 ){
			_mcu_core_priv_disable_irq((void*)(u32)(i2c_irqs[port]));
			switch(port){
			case 0:
#if defined __lpc13uxx || __lpc13xx
				LPC_SYSCON->SYSAHBCLKCTRL &= ~(SYSAHBCLKCTRL_I2C);
#endif
				_mcu_lpc_core_disable_pwr(PCI2C0);
				break;
#if MCU_I2C_PORTS > 1
			case 1:
				_mcu_lpc_core_disable_pwr(PCI2C1);
				break;
#endif
#if MCU_I2C_PORTS > 2
			case 2:
				_mcu_lpc_core_disable_pwr(PCI2C2);
				break;
#endif
			}
		}
		i2c_local[port].ref_count--;
	}
}

int _mcu_i2c_dev_powered_on(int port){
	return ( i2c_local[port].ref_count != 0 );
}

int mcu_i2c_getattr(int port, void * ctl){
	memcpy(ctl, &(i2c_local_attr[port]), sizeof(i2c_attr_t));
	return 0;
}

int mcu_i2c_setattr(int port, void * ctl){
	LPC_I2C_Type * i2c_regs;
	i2c_attr_t * ctl_ptr;
	int count;
#if defined __lpc13uxx || defined __lpc13xx
	pio_attr_t pattr;
	__IO uint32_t * regs_iocon;
#endif

	ctl_ptr = (i2c_attr_t *)ctl;

	//Check for a valid port
	i2c_regs = i2c_regs_table[port];

	if ( ctl_ptr->bitrate == 0 ){
		errno = EINVAL;
		return -1 - offsetof(i2c_attr_t, bitrate);
	}

	if ( ctl_ptr->pin_assign != MCU_GPIO_CFG_USER ){

		switch(port){
		case 0:
			if (1){ //This makes auto-indent work properly with eclipse
				switch(ctl_ptr->pin_assign){
				case 0:

#if defined __lpc13uxx || defined __lpc13xx
					pattr.mask = (1<<4)|(1<<5);
					pattr.mode = PIO_MODE_OUTPUT | PIO_MODE_OPENDRAIN;
					mcu_pio_setattr(port, &pattr);

					regs_iocon = _mcu_get_iocon_regs(0, 4);
					*regs_iocon = 0;
					regs_iocon = _mcu_get_iocon_regs(0, 5);
					*regs_iocon = 0;
#else
					enable_opendrain_pin(MCU_I2C_PORT0_PINASSIGN0,MCU_I2C_SCLPIN0_PINASSIGN0);
					enable_opendrain_pin(MCU_I2C_PORT0_PINASSIGN0,MCU_I2C_SDAPIN0_PINASSIGN0);
#endif


					if ( _mcu_i2c_cfg_pio(0, MCU_I2C_PORT0_PINASSIGN0,
							MCU_I2C_SCLPIN0_PINASSIGN0,
							MCU_I2C_SDAPIN0_PINASSIGN0) < 0 ){
						errno = EINVAL;
						return -1;
					}
					break;
				default:
					errno = EINVAL;
					return -1 - offsetof(i2c_attr_t, pin_assign);
				}
			}
			break;
#if MCU_I2C_PORTS > 1
		case 1:
			if (1){
				switch(ctl_ptr->pin_assign){
				case 0:
					//configure pins as open drain
					enable_opendrain_pin(0,0);
					enable_opendrain_pin(0,1);

					if( _mcu_i2c_cfg_pio(1, 0, 1, 0) < 0 ){
						errno = EINVAL;
						return -1;
					}
					break;
				case 1:
					enable_opendrain_pin(0,19);
					enable_opendrain_pin(0,20);
					if ( _mcu_i2c_cfg_pio(1, 0, 20, 19) < 0 ){
						errno = EINVAL;
						return -1;
					}
					break;
				default:
					errno = EINVAL;
					return -1 - offsetof(i2c_attr_t, pin_assign);
				}
			}
			break;
#endif
#if MCU_I2C_PORTS > 2
		case 2:
			if (1){
				switch(ctl_ptr->pin_assign){
				case 0:
					enable_opendrain_pin(0,10);
					enable_opendrain_pin(0,11);
					_mcu_i2c_cfg_pio(2, 0, 11, 10);
					break;
				default:
					errno = EINVAL;
					return -1 - offsetof(i2c_attr_t, pin_assign);
				}
			}
			break;
#endif
		default:
			errno = EINVAL;
			return -1 - offsetof(i2c_attr_t, pin_assign);
		}
	}

	i2c_local[port].state = I2C_DONE_FLAG; //Set the done flag (not busy)

	count = ((mcu_board_config.core_periph_freq) / (ctl_ptr->bitrate * 2));
	if ( count > 0xFFFF ){
		count = 0xFFFF;
	}
	i2c_regs->SCLH = count;
	i2c_regs->SCLL = count;

	//Enable the I2C unit
	i2c_regs->CONSET = (I2EN);
	memcpy(&(i2c_local_attr[port]), ctl_ptr, sizeof(i2c_attr_t));
	i2c_local_attr[port].bitrate = mcu_board_config.core_periph_freq / (2 * count);
	return 0;
}

int mcu_i2c_setup(int port, void * ctl){
	i2c_reqattr_t * ctl_ptr;
	ctl_ptr = ctl;
	i2c_local[port].transfer.slave_addr = ctl_ptr->slave_addr;
	i2c_local[port].transfer.transfer = ctl_ptr->transfer;
	return 0;
}

int mcu_i2c_geterr(int port, void * ctl){
	return i2c_local[port].err;
}

int mcu_i2c_setaction(int port, void * ctl){
	mcu_action_t * action = (mcu_action_t*)ctl;
	if( action->callback == 0 ){
		if ( !(i2c_local[port].state & I2C_DONE_FLAG) ){
			exec_callback(port, DEVICE_OP_CANCELLED);
		}
	}

	if( _mcu_core_priv_validate_callback(action->callback) < 0 ){
		return -1;
	}

	i2c_local[port].handler.callback = action->callback;
	i2c_local[port].handler.context = action->context;

	_mcu_core_setirqprio(i2c_irqs[port], action->prio);


	return 0;
}

int _mcu_i2c_dev_write(const device_cfg_t * cfg, device_transfer_t * wop){
	int port = DEVICE_GET_PORT(cfg);;
	return i2c_transfer(port, WRITE_OP, wop);
}

int _mcu_i2c_dev_read(const device_cfg_t * cfg, device_transfer_t * rop){
	int port = DEVICE_GET_PORT(cfg);
	return i2c_transfer(port, READ_OP, rop);
}

static void be_done(int port){
	i2c_local[port].state = I2C_DONE_FLAG;
	*(i2c_local[port].ret) = 0;
	_mcu_core_exec_event_handler(&(i2c_local[port].handler), (mcu_event_t)i2c_local[port].err);
}

static void _mcu_i2c_isr(int port) {
	uint8_t stat_value;
	LPC_I2C_Type * i2c_regs;
	// this handler deals with master read and master write only
	i2c_regs = i2c_get_regs(port);

	if ( i2c_regs == NULL ){
		be_done(port);
		return;
	}
	stat_value = i2c_regs->STAT;
	switch ( stat_value ){
	case 0x08: //Start Condition has been sent
	case 0x10: //Repeated Start condition
		i2c_local[port].err = 0;
		if ( i2c_local[port].state == I2C_STATE_NONE){
			i2c_regs->DAT = i2c_local[port].addr | 0x01; //Set the Read bit -- repeated start after write ptr
		} else {
			i2c_regs->DAT = i2c_local[port].addr; //send the address -- in write mode
		}

		i2c_regs->CONSET = AA;
		i2c_regs->CONCLR = STA;
		break;
	case 0x18: //SLA+W transmitted -- Ack Received
		i2c_regs->DAT = i2c_local[port].ptr.ptr8[1]; //Send the offset pointer (MSB of 16 or 8 bit)
		i2c_regs->CONSET = AA;
		break;
	case 0x28: //Data byte transmitted -- Ack Received
		if( i2c_local[port].state == I2C_STATE_WR_PTR_ONLY ){
			i2c_local[port].state = I2C_STATE_NONE | I2C_DONE_FLAG;
			i2c_regs->CONSET = STO|AA;
			break;
		}

		if (( i2c_local[port].state == I2C_STATE_RD_16_OP )|| ( i2c_local[port].state == I2C_STATE_WR_16_OP )){
			i2c_regs->DAT = i2c_local[port].ptr.ptr8[0]; //Send the offset pointer (LSB)
			i2c_regs->CONSET = AA;
			if ( i2c_local[port].state == I2C_STATE_RD_16_OP ){
				i2c_local[port].state = I2C_STATE_RD_OP;
			}
			break;
		}

		if ( i2c_local[port].size ){

			if ( i2c_local[port].state == I2C_STATE_RD_OP ){
				i2c_regs->CONSET = AA|STA; //Restart (then send read command)
				i2c_local[port].state = I2C_STATE_NONE;
				break;
			}

			//Transmit data
			i2c_regs->DAT = *(i2c_local[port].data);
			i2c_local[port].data++;
			i2c_regs->CONSET = AA;
			i2c_local[port].size--;
		} else {
			i2c_regs->CONSET = STO|AA;
			i2c_local[port].state = I2C_STATE_NONE | I2C_DONE_FLAG;
		}
		break;
	case 0x20:
	case 0x30:
	case 0x48:
		//Receiver nack'd
		i2c_regs->CONSET = STO;
		i2c_local[port].size = 0;
		i2c_local[port].err = I2C_ERROR_ACK;
		i2c_local[port].state = I2C_DONE_FLAG;
		break;
	case 0x38:
		i2c_local[port].size = 0;
		i2c_regs->CONSET = STO;
		i2c_local[port].err = I2C_ERROR_ARBITRATION_LOST;
		i2c_local[port].state = I2C_DONE_FLAG;
		break;
	case 0x40: //SLA+R transmitted -- Ack received
		if ( i2c_local[port].size > 1 ){
			i2c_regs->CONSET = AA; //only ACK if more than one byte is coming
		} else {
			i2c_regs->CONCLR = AA;
		}
		break;
	case 0x50: //Data Byte received -- Ack returned
		//Receive Data
		if ( i2c_local[port].size ) i2c_local[port].size--;
		*(i2c_local[port].data) = (char)i2c_regs->DAT;
		i2c_local[port].data++;
		if ( i2c_local[port].size > 1 ){
			i2c_regs->CONSET = AA;
		} else {
			i2c_regs->CONCLR = AA;
		}
		break;
	case 0x58: //Data byte received -- Not Ack returned
		if ( i2c_local[port].size ) i2c_local[port].size--;
		*(i2c_local[port].data) = (char)i2c_regs->DAT;
		i2c_local[port].data++;

		i2c_regs->CONSET = STO;
		i2c_local[port].state = I2C_STATE_NONE | I2C_DONE_FLAG;
		break;
	case 0x00:
		i2c_local[port].err = I2C_ERROR_START;
		i2c_local[port].state = I2C_DONE_FLAG;
		i2c_regs->CONSET = STO;
		break;
	}

	i2c_regs->CONCLR = SI; //clear the interrupt flag

	if ( i2c_local[port].state & I2C_DONE_FLAG ){
		exec_callback(port, 0);
	}
}

void exec_callback(int port, void * data){
	i2c_local[port].state |= I2C_DONE_FLAG;
	if ( i2c_local[port].err != 0 ){
		*(i2c_local[port].ret) = -1;
	}

	_mcu_core_exec_event_handler(&(i2c_local[port].handler), data);
}

int i2c_transfer(int port, int op, device_transfer_t * dop){
	LPC_I2C_Type * i2c_regs;
	int size = dop->nbyte;
	i2c_regs = i2c_get_regs(port);


	if( i2c_local[port].transfer.transfer == I2C_TRANSFER_READ_ONLY){ //writes will always write the loc value -- so no need to abort here
		if ( dop->nbyte == 0 ){
			return 0;
		}
	}

	if ( !(i2c_local[port].state & I2C_DONE_FLAG) ){
		errno = EAGAIN;
		return -1;
	}

	if( _mcu_core_priv_validate_callback(dop->callback) < 0 ){
		return -1;
	}

	i2c_local[port].handler.callback = dop->callback;
	i2c_local[port].handler.context = dop->context;

	i2c_clr_done(port); //clear the done flag
	i2c_local[port].addr = (i2c_local[port].transfer.slave_addr << 1); //set the addr -- 7-bit addr is passed and shifted left 1 bit
	i2c_local[port].data = (volatile void*)dop->buf;
	i2c_local[port].ret = &(dop->nbyte);

	switch(i2c_local[port].transfer.transfer){
	case I2C_TRANSFER_NORMAL:
		i2c_local[port].ptr.ptr8[1] = dop->loc; //8-bit ptr
		i2c_local[port].size = size;
		if ( op == WRITE_OP ){
			i2c_local[port].state = I2C_STATE_WR_OP;
		} else {
			i2c_local[port].state = I2C_STATE_RD_OP;
		}
		break;
	case I2C_TRANSFER_WRITE_PTR:
		i2c_local[port].ptr.ptr8[1] = dop->loc; //8-bit ptr
		i2c_local[port].size = 0;
		i2c_local[port].state = I2C_STATE_WR_PTR_ONLY;
		break;
	case I2C_TRANSFER_NORMAL_16:
		i2c_local[port].ptr.ptr16 = dop->loc;  //16-bit ptr
		i2c_local[port].size = size;
		if ( op == WRITE_OP ){
			i2c_local[port].state = I2C_STATE_WR_16_OP;
		} else {
			i2c_local[port].state = I2C_STATE_RD_16_OP;
		}
		break;
	case I2C_TRANSFER_READ_ONLY:
		i2c_local[port].size = size; //just start reading (no ptr)
		if ( op == READ_OP ){
			i2c_local[port].state = I2C_STATE_NONE;
		} else {
			errno = EINVAL;
			return -1;
		}
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	//Master transmitter mode -- write the pointer value
	i2c_regs->CONSET = STA; //exec start condition
	return 0;
}

void _mcu_core_i2c0_isr(void){ _mcu_i2c_isr(0); }
#if MCU_I2C_PORTS > 1
void _mcu_core_i2c1_isr(void){ _mcu_i2c_isr(1); }
#endif
#if MCU_I2C_PORTS > 2
void _mcu_core_i2c2_isr(void){ _mcu_i2c_isr(2); }
#endif

#endif



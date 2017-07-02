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
#include "mcu/i2c.h"
#include "mcu/debug.h"
#include "mcu/core.h"
#include "mcu/pio.h"

#if MCU_I2C_PORTS > 0

#define WRITE_OP 0
#define READ_OP 1

typedef union {
	u16 ptr16;
	u8 ptr8[2];
} i2c_ptr_t;

typedef struct MCU_PACK {
	mcu_event_handler_t handler;
	char * volatile data;
	int * ret;
	volatile u16 size;
	u16 resd;
} i2c_local_transfer_t;

typedef struct MCU_PACK {
	i2c_slave_setup_t setup;
	mcu_event_handler_t handler;
	i2c_ptr_t ptr;
} i2c_local_slave_t;


typedef struct MCU_PACK {
	u8 ref_count;
	volatile u8 state;
	volatile u8 err;
	u8 resd[1];
	volatile i2c_ptr_t ptr;
	i2c_setup_t transfer;
	i2c_local_transfer_t master;
	i2c_local_slave_t slave;
} i2c_local_t;

static void enable_opendrain_pin(int pio_port, int pio_pin, int internal_pullup) MCU_PRIV_CODE;
void enable_opendrain_pin(int pio_port, int pio_pin, int internal_pullup){
	pio_attr_t pattr;
	pattr.mask = (1<<pio_pin);
	pattr.mode = PIO_MODE_OUTPUT | PIO_MODE_OPENDRAIN | internal_pullup;
	mcu_pio_setattr(pio_port, &pattr);
}

static LPC_I2C_Type * const i2c_regs_table[MCU_I2C_PORTS] = MCU_I2C_REGS;

static u8 const i2c_irqs[MCU_I2C_PORTS] = MCU_I2C_IRQS;

static void set_master_done(LPC_I2C_Type * regs, int port, int error);

static void receive_byte(LPC_I2C_Type * regs, i2c_local_transfer_t * op);
static void transmit_byte(LPC_I2C_Type * regs, i2c_local_transfer_t * op);

static int receive_slave_byte(LPC_I2C_Type * regs, i2c_local_slave_t * op);
static int transmit_slave_byte(LPC_I2C_Type * regs, i2c_local_slave_t * op);


static void set_ack(LPC_I2C_Type * regs, u16 size);
static void set_slave_ack(LPC_I2C_Type * regs, i2c_local_slave_t * slave);

static inline LPC_I2C_Type * i2c_get_regs(int port) MCU_ALWAYS_INLINE;
LPC_I2C_Type * i2c_get_regs(int port){
	return i2c_regs_table[port];
}

i2c_attr_t i2c_local_attr[MCU_I2C_PORTS] MCU_SYS_MEM;
i2c_local_t i2c_local[MCU_I2C_PORTS] MCU_SYS_MEM;

#define AA (1<<2) //assert acknowledge flag
#define SI (1<<3) //I2C Interrupt flag
#define STO (1<<4) //stop condition
#define STA (1<<5) //start condition
#define I2EN (1<<6) //interface enable


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
		memset(&(i2c_local[port].master), 0, sizeof(i2c_local_transfer_t));
		memset(&(i2c_local[port].slave), 0, sizeof(i2c_local_slave_t));
		_mcu_cortexm_priv_enable_irq((void*)(u32)(i2c_irqs[port]));
	}
	i2c_local[port].ref_count++;
}

void _mcu_i2c_dev_power_off(int port){
	LPC_I2C_Type * i2c_regs;
	if ( i2c_local[port].ref_count > 0 ){
		if ( i2c_local[port].ref_count == 1 ){
			i2c_regs = i2c_regs_table[port];
			i2c_regs->CONCLR = (AA);
			i2c_regs->ADR0 = 0;
			i2c_regs->ADR1 = 0;
			i2c_regs->ADR2 = 0;
			i2c_regs->ADR3 = 0;
			_mcu_cortexm_priv_disable_irq((void*)(u32)(i2c_irqs[port]));
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
	int internal_pullup;
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

	if ( ctl_ptr->o_flags & I2C_ATTR_FLAG_PULLUP ){
		internal_pullup = PIO_MODE_PULLUP;
	} else {
		internal_pullup = 0;
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
					enable_opendrain_pin(MCU_I2C_PORT0_PINASSIGN0,MCU_I2C_SCLPIN0_PINASSIGN0,internal_pullup);
					enable_opendrain_pin(MCU_I2C_PORT0_PINASSIGN0,MCU_I2C_SDAPIN0_PINASSIGN0,internal_pullup);
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
					enable_opendrain_pin(0,0,internal_pullup);
					enable_opendrain_pin(0,1,internal_pullup);

					if( _mcu_i2c_cfg_pio(1, 0, 1, 0) < 0 ){
						errno = EINVAL;
						return -1;
					}
					break;
				case 1:
					enable_opendrain_pin(0,19,internal_pullup);
					enable_opendrain_pin(0,20,internal_pullup);
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
					enable_opendrain_pin(0,10,internal_pullup);
					enable_opendrain_pin(0,11,internal_pullup);
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

	i2c_regs->CONCLR = 0xFF;

	i2c_local[port].state = I2C_STATE_NONE;
	i2c_local[port].master.size = 0; //no bytes
	i2c_local[port].master.data = 0; //no data pointer

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
	i2c_setup_t * ctl_ptr;
	ctl_ptr = ctl;
	i2c_local[port].transfer.slave_addr = ctl_ptr->slave_addr;
	i2c_local[port].transfer.transfer = ctl_ptr->transfer;
	return 0;
}

int mcu_i2c_slave_setup(int port, void * ctl){
	i2c_slave_setup_t * ctlp = ctl;
	LPC_I2C_Type * i2c_regs;
	int gen_call = 0;

	// \todo To fix Issue #30, we need to check the dest mem and make sure it is writable by the caller

	memcpy( &(i2c_local[port].slave.setup), ctl, sizeof(i2c_slave_setup_t));

	if( ctlp->size > 255 ){
		i2c_local[port].slave.setup.o_flags |= I2C_SLAVE_SETUP_FLAG_16_BIT_PTR;
	}

	i2c_regs = i2c_regs_table[port];

	if( ctlp->o_flags & I2C_SLAVE_SETUP_FLAG_ENABLE_GENERAL_CALL ){
		gen_call = 1;
	}

	if( i2c_local[port].slave.setup.o_flags & I2C_SLAVE_SETUP_FLAG_ALT_ADDR1 ){
		i2c_regs->ADR1 = (ctlp->addr << 1) | gen_call;
#if MCU_I2C_API == 0
		i2c_regs->MASK1 = 0x00;
#endif
	} else if( i2c_local[port].slave.setup.o_flags & I2C_SLAVE_SETUP_FLAG_ALT_ADDR2 ){
		i2c_regs->ADR2 = (ctlp->addr << 1) | gen_call;
#if MCU_I2C_API == 0
		i2c_regs->MASK2 = 0x00;
#endif
	} else if( i2c_local[port].slave.setup.o_flags & I2C_SLAVE_SETUP_FLAG_ALT_ADDR3 ){
		i2c_regs->ADR3 = (ctlp->addr << 1) | gen_call;
#if MCU_I2C_API == 0
		i2c_regs->MASK3 = 0x00;
#endif
	} else {
		i2c_regs->ADR0 = (ctlp->addr << 1) | gen_call;
#if MCU_I2C_API == 0
		i2c_regs->MASK0 = 0x00;
#endif
	}

	i2c_regs->CONSET = (AA);

	return 0;
}

int mcu_i2c_reset(int port, void * ctl){
	LPC_I2C_Type * i2c_regs;
	i2c_regs = i2c_regs_table[port];

	i2c_regs->CONCLR = 0xFF;
	i2c_regs->CONSET = I2EN;

	return 0;
}

int mcu_i2c_geterr(int port, void * ctl){
	return i2c_local[port].err;
}

int mcu_i2c_setaction(int port, void * ctl){
	mcu_action_t * action = (mcu_action_t*)ctl;

	_mcu_cortexm_set_irq_prio(i2c_irqs[port], action->prio);

	if( action->callback == 0 ){
		i2c_local[port].slave.handler.callback = 0;
		i2c_local[port].slave.handler.context = 0;
		return 0;
	}

	if( _mcu_cortexm_priv_validate_callback(action->callback) < 0 ){
		return -1;
	}

	i2c_local[port].slave.handler.callback = action->callback;
	i2c_local[port].slave.handler.context = action->context;

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

void set_master_done(LPC_I2C_Type * regs, int port, int error){
	if( error != 0 ){
		*(i2c_local[port].master.ret) = -1;
	} else {
		*(i2c_local[port].master.ret) -= i2c_local[port].master.size;
	}
	i2c_local[port].master.size = 0;
	i2c_local[port].err = error;
	i2c_local[port].state = I2C_STATE_MASTER_COMPLETE;
	i2c_local[port].master.data = 0;

	regs->CONSET = STO;

}


void set_ack(LPC_I2C_Type * regs, u16 size){
	if( size > 1 ){
		regs->CONSET = AA;
	} else {
		regs->CONCLR = AA;
	}
}

void receive_byte(LPC_I2C_Type * regs, i2c_local_transfer_t * op){
	if( op->size && op->data ){
		*(op->data) = regs->DAT;
		op->data++;
		op->size--;
	} else {
		regs->DAT;
	}

	set_ack(regs, op->size);
}

void transmit_byte(LPC_I2C_Type * regs, i2c_local_transfer_t * op){
	if( op->size && op->data ){
		regs->DAT = *(op->data);
		op->data++;
		op->size--;
	} else {
		regs->DAT = 0xFF;
	}

	set_ack(regs, op->size);
}

int receive_slave_byte(LPC_I2C_Type * regs, i2c_local_slave_t * op){
	if( op->ptr.ptr16 < op->setup.size ){
		op->setup.data[ op->ptr.ptr16 ] = regs->DAT;
		op->ptr.ptr16++;
	} else {
		regs->DAT;
		return 0;
	}

	return (op->ptr.ptr16+1) < op->setup.size;
}

int transmit_slave_byte(LPC_I2C_Type * regs, i2c_local_slave_t * op){
	if( op->ptr.ptr16 < op->setup.size ){
		regs->DAT = op->setup.data[ op->ptr.ptr16 ];
		op->ptr.ptr16++;
		if( op->ptr.ptr16 == op->setup.size ){
			op->ptr.ptr16 = 0;
		}
	} else {
		regs->DAT = 0xFF;
	}
	return 0;
}

void set_slave_ack(LPC_I2C_Type * regs, i2c_local_slave_t * slave){
	if( slave->setup.size > (slave->ptr.ptr16 + 1) ){
		regs->CONSET = AA;
	} else {
		regs->CONCLR = AA;
	}
}

static void _mcu_i2c_isr(int port) {
	u8 stat_value;
	u32 event;
	LPC_I2C_Type * i2c_regs;
	// this handler deals with master read and master write only
	i2c_regs = i2c_get_regs(port);

	stat_value = i2c_regs->STAT;

	switch ( stat_value ){
	case 0x08: //Start Condition has been sent
	case 0x10: //Repeated Start condition
		i2c_local[port].err = 0;
		if ( i2c_local[port].state == I2C_STATE_START ){
			i2c_regs->DAT = (i2c_local[port].transfer.slave_addr << 1) | 0x01; //Set the Read bit -- repeated start after write ptr
		} else {
			i2c_regs->DAT = (i2c_local[port].transfer.slave_addr << 1); //send the address -- in write mode
		}

		i2c_regs->CONSET = AA;
		i2c_regs->CONCLR = STA;
		break;
	case 0x18: //SLA+W transmitted -- Ack Received
		if( i2c_local[port].state == I2C_STATE_WR_ONLY ){
			//this is write only mode -- don't send the ptr
			transmit_byte(i2c_regs, &(i2c_local[port].master));
		} else {
			i2c_regs->DAT = i2c_local[port].ptr.ptr8[1]; //Send the offset pointer (MSB of 16 or 8 bit)
			i2c_regs->CONSET = AA;
		}
		break;
	case 0x20: //SLA+W has been transmitted; NOT ACK has been received
		set_master_done(i2c_regs, port, I2C_ERROR_ACK);
		break;
	case 0x28: //Data byte transmitted -- Ack Received
		if( i2c_local[port].state == I2C_STATE_WR_PTR_ONLY ){
			i2c_local[port].state = I2C_STATE_MASTER_COMPLETE;
			i2c_regs->CONSET = STO|AA;
			break;
		} else if (( i2c_local[port].state == I2C_STATE_RD_16_OP ) || ( i2c_local[port].state == I2C_STATE_WR_16_OP )){
			i2c_regs->DAT = i2c_local[port].ptr.ptr8[0]; //Send the offset pointer (LSB)
			i2c_regs->CONSET = AA;
			if ( i2c_local[port].state == I2C_STATE_RD_16_OP ){ i2c_local[port].state = I2C_STATE_RD_OP; }
			if ( i2c_local[port].state == I2C_STATE_WR_16_OP ){ i2c_local[port].state = I2C_STATE_WR_OP; }
			break;
		}

		if ( i2c_local[port].master.size ){

			if ( i2c_local[port].state == I2C_STATE_RD_OP ){
				i2c_regs->CONSET = STA; //Restart (then send read command)
				i2c_local[port].state = I2C_STATE_START;
				break;
			}

			//Transmit data
			transmit_byte(i2c_regs, &(i2c_local[port].master));

		} else {
			set_master_done(i2c_regs, port, 0);
		}
		break;
	case 0x48: //SLA+R has been transmitted; NOT ACK has been received.
		set_master_done(i2c_regs, port, I2C_ERROR_ACK);
		break;
	case 0x30: //Data byte in I2DAT has been transmitted; NOT ACK has been received.
		set_master_done(i2c_regs, port, 0);
		break;
	case 0x38:
		set_master_done(i2c_regs, port, I2C_ERROR_ARBITRATION_LOST);
		break;
	case 0x40: //SLA+R transmitted -- Ack received
		set_ack(i2c_regs, i2c_local[port].master.size);
		break;
	case 0x50: //Data Byte received -- Ack returned
		//Receive Data
		receive_byte(i2c_regs, &(i2c_local[port].master));
		break;
	case 0x58: //Data byte received -- Not Ack returned
		receive_byte(i2c_regs, &(i2c_local[port].master));
		set_master_done(i2c_regs, port, 0);
		break;

	case 0x00: //Bus error in Master or selected slave mode -- illegal start or stop
		if( (i2c_local[port].state >= I2C_STATE_START) &&
				(i2c_local[port].state <= I2C_STATE_MASTER_COMPLETE)){
			set_master_done(i2c_regs, port, I2C_ERROR_BUS_BUSY);
		} else if( (i2c_local[port].state >= I2C_STATE_SLAVE_READ) &&
				(i2c_local[port].state <= I2C_STATE_SLAVE_WRITE_COMPLETE)){
			i2c_regs->CONSET = STO;
			_mcu_cortexm_execute_event_handler(&(i2c_local[port].slave.handler), MCU_EVENT_SET_CODE(I2C_EVENT_SLAVE_BUS_ERROR));
		}

		break;

		//SLAVE OPERATION STARTS HERE ---------------------------------------------------------------
	case 0x68: //Arbitration lost in SLA+R/W as master; Own SLA+W has been received, ACK returned
	case 0x60: //Slave mode SLA+W has been received -- Ack returned
	case 0x78: //Arbitration lost and general ack received
	case 0x70: //General call has been received -- ack returned
		if( i2c_local[port].slave.setup.o_flags & I2C_SLAVE_SETUP_FLAG_16_BIT_PTR ){
			i2c_local[port].state = I2C_STATE_SLAVE_READ_PTR_16;
		} else {
			i2c_local[port].state = I2C_STATE_SLAVE_READ_PTR;
		}
		i2c_local[port].slave.ptr.ptr16 = 0;
		i2c_regs->CONSET = AA;
		break;

	case 0x90: //data has been received and ack returned on general call
	case 0x80: //data has been received and ack has been returned
		if( i2c_local[port].state == I2C_STATE_SLAVE_READ_PTR_16 ){
			i2c_local[port].slave.ptr.ptr8[1] = i2c_regs->DAT; //MSB is first
			i2c_local[port].state = I2C_STATE_SLAVE_READ_PTR;
		} else if( i2c_local[port].state == I2C_STATE_SLAVE_READ_PTR ){
			i2c_local[port].slave.ptr.ptr8[0] = i2c_regs->DAT; //LSB is second
			i2c_local[port].state = I2C_STATE_SLAVE_READ_PTR_COMPLETE;
		} else {
			i2c_local[port].state = I2C_STATE_SLAVE_READ;
			receive_slave_byte(i2c_regs, &(i2c_local[port].slave));
		}
		set_slave_ack(i2c_regs, &(i2c_local[port].slave));

		break;

	case 0x88: //data has been received and NOT ack has been returned
	case 0x98: //data has been received and NOT ack has been returned on general call address
		receive_slave_byte(i2c_regs, &(i2c_local[port].slave));

		//keep slave address active
		i2c_regs->CONSET = AA;
		i2c_local[port].state = I2C_STATE_SLAVE_READ_COMPLETE;
		//the device is no longer addressed and won't interrupt when the stop condition occurs
		_mcu_cortexm_execute_event_handler(&(i2c_local[port].slave.handler), MCU_EVENT_SET_CODE(I2C_EVENT_DATA_READY));
		break;

	case 0xA8: //Own SLA+R has been received and ack returned
	case 0xB0: //Arbitration lost in SLA+R/W as master; Own SLA+R has been received, ACK has been returned.
	case 0xB8: //Data byte in I2DAT has been transmitted; ACK has been received.
		//load byte and send ack
		i2c_local[port].state = I2C_STATE_SLAVE_WRITE; //Slave is being read (host will write the data)
		transmit_slave_byte(i2c_regs, &(i2c_local[port].slave));
		i2c_regs->CONSET = AA;

		break;

	case 0xC0: //Data byte in I2DAT has been transmitted; NOT ACK has been received.
	case 0xC8: //Last data byte in I2DAT has been transmitted (AA = 0); ACK has been received.
		//set ack to stay in slave mode
		i2c_local[port].state = I2C_STATE_SLAVE_WRITE_COMPLETE;
		i2c_regs->CONSET = AA;

		//the device is no longer addressed and won't interrupt when the stop condition occurs
		_mcu_cortexm_execute_event_handler(&(i2c_local[port].slave.handler), MCU_EVENT_SET_CODE(I2C_EVENT_WRITE_COMPLETE));
		break;

	case 0xA0: //stop or restart has been received while addressed

		//execute read and write callbacks
		event = 0;
		if( (i2c_local[port].state == I2C_STATE_SLAVE_WRITE_COMPLETE) ||
				(i2c_local[port].state == I2C_STATE_SLAVE_WRITE) ){
			event = I2C_EVENT_WRITE_COMPLETE;
		} else if( (i2c_local[port].state == I2C_STATE_SLAVE_READ_COMPLETE) ||
				(i2c_local[port].state == I2C_STATE_SLAVE_READ) ){
			event = I2C_EVENT_DATA_READY;
		} else {
			event = I2C_EVENT_UPDATE_POINTER_COMPLETE;
		}

		i2c_regs->CONSET = AA;

		_mcu_cortexm_execute_event_handler(&(i2c_local[port].slave.handler), MCU_EVENT_SET_CODE(event));

		break;
	}

	i2c_regs->CONCLR = SI; //clear the interrupt flag

	if ( i2c_local[port].state == I2C_STATE_MASTER_COMPLETE ){
		i2c_local[port].state = I2C_STATE_NONE;
		_mcu_cortexm_execute_event_handler(&(i2c_local[port].master.handler), &(i2c_local[port].transfer));
	}
}


int i2c_transfer(int port, int op, device_transfer_t * dop){
	LPC_I2C_Type * i2c_regs;
	int size = dop->nbyte;
	i2c_regs = i2c_get_regs(port);

	//writes will always write the loc value -- so only abort on read only
	if( i2c_local[port].transfer.transfer == I2C_TRANSFER_READ_ONLY){
		if ( dop->nbyte == 0 ){
			return 0;
		}
	}

	if ( i2c_local[port].state != I2C_STATE_NONE ){
		errno = EAGAIN;
		return -1;
	}
	i2c_local[port].state = I2C_STATE_START;


	if( _mcu_cortexm_priv_validate_callback(dop->callback) < 0 ){
		errno = EPERM;
		return -1;
	}

	i2c_local[port].master.handler.callback = dop->callback;
	i2c_local[port].master.handler.context = dop->context;
	i2c_local[port].master.data = (void * volatile)dop->buf;
	i2c_local[port].master.ret = &(dop->nbyte);

	switch(i2c_local[port].transfer.transfer){
	case I2C_TRANSFER_NORMAL:
		i2c_local[port].ptr.ptr8[1] = dop->loc; //8-bit ptr
		i2c_local[port].master.size = size;
		if ( op == WRITE_OP ){
			i2c_local[port].state = I2C_STATE_WR_OP;
		} else {
			i2c_local[port].state = I2C_STATE_RD_OP;
		}
		break;
	case I2C_TRANSFER_WRITE_PTR:
		i2c_local[port].ptr.ptr8[1] = dop->loc; //8-bit ptr
		i2c_local[port].master.size = 0;
		i2c_local[port].state = I2C_STATE_WR_PTR_ONLY;
		break;
	case I2C_TRANSFER_NORMAL_16:
		i2c_local[port].ptr.ptr16 = dop->loc;  //16-bit ptr
		i2c_local[port].master.size = size;
		if ( op == WRITE_OP ){
			i2c_local[port].state = I2C_STATE_WR_16_OP;
		} else {
			i2c_local[port].state = I2C_STATE_RD_16_OP;
		}
		break;
	case I2C_TRANSFER_DATA_ONLY:
	case I2C_TRANSFER_WRITE_ONLY:
	case I2C_TRANSFER_READ_ONLY:
		i2c_local[port].master.size = size;
		if ( op == WRITE_OP ){
			i2c_local[port].state = I2C_STATE_WR_ONLY;
		} else {
			i2c_local[port].state = I2C_STATE_START;
		}
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	//Master transmitter mode
	i2c_regs->CONSET = STA; //exec start condition
	return 0;
}

void _mcu_core_i2c0_isr(){ _mcu_i2c_isr(0); }
#if MCU_I2C_PORTS > 1
void _mcu_core_i2c1_isr(){ _mcu_i2c_isr(1); }
#endif
#if MCU_I2C_PORTS > 2
void _mcu_core_i2c2_isr(){ _mcu_i2c_isr(2); }
#endif

#endif



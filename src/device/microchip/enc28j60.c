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
#include <sys/types.h>
#include "mcu/spi.h"
#include "mcu/pio.h"
#include "iface/dev/microchip/enc28j60.h"
#include "device/microchip/enc28j60.h"


typedef struct MCU_PACK {
	uint16_t next_pkt;
	uint16_t size;
	uint16_t flags;
} rx_pkt_hdr_t;

typedef struct MCU_PACK {
	uint16_t pktcount;
	uint16_t next_pkt;
	uint16_t size;
	uint16_t flags;
	uint16_t ptr;
	uint16_t wr_ptr;
	uint16_t rx_start;
	uint16_t rx_end;
} rx_info_t;

#define FULL_DUPLEX

#define ENC28J60_RX_BUFFER_SIZE 6144  //holds up to three TCP/IP packets
#define ENC28J60_TX_BUFFER_SIZE 2048 //holds one TCP/IP packet

//RX Buffer
#define ENC28J60_RX_BUFFER 0x0000
#define ENC28J60_RX_BUFFER_END (ENC28J60_RX_BUFFER + ENC28J60_RX_BUFFER_SIZE - 1)
#define ENC28J60_RX_BUFFER_L ((int8_t)(ENC28J60_RX_BUFFER&0x00FF))
#define ENC28J60_RX_BUFFER_H ((int8_t)(ENC28J60_RX_BUFFER>>8))
#define ENC28J60_RX_BUFFER_END_L ((int8_t)(ENC28J60_RX_BUFFER_END&0x00FF))
#define ENC28J60_RX_BUFFER_END_H ((int8_t)(ENC28J60_RX_BUFFER_END>>8))

//TX Buffer
#define ENC28J60_TX_BUFFER (ENC28J60_RX_BUFFER_END+1)
#define ENC28J60_TX_BUFFER_END (ENC28J60_TX_BUFFER + ENC28J60_TX_BUFFER_SIZE - 1)
#define ENC28J60_TX_BUFFER_L ((int8_t)(ENC28J60_TX_BUFFER&0x00FF))
#define ENC28J60_TX_BUFFER_H ((int8_t)(ENC28J60_TX_BUFFER>>8))

#define CLEAR_MASK 0x1F
#define READ_CONTROL_MASK 0
#define READ_BUFFER 0x3A
#define WRITE_CONTROL_MASK (2<<5)
#define WRITE_BUFFER (0x7A)
#define BIT_MCU_SET_MASK (4<<5)
#define BIT_CLEAR_MASK (5<<5)
#define SYSTEM_RESET 0xFF

//Control Registers
//Name	Bank:Value
#define	EIE	0x1B
#define	EIR	0x1C
#define	ESTAT	0x1D
#define	ECON2	0x1E
#define	ECON1	0x1F

//ETH/MAC/MII Registers
#define	ERDPTL	0x0000
#define	ERDPTH	0x0001
#define	EWRPTL	0x0002
#define	EWRPTH	0x0003
#define	ETXSTL	0x0004
#define	ETXSTH	0x0005
#define	ETXNDL	0x0006
#define	ETXNDH	0x0007
#define	ERXSTL	0x0008
#define	ERXSTH	0x0009
#define	ERXNDL	0x000A
#define	ERXNDH	0x000B
#define	ERXRDPTL	0x000C
#define	ERXRDPTH	0x000D
#define	ERXWRPTL	0x000E
#define	ERXWRPTH	0x000F
#define	EDMASTL	0x0010
#define	EDMASTH	0x0011
#define	EDMANDL	0x0012
#define	EDMANDH	0x0013
#define	EDMADSTL	0x0014
#define	EDMADSTH	0x0015
#define	EDMACSL	0x0016
#define	EDMACSH	0x0017

#define	EHT0	0x0100
#define	EHT1	0x0101
#define	EHT2	0x0102
#define	EHT3	0x0103
#define	EHT4	0x0104
#define	EHT5	0x0105
#define	EHT6	0x0106
#define	EHT7	0x0107
#define	EPMM0	0x0108
#define	EPMM1	0x0109
#define	EPMM2	0x010A
#define	EPMM3	0x010B
#define	EPMM4	0x010C
#define	EPMM5	0x010D
#define	EPMM6	0x010E
#define	EPMM7	0x010F
#define	EPMCSL	0x0110
#define	EPMCSH	0x0111
#define	EPMOL	0x0114
#define	EPMOH	0x0115
#define	ERXFCON	0x0118
#define	EPKTCNT	0x0119


#define	MACON1	0x0200
#define	MACON3	0x0202
#define	MACON4	0x0203
#define	MABBIPG	0x0204
#define	MAIPGL	0x0206
#define	MAIPGH	0x0207
#define	MACLCON1	0x0208
#define	MACLCON2	0x0209
#define	MAMXFLL	0x020A
#define	MAMXFLH	0x020B
#define	MICMD	0x0212
#define	MIREGADR	0x0214
#define	MIWRL	0x0216
#define	MIWRH	0x0217
#define	MIRDL	0x0218
#define	MIRDH	0x0219

#define	MAADR5	0x0300
#define	MAADR6	0x0301
#define	MAADR3	0x0302
#define	MAADR4	0x0303
#define	MAADR1	0x0304
#define	MAADR2	0x0305
#define	EBSTSD	0x0306
#define	EBSTCON	0x0307
#define	EBSTCSL	0x0308
#define	EBSTCSH	0x0309
#define	MISTAT	0x0310
#define	EREVID	0x0312
#define	ECOCON	0x0315
#define	EFLOCON	0x0317
#define	EPAUSL	0x0318
#define	EPAUSH	0x0319

//Physical registers
#define PHCON1 0x00
#define PHSTAT1 0x01
#define PHID1 0x02
#define PHID2 0x03
#define PHCON2 0x10
#define PHSTAT2 0x11
#define PHIE 0x12
#define PHIR 0x13
#define PHLCON 0x14

#define enc28j60_rst(cfg) (spi_swap(cfg->periph.port, SYSTEM_RESET))

static int8_t set_reg_bit(const device_cfg_t * cfg, uint16_t reg, uint8_t mask);
static int8_t clear_reg_bit(const device_cfg_t * cfg, uint16_t reg, uint8_t mask);
static int8_t read_eth_reg(const device_cfg_t * cfg, uint16_t reg); //read an eth register in the desired bank

#define read_mii_reg(cfg, reg) read_mac_reg(cfg, reg) //read MII register in the desired bank
static int8_t read_mac_reg(const device_cfg_t * cfg, uint16_t reg); //read MAC register in the desired bank
#define write_mii_reg(cfg, reg, byte) write_eth_reg(cfg, reg, byte) //write MII register in desired bank
#define write_mac_reg(cfg, reg, byte) write_eth_reg(cfg, reg, byte) //write MAC register in desired bank
static int8_t write_eth_reg(const device_cfg_t * cfg, uint16_t reg, int8_t byte); //write ETH register in desired bank
//static int16_t read_phy_reg(const device_cfg_t * cfg, uint8_t reg);
static int8_t write_phy_reg(const device_cfg_t * cfg, uint8_t reg, int16_t word);
static int8_t set_bank(const device_cfg_t * cfg, uint8_t bank);


//This writes registers in the current bank
static void write_register(const device_cfg_t * cfg, int8_t reg, int8_t byte);
static int8_t read_register(const device_cfg_t * cfg, int8_t reg); //This reads eth registers in the current bank
static int8_t read_register_dummy(const device_cfg_t * cfg, int8_t reg);

static int (* const enc28j60_ioctl_func_table[I_ETH_TOTAL])(const device_cfg_t *, void*) = {
		enc28j60_getattr,
		enc28j60_setattr,
		enc28j60_setaction,
		enc28j60_init_tx_pkt,
		enc28j60_send_tx_pkt,
		enc28j60_tx_pkt_busy,
		enc28j60_rx_pkt_rdy,
		enc28j60_rx_pkt_complete
};

static void rx_init(const device_cfg_t * cfg);

static void assert_cs(const device_cfg_t * cfg){
	mcu_pio_clrmask(cfg->pcfg.spi.cs.port, (void*)(ssize_t)(1<<cfg->pcfg.spi.cs.pin));
}

static void deassert_cs(const device_cfg_t * cfg){
	mcu_pio_setmask(cfg->pcfg.spi.cs.port, (void*)(ssize_t)(1<<cfg->pcfg.spi.cs.pin));
}

int enc28j60_open(const device_cfg_t * cfg){
	int err;
	uint8_t status;
	pio_attr_t gattr;

	deassert_cs(cfg);

	err = mcu_check_spi_port(cfg);
	if ( err < 0 ){
		errno = EAGAIN;
		return -1;
	}

	gattr.mask = (1<<cfg->pcfg.spi.cs.pin);
	gattr.mode = PIO_MODE_OUTPUT | PIO_MODE_DIRONLY;
	mcu_pio_setattr(cfg->pcfg.spi.cs.port, &gattr);

	//first ping the device for a response
	write_eth_reg(cfg, MAADR5, 0xA5);
	status = read_eth_reg(cfg, MAADR5);
	if ( status != 0xA5 ){
		errno = EIO;
		return -1;
	}
	//initialize the device
	return 0;
}

int enc28j60_ioctl(const device_cfg_t * cfg, int request, void * ctl){
	int device_request;
	if ( _IOCTL_IDENT(request) == ETH_IOC_IDENT_CHAR ){
		device_request = _IOCTL_NUM(request);

		//call the request from the table
		if ( device_request < I_ETH_TOTAL){
			return enc28j60_ioctl_func_table[device_request](cfg, ctl);
		} else {
			errno = EINVAL;
			return -1;
		}

	} else {
		return mcu_spi_ioctl(cfg, request, ctl);
	}
	return 0;
}


static int complete_spi_operation(void * context, mcu_event_t data){
	const device_cfg_t * cfg = (device_cfg_t *)context;
	enc28j60_state_t * state = (enc28j60_state_t*)cfg->state;
	deassert_cs(cfg);
	state->callback( state->context, NULL);
	return 0;
}

int enc28j60_read(const device_cfg_t * cfg, device_transfer_t * rop){
	int err;
	enc28j60_state_t * state = (enc28j60_state_t*)cfg->state;

	//Initialize the callback to deassert the CS line
	state->callback = rop->callback;
	state->context = rop->context;
	rop->callback = complete_spi_operation;
	rop->context = (void*)cfg;

	//read nbytes from the buffer
	assert_cs(cfg);
	mcu_spi_swap(cfg->periph.port, (void*)READ_BUFFER);
	err = mcu_spi_read(cfg, rop);
	if ( err != 0 ){
		deassert_cs(cfg);
	}
	return err;
}


int enc28j60_write(const device_cfg_t * cfg, device_transfer_t * wop){
	int err;
	enc28j60_state_t * state = (enc28j60_state_t*)cfg->state;

	//Initialize the callback to deassert the CS line
	state->callback = wop->callback;
	state->context = wop->context;
	wop->callback = complete_spi_operation;
	wop->context = (void*)cfg;

	assert_cs(cfg);
	mcu_spi_swap(cfg->periph.port, (void*)(size_t)WRITE_BUFFER);
	err = mcu_spi_write(cfg, wop);
	if ( err != 0 ){
		deassert_cs(cfg);
	}
	return err;
}


int enc28j60_close(const device_cfg_t * cfg){
	//Disable packet reception
	clear_reg_bit(cfg, ECON1, 0x04); //disable RX

	set_reg_bit(cfg, ECON2, 0x08); //set VRPS to put regulator in low-current mode
	set_reg_bit(cfg, ECON2, 0x20); //Enable the power save bit
	return mcu_spi_close(cfg);
}


int enc28j60_init_tx_pkt(const device_cfg_t * cfg, void * ctl){
	union {
		uint8_t u8[2]; uint16_t u16;
	} addr;

	addr.u16 = ENC28J60_TX_BUFFER;
	set_reg_bit(cfg, ECON1, 0x80); //hold transmit logic in reset
	write_eth_reg(cfg, ETXSTL, addr.u8[0]);
	write_eth_reg(cfg, ETXSTH, addr.u8[1]);

	write_eth_reg(cfg, EWRPTL, addr.u8[0]);
	write_eth_reg(cfg, EWRPTH, addr.u8[1]);

	//get the packet ready by writing the first unique byte
	assert_cs(cfg);
	mcu_spi_swap(cfg->periph.port, (void*)(size_t)WRITE_BUFFER);
	mcu_spi_swap(cfg->periph.port, (void*)(size_t)((1<<1)|(1<<2)));
	deassert_cs(cfg);

	clear_reg_bit(cfg, ECON1, 0x80); //normal transmit logic operation
	return 0;
}


int enc28j60_send_tx_pkt(const device_cfg_t * cfg, void * ctl){
	union { uint8_t u8[2]; uint16_t u16; } addr;

	//Read the write pointer and mark the end of the packet
	addr.u8[0] = read_eth_reg(cfg, EWRPTL);
	addr.u8[1] = read_eth_reg(cfg, EWRPTH);
	addr.u16 -= 1;

	write_eth_reg(cfg, ETXNDL, addr.u8[0]);  //This points to the last byte IN the packet
	write_eth_reg(cfg, ETXNDH, addr.u8[1]);

	write_eth_reg(cfg, EIR, 0); //clear the interrupt flags
	set_reg_bit(cfg, ECON1, 0x08); //start the packet transfer

	return 0;

}

int enc28j60_tx_pkt_busy(const device_cfg_t * cfg, void * ctl){
	uint8_t tmp;
	enc28j60_txstatus_t status;
	union { uint8_t u8[2]; uint16_t u16; } addr;
	tmp = read_eth_reg(cfg, EIR);
	char * p;
	int i;

	if ( tmp & 0x02 ){
		//transmit error occurred
		errno = EIO;
		return -1;
	}

	if ( tmp & 0x08 ){
		//transmit is complete
		if( ctl != NULL ){
			//read the tx status

			addr.u8[0] = read_eth_reg(cfg, ETXNDL);
			addr.u8[1] = read_eth_reg(cfg, ETXNDH);
			addr.u16++;

			//Write the read pointer to the address of the next packet
			write_eth_reg(cfg, ERDPTL, addr.u8[0]);
			write_eth_reg(cfg, ERDPTH, addr.u8[1]);

			assert_cs(cfg);
			mcu_spi_swap(cfg->periph.port, (void*)(size_t)READ_BUFFER);

			p = (char*)&status;

			for(i=0; i < sizeof(status); i++){
				*p++ = mcu_spi_swap(cfg->periph.port, 0x00);
			}
			deassert_cs(cfg);

			memcpy(ctl, &status, sizeof(status));

		}
		return 0;
	}

	return 1;
}

int enc28j60_rx_pkt_rdy(const device_cfg_t * cfg, void * ctl){
	int i;
	uint8_t count;
	rx_pkt_hdr_t hdr;
	rx_info_t info;
	char * p;
	union { uint16_t u16; uint8_t u8[2]; } addr;
	enc28j60_state_t * state = cfg->state;


	memset(&info, 0, sizeof(info));
	//See if a packet is ready to be read
	count = read_eth_reg(cfg, EPKTCNT);

	info.ptr = read_eth_reg(cfg, ERXRDPTL);
	info.ptr |= (read_eth_reg(cfg, ERXRDPTL)<<8);

	info.wr_ptr = read_eth_reg(cfg, ERXWRPTL);
	info.wr_ptr |= (read_eth_reg(cfg, ERXWRPTH) << 8);

	info.rx_start = read_eth_reg(cfg, ERXSTL);
	info.rx_start |= (read_eth_reg(cfg, ERXSTH)<<8);

	info.rx_end = read_eth_reg(cfg, ERXNDL);
	info.rx_end |= (read_eth_reg(cfg, ERXNDH)<<8);

	info.pktcount = count;

	hdr.size = 0;
	if ( count > 0 ){
		addr.u16 = state->next_pkt;
		//Write the read pointer to the address of the next packet
		write_eth_reg(cfg, ERDPTL, addr.u8[0]);
		write_eth_reg(cfg, ERDPTH, addr.u8[1]);

		//Read the packet's status bytes -- and return the recv'd byte count
		assert_cs(cfg);
		mcu_spi_swap(cfg->periph.port, (void*)(size_t)READ_BUFFER);

		p = (char*)&hdr;

		for(i=0; i < sizeof(rx_pkt_hdr_t); i++){
			*p++ = mcu_spi_swap(cfg->periph.port, 0x00);
		}
		deassert_cs(cfg);

		info.flags = hdr.flags;
		info.next_pkt = hdr.next_pkt;
		info.size = hdr.size;

		state->next_pkt = hdr.next_pkt;
	}

	if( ctl != NULL ){
		memcpy(ctl, &info, sizeof(info));
	}

	if( count == 255 ){
		//This is an error, the RX circuitry needs to be reset
		errno = EPROTO;
		rx_init(cfg);
		return -1;
	}

	return hdr.size;
}

int enc28j60_rx_pkt_complete(const device_cfg_t * cfg, void * ctl){
	union { uint16_t u16; uint8_t u8[2]; } addr;
	enc28j60_state_t * state = cfg->state;

	addr.u16 = state->next_pkt;

	//decrement the packet counter
	set_reg_bit(cfg, ECON2, 0x40); //decrement the packet count

	//free space in the buffer
	write_eth_reg(cfg, ERXRDPTL, addr.u8[0]);
	write_eth_reg(cfg, ERXRDPTH, addr.u8[1]);

	return 0;
}

int enc28j60_getattr(const device_cfg_t * cfg, void * ctl){
	int tmp;
	eth_attr_t * attr = (eth_attr_t *)ctl;

	attr->mac_addr[0] = read_mac_reg(cfg, MAADR1);
	attr->mac_addr[1] = read_mac_reg(cfg, MAADR2);
	attr->mac_addr[2] = read_mac_reg(cfg, MAADR3);
	attr->mac_addr[3] = read_mac_reg(cfg, MAADR4);
	attr->mac_addr[4] = read_mac_reg(cfg, MAADR5);
	attr->mac_addr[5] = read_mac_reg(cfg, MAADR6);

	tmp = read_mac_reg(cfg, MACON3);
	if ( tmp & 0x01 ){
		attr->mode |= ETH_MODE_FULLDUPLEX;
	} else {
		attr->mode |= ETH_MODE_HALFDUPLEX;
	}

	return 0;
}

void rx_init(const device_cfg_t * cfg){
	//initialize RX Buffer
	enc28j60_state_t * state = cfg->state;
	set_reg_bit(cfg, ECON1, 0xC0); //hold TX/RX in reset


	write_eth_reg(cfg, ERXSTL, ENC28J60_RX_BUFFER_L);
	write_eth_reg(cfg, ERXSTH, ENC28J60_RX_BUFFER_H);
	write_eth_reg(cfg, ERXNDL, ENC28J60_RX_BUFFER_END_L);
	write_eth_reg(cfg, ERXNDH, ENC28J60_RX_BUFFER_END_H);


	//Point RX pointer to beginning of buffer
	write_eth_reg(cfg, ERXRDPTL, ENC28J60_RX_BUFFER_L);
	write_eth_reg(cfg, ERXRDPTH, ENC28J60_RX_BUFFER_H);

	write_eth_reg(cfg, ERDPTL, ENC28J60_RX_BUFFER_L);
	write_eth_reg(cfg, ERDPTH, ENC28J60_RX_BUFFER_H);

	state->next_pkt = ENC28J60_RX_BUFFER;

	write_eth_reg(cfg, EPKTCNT, 0);

	clear_reg_bit(cfg, ECON1, 0xC0); //hold TX/RX in reset

}

int enc28j60_setattr(const device_cfg_t * cfg, void * ctl){
	int8_t temp;
	enc28j60_attr_t * attr = (enc28j60_attr_t *)ctl;

	set_reg_bit(cfg, ECON1, 0xC0); //hold TX/RX in reset
	set_reg_bit(cfg, ECON2, 0x80); //set the Auto Increment bit

	write_eth_reg(cfg, EIR, 0x00); //clear interrupts


	//wait for Power-on reset to complete
	do {
		temp = read_eth_reg(cfg, ESTAT);
	} while ( !(temp & 0x01) ); //wait for the OSC clock to stabilize

	/*
	do{
		set_reg_bit(cfg, ECON2, 0x40);
		temp = read_eth_reg(cfg, EPKTCNT);
	} while(temp); //get rid of all pending packets
	*/

	rx_init(cfg);

	//Setup the Receive filters
	write_eth_reg(cfg, ERXFCON, 0x20);


	//MAC Initialization
	write_mac_reg(cfg, MACON1, 0x0D);
	if ( attr->full_duplex == true ){
		write_mac_reg(cfg, MACON3, 0x37);
	} else {
		write_mac_reg(cfg, MACON3, 0x36);
	}

	write_mac_reg(cfg, MACON4, (1<<6)); //set the DEFER bit for IEEE conformance
	write_mac_reg(cfg, MABBIPG, 0x15);
	write_mac_reg(cfg, MACON4, 0x40);
	write_mac_reg(cfg, MAMXFLL, 0xEE);  //Frame length is 1518
	write_mac_reg(cfg, MAMXFLH, 0x05);
	write_mac_reg(cfg, MABBIPG, 0x15);
	write_mac_reg(cfg, MAIPGL, 0x12);

	//write the MAC Address MAADR1:6
	write_mac_reg(cfg, MAADR1, attr->mac_addr[0]);
	write_mac_reg(cfg, MAADR2, attr->mac_addr[1]);
	write_mac_reg(cfg, MAADR3, attr->mac_addr[2]);
	write_mac_reg(cfg, MAADR4, attr->mac_addr[3]);
	write_mac_reg(cfg, MAADR5, attr->mac_addr[4]);
	write_mac_reg(cfg, MAADR6, attr->mac_addr[5]);

	if ( attr->full_duplex == true ){
		//write Physical registers
		write_phy_reg(cfg, PHCON1, 0x0100); //enable full duplex--just like MACON3.0
	} else {
		write_phy_reg(cfg, PHCON1, 0x0000); //disable full duplex--just like MACON3.0
	}

	//write_phy_reg(cfg, PHLCON, 0x0419);  //LED configuration

	write_eth_reg(cfg, EPKTCNT, 0x00);
	//enable packet reception
	set_reg_bit(cfg, ECON1, 0x04); //this is not setting???

	return 0;
}

int enc28j60_setaction(const device_cfg_t * cfg, void * ctl){
	errno = ENOTSUP;
	return -1;
}

/*
int16_t read_phy_reg(const device_cfg_t * cfg, uint8_t reg){
	union { int16_t s; int8_t c[2]; } value;
	do {
		value.c[0] = read_mii_reg(cfg, MISTAT);
	} while (value.c[0] & 0x01);
	//load the address
	write_mii_reg(cfg, MIREGADR, reg);
	//set read flag
	write_mii_reg(cfg, MICMD, 0x01);
	//wait for busy flag to clear
	_delay_us(10);
	do {
		value.c[0] = read_mii_reg(cfg, MISTAT);
	} while (value.c[0] & 0x01);
	write_mii_reg(cfg, MICMD, 0x00); //clear the read bit
	//read registers
	value.c[0] = read_mii_reg(cfg, MIRDL);
	value.c[1] = read_mii_reg(cfg, MIRDH);
	return value.s;
}
 */

int8_t write_phy_reg(const device_cfg_t * cfg, uint8_t reg, int16_t word){
	union { int16_t s; int8_t c[2]; } value;
	//wait for busy flag to clear
	do {
		value.c[0]= read_mii_reg(cfg, MISTAT);
	} while (value.c[0] & 0x01);
	//load the address
	write_mii_reg(cfg, MIREGADR, reg);
	//load the value
	value.s = word;
	write_mii_reg(cfg, MIWRL, value.c[0]);
	write_mii_reg(cfg, MIWRH, value.c[1]);
	return 0;
}

int8_t read_eth_reg(const device_cfg_t * cfg, uint16_t reg){
	union { int16_t s; int8_t c[2]; } value;
	value.s = (int16_t)reg;
	set_bank(cfg, value.c[1]);
	return read_register(cfg, value.c[0]);
}

int8_t read_mac_reg(const device_cfg_t * cfg, uint16_t reg){
	union { int16_t s; int8_t c[2]; } value;
	value.s = (int16_t)reg;
	set_bank(cfg, value.c[1]);
	return read_register_dummy(cfg, value.c[0]);
}

int8_t write_eth_reg(const device_cfg_t * cfg, uint16_t reg, int8_t byte){
	union { int16_t s; int8_t c[2]; } value;
	value.s = (int16_t)reg;
	set_bank(cfg, value.c[1]);
	value.c[0] &= CLEAR_MASK;
	write_register(cfg, value.c[0], byte);
	return 0;
}

int8_t set_reg_bit(const device_cfg_t * cfg, uint16_t reg, uint8_t mask){
	union { int16_t s; int8_t c[2]; } value;
	value.s = (int16_t)reg;
	set_bank(cfg, value.c[1]);
	value.c[0] &= CLEAR_MASK;
	assert_cs(cfg);
	mcu_spi_swap(cfg->periph.port, (void*)(size_t)(value.c[0]|BIT_MCU_SET_MASK));
	mcu_spi_swap(cfg->periph.port, (void*)(size_t)mask);
	deassert_cs(cfg);
	return 0;
}

int8_t clear_reg_bit(const device_cfg_t * cfg, uint16_t reg, uint8_t mask){
	union { int16_t s; int8_t c[2]; } value;
	value.s = (int16_t)reg;
	set_bank(cfg, value.c[1]);
	value.c[0] &= CLEAR_MASK;
	assert_cs(cfg);
	mcu_spi_swap(cfg->periph.port, (void*)(size_t)(value.c[0]|BIT_CLEAR_MASK));
	mcu_spi_swap(cfg->periph.port, (void*)(size_t)mask);
	deassert_cs(cfg);
	return 0;
}


int8_t set_bank(const device_cfg_t * cfg, uint8_t bank) {
	uint8_t temp;
	//first clear the lowest two bits
	temp = bank & 0x03;
	assert_cs(cfg);
	mcu_spi_swap(cfg->periph.port, (void*)(size_t)(ECON1|BIT_CLEAR_MASK));
	mcu_spi_swap(cfg->periph.port, (void*)(size_t)0x03);
	deassert_cs(cfg);
	assert_cs(cfg);
	mcu_spi_swap(cfg->periph.port, (void*)(size_t)(ECON1|BIT_MCU_SET_MASK));
	mcu_spi_swap(cfg->periph.port, (void*)(size_t)temp);
	deassert_cs(cfg);
	return 0;
}



void write_register(const device_cfg_t * cfg, int8_t reg, int8_t byte){
	uint8_t temp;
	assert_cs(cfg);
	temp = reg & CLEAR_MASK;
	temp |= WRITE_CONTROL_MASK;
	mcu_spi_swap(cfg->periph.port, (void*)(size_t)temp);
	mcu_spi_swap(cfg->periph.port, (void*)(size_t)byte);
	deassert_cs(cfg);
}

int8_t read_register(const device_cfg_t * cfg, int8_t reg){
	/*
	uint8_t temp[2] = {reg & CLEAR_MASK, 0xFF};
	transferMasterSPIData(temp, temp, 2);
	return temp[1];
	 */

	uint8_t temp;
	assert_cs(cfg);
	temp = reg & CLEAR_MASK;
	//temp |= READ_CONTROL_MASK;
	mcu_spi_swap(cfg->periph.port, (void*)(size_t)temp);
	temp = mcu_spi_swap(cfg->periph.port, (void*)(size_t)0xFF);
	deassert_cs(cfg);
	return temp;

}

int8_t read_register_dummy(const device_cfg_t * cfg, int8_t reg){
	uint8_t temp;
	assert_cs(cfg);
	temp = reg & CLEAR_MASK;
	temp |= READ_CONTROL_MASK;
	mcu_spi_swap(cfg->periph.port, (void*)(size_t)temp);
	mcu_spi_swap(cfg->periph.port, (void*)(size_t)0xFF); //this is the dummy byte
	temp = mcu_spi_swap(cfg->periph.port, (void*)(size_t)0xFF);
	deassert_cs(cfg);
	return temp;
}

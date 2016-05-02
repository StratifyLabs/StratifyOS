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
#include <unistd.h>
#include "mcu/pio.h"
#include "mcu/spi.h"
#include "mcu/core.h"
#include "mcu/debug.h"
#include "mcu/wdt.h"
#include "iface/device_config.h"

#include "device/sys.h"
#include "mcu/core.h"
#include "iface/dev/disk.h"
#include "sdspi_local.h"
#include "mcu/task.h"

#define BLOCK_SIZE 512
#define CMD_FRAME_SIZE 16

#define LONG_DELAY 250
#define SHORT_DELAY 100


static int _sdspi_erase_blocks(const device_cfg_t * cfg, uint32_t block_num, uint32_t end_block);
static int _sdspi_busy(const device_cfg_t * cfg);
static int _sdspi_status(const device_cfg_t * cfg, uint8_t * buf);
static int _sdspi_csd(const device_cfg_t * cfg, uint8_t * buf);

static sdspi_r1_t _sdspi_cmd_r1(const device_cfg_t * cfg, uint8_t cmd, uint32_t arg, uint8_t * r);
//static sdspi_r2_t _sdspi_cmd_r2(const device_cfg_t * cfg, uint8_t cmd, uint32_t arg, uint8_t * r);
static sdspi_r3_t _sdspi_cmd_r3(const device_cfg_t * cfg, uint8_t cmd, uint32_t arg);


static int _sdspi_send_cmd(const device_cfg_t * cfg, uint8_t cmd, uint32_t arg, uint8_t * response);
static int _sdspi_parse_response(uint8_t * response, int num, sdspi_r_t * r, uint32_t * arg);
static int _sdspi_parse_data(uint8_t * dest, int nbyte, int count, uint8_t token, uint8_t * response);
static int _sdspi_read_data(const device_cfg_t * cfg, void * data, int nbyte, uint8_t token, uint8_t * first_response);
//static int _sdspi_write_data(const device_cfg_t * cfg, const void * data, int nbyte);

static int _sdspi_transfer(const device_cfg_t * cfg, const uint8_t * data_out, uint8_t * data_in, int nbyte);

static int _sdspi_try_read(const device_cfg_t * cfg, int first);

static void _sdspi_deassert_cs(const device_cfg_t * cfg){
	mcu_pio_setmask(cfg->pcfg.spi.cs.port, (void*)(ssize_t)(1<<cfg->pcfg.spi.cs.pin));
}

static void _sdspi_assert_cs(const device_cfg_t * cfg){
	mcu_pio_clrmask(cfg->pcfg.spi.cs.port, (void*)(ssize_t)(1<<cfg->pcfg.spi.cs.pin));
}

static void _sdspi_state_callback(const device_cfg_t * cfg, int err, int nbyte);


int sdspi_open(const device_cfg_t * cfg){
	int err;
	pio_attr_t attr;
	//sdspi_state_t * state = (sdspi_state_t*)cfg->state;

	err = mcu_check_spi_port(cfg);
	if ( err < 0 ){
		return err;
	}

	_sdspi_deassert_cs(cfg);
	attr.mask = (1<<cfg->pcfg.spi.cs.pin);
	attr.mode = PIO_MODE_OUTPUT | PIO_MODE_DIRONLY;
	mcu_pio_setattr(cfg->pcfg.spi.cs.port, &attr);

	//state->prot = 0;

	//The device is ready to use
	return 0;
}


void _sdspi_state_callback(const device_cfg_t * cfg, int err, int nbyte){
	sdspi_state_t * state = ((device_cfg_t *)cfg)->state;
	if( state->callback ){
		*(state->nbyte) = nbyte;
		if( nbyte < 0 ){
			struct _reent * reent = task_table[state->op.tid].reent;
			reent->_errno = err;
		}
		state->callback(state->context, 0);
		state->callback = 0;
	}
}

static int continue_spi_read(void * cfg, mcu_event_t ignore){
	//data has been read -- complete the operation
	sdspi_state_t * state = ((device_cfg_t *)cfg)->state;
	uint16_t checksum;

	if( state->count < 0 ){

		state->timeout++;
		if( state->timeout > 5000 ){
			//failed to read the data
			_sdspi_deassert_cs(cfg);

			_sdspi_state_callback(cfg, EIO, -2);
			return 0;
		}

		//try again to try the start of the data
		return _sdspi_try_read(cfg, 0);

	} else {
		//the read is complete

		_sdspi_transfer(cfg, 0, state->cmd, CMD_FRAME_SIZE); //gobble up the CRC
		checksum = (state->cmd[0] << 8) + state->cmd[1];
		if( checksum != crc16(0x0000, (const uint8_t *)state->buf, (size_t)*(state->nbyte)) ){
			*(state->nbyte) = -1;
		}

		//execute the callback
		_sdspi_state_callback(cfg, 0, *(state->nbyte));
	}

	return 0;
}

int _sdspi_try_read(const device_cfg_t * cfg, int first){
	sdspi_state_t * state = cfg->state;
	state->count = _sdspi_parse_data((uint8_t*)state->buf, *(state->nbyte), -1, SDSPI_START_BLOCK_TOKEN, state->cmd);
	if( state->count >= 0 ){
		state->op.nbyte = *(state->nbyte) - state->count;
		state->op.buf = (void*)&(state->buf[state->count]);
	} else {
		state->op.nbyte = CMD_FRAME_SIZE;
		state->op.buf = state->cmd;
	}
	state->op.context = (void*)cfg;
	state->op.callback = continue_spi_read;


	if( first != 0 ){
		//send the command for the first time
		return mcu_spi_read(cfg, &(state->op));
	}

	if( mcu_spi_read(cfg, &(state->op)) != 0 ){
		_sdspi_state_callback(cfg, EINVAL, -5);
		return 0;
	}
	return 1;
}

int sdspi_read(const device_cfg_t * cfg, device_transfer_t * rop){
	//first write the header command
	sdspi_state_t * state = cfg->state;
	sdspi_r1_t r1;

	if( rop->nbyte != BLOCK_SIZE ){
		errno = EINVAL;
		return -1;
	}

	if( _sdspi_busy(cfg) != 0 ){
		errno = EBUSY;
		return -1;
	}

	state->context = rop->context;
	state->callback = rop->callback;
	state->nbyte = &(rop->nbyte);
	state->buf = rop->buf;
	state->timeout = 0;
	state->op.tid = rop->tid;


	r1 = _sdspi_cmd_r1(cfg, SDSPI_CMD17_READ_SINGLE_BLOCK, rop->loc, state->cmd);
	if( r1.u8 != 0x00 ){
		if( (r1.param_error) ){
			errno = EINVAL;
			return -6;
		}

		if( (r1.addr_error) ){
			errno = EINVAL;
			return -5;
		}

		if( (r1.erase_sequence_error) ){
			errno = EINVAL;
			return -4;
		}

		if( (r1.crc_error) ){
			errno = EINVAL;
			return -3;
		}

		if( (r1.illegal_command) ){
			errno = EINVAL;
			return -2;
		}

		errno = EIO;
		return -6;
	}

	_sdspi_assert_cs(cfg);
	_mcu_core_delay_us(LONG_DELAY);

	return _sdspi_try_read(cfg, 1);
}

static int continue_spi_write(void * cfg, mcu_event_t ignore){
	sdspi_state_t * state = ((const device_cfg_t *)cfg)->state;

	uint16_t checksum;

	//calculate and write the checksum
	checksum = crc16(0x0000, (const uint8_t*)state->buf, *(state->nbyte));

	//finish the write
	state->cmd[0] = checksum >> 8;
	state->cmd[1] = checksum;
	state->cmd[2] = 0xFF;
	state->cmd[3] = 0xFF;
	state->cmd[4] = 0xFF;
	_sdspi_transfer(cfg, state->cmd, state->cmd, 5); //send dummy CRC
	_sdspi_deassert_cs(cfg);

	if( (state->cmd[2] & 0x1F) == 0x05 ){
		//data was accepted
		_sdspi_state_callback(cfg, 0, *(state->nbyte));
	} else {
		//data was not accepted
		_sdspi_state_callback(cfg, EIO, -1);
	}

	return 0;
}


int sdspi_write(const device_cfg_t * cfg, device_transfer_t * wop){

	sdspi_state_t * state = cfg->state;
	sdspi_r1_t r1;


	if( wop->nbyte != BLOCK_SIZE ){
		errno = EINVAL;
		return -1;
	}

	//check to see if device is busy
	if( _sdspi_busy(cfg) != 0 ){
		errno = EBUSY;
		return -1;
	}

	state->context = wop->context;
	state->callback = wop->callback;
	state->nbyte = &(wop->nbyte);
	state->buf = wop->buf;
	state->timeout = 0;

	r1 = _sdspi_cmd_r1(cfg, SDSPI_CMD24_WRITE_SINGLE_BLOCK, wop->loc, state->cmd);
	if( r1.u8 != 0x00 ){
		if( (r1.addr_error) || (r1.param_error) ){
			errno = EINVAL;
			return -1;
		}

		errno = EIO;
		return -1;
	}


	state->cmd[0] = 0xFF;  //busy byte
	state->cmd[1] = SDSPI_START_BLOCK_TOKEN;

	_sdspi_assert_cs(cfg);
	_mcu_core_delay_us(LONG_DELAY);
	_sdspi_transfer(cfg, state->cmd, 0, 2);

	state->op.nbyte = wop->nbyte;
	state->op.buf = wop->buf;
	state->op.context = (void*)cfg;
	state->op.callback = continue_spi_write;
	state->op.tid = wop->tid;

	return mcu_spi_write(cfg, &(state->op));
}

int sdspi_ioctl(const device_cfg_t * cfg, int request, void * ctl){
	//sdspi_cfg_t * sst_cfg = (sdspi_cfg_t*)cfg->dcfg;
	sdspi_state_t * state = (sdspi_state_t*)cfg->state;
	disk_attr_t * attr = ctl;
	disk_erase_block_t * deb = ctl;
	disk_erase_block_t debs;
	sdspi_r_t resp;
	spi_attr_t spi_attr;
	int timeout;

	uint16_t erase_size;
	uint16_t erase_timeout;

	uint8_t buffer[64];

	switch(request){
	case I_DISK_ERASE_BLOCK:
	case I_DISK_ERASE_DEVICE:
		if( state->prot == 1 ){
			errno = EROFS;
			return -1;
		}
		break;
	}

	switch(request){
	case I_DISK_PROTECT:
	case I_DISK_UNPROTECT:
	case I_DISK_ERASE_DEVICE:
	case I_DISK_POWER_DOWN:
	case I_DISK_POWER_UP:
	case I_DISK_GET_BLOCKSIZE:
	case I_DISK_GET_DEVICE_ERASETIME:
	case I_DISK_GET_BLOCK_ERASETIME:
	case I_DISK_GET_SIZE:
		errno = ENOTSUP;
		return -1;
		break;


	case I_SDSPI_STATUS:


		break;

	case I_DISK_INIT:
		//set SPI to 100kbits
		spi_attr.bitrate = 100000;
		spi_attr.format = SPI_ATTR_FORMAT_SPI;
		spi_attr.mode = SPI_ATTR_MODE0;
		spi_attr.width = 8;
		spi_attr.pin_assign = cfg->pin_assign;
		spi_attr.master = SPI_ATTR_MASTER;
		if( mcu_spi_setattr(cfg->periph.port, &spi_attr) < 0 ){
			mcu_priv_debug("Attr\n");
			return -2;
		}


		_mcu_core_delay_us(500);

		//init sequence
		//apply at least 74 init clocks with DI and CS high
		_sdspi_deassert_cs(cfg);
		_mcu_core_delay_us(LONG_DELAY);
		_sdspi_transfer(cfg, 0, 0, CMD_FRAME_SIZE*6);
		_mcu_core_delay_us(LONG_DELAY);


		_mcu_core_delay_us(500);

		resp.r1 = _sdspi_cmd_r1(cfg, SDSPI_CMD0_GO_IDLE_STATE, 0, 0);
		if( resp.r1.start == 1 ){
			errno = EIO;
			mcu_priv_debug("GO IDLE\n");
			return -4;
		}

		if( resp.r1.idle != 1 ){
			errno = EIO;
			mcu_priv_debug("No IDLE\n");
			return -5;
		}

		//send CMD8
		resp.r3 = _sdspi_cmd_r3(cfg, SDSPI_CMD8_SEND_IF_COND, 0x01AA);
		if( resp.r3.r1.u8 != 0x01 ){
			errno = EIO;
			mcu_priv_debug("NO IF COND\n");
			return -6;
		}

		//enable checksums
		resp.r1 = _sdspi_cmd_r1(cfg, SDSPI_CMD59_CRC_ON_OFF, 0xFFFFFFFF, 0);
		if( resp.r1.u8 != 0x01 ){
			errno = EIO;
			mcu_priv_debug("NO 59\n");
			return -7;
		}

		timeout = 0;
		do {
			resp.r1 = _sdspi_cmd_r1(cfg, 55, 0, 0);  //send 55
			if( resp.r1.u8 != 0x01 ){
				errno = EIO;
				mcu_priv_debug("NO 55\n");
				return -7;
			}

			resp.r1 = _sdspi_cmd_r1(cfg, 41, 1<<30, 0);  //indicate that HC is supported
			if( (resp.r1.u8 != 0x01) && (resp.r1.u8 != 0x00) ){  //this takes awhile to return to zero
				errno = EIO;
				mcu_priv_debug("HC?\n");
				return -8;
			}
			timeout++;
			//reset wdt
			mcu_wdt_priv_reset(0);
		} while( (resp.r1.u8 != 0x00) && (timeout < 500) );

		if( timeout == 500 ){
			errno = EIO;
			mcu_priv_debug("TIMEOUT\n");
			return -9;
		}

		spi_attr.bitrate = cfg->bitrate;
		if( mcu_spi_setattr(cfg->periph.port, &spi_attr) < 0 ){
			mcu_priv_debug("BITRATE\n");
			return -10;
		}

		_sdspi_assert_cs(cfg);
		_mcu_core_delay_us(LONG_DELAY);
		_sdspi_transfer(cfg, 0, 0, CMD_FRAME_SIZE);
		_sdspi_deassert_cs(cfg);

		return 0;

	case I_DISK_ERASE_BLOCK:
		debs.start = ((ssize_t)ctl) / BLOCK_SIZE;
		debs.end = debs.start;
		deb = &debs;
		/* no break */
	case I_DISK_ERASEBLOCKS:
		//erase blocks in a sequence
		if( _sdspi_busy(cfg) != 0 ){
			errno = EBUSY;
			return -1;
		}
		return _sdspi_erase_blocks(cfg, deb->start, deb->end);

	case I_DISK_BUSY:
		return _sdspi_busy(cfg);

	case I_DISK_GETATTR:

		if( _sdspi_busy(cfg) != 0 ){
			errno = EBUSY;
			return -1;
		}

		_sdspi_assert_cs(cfg);
		_mcu_core_delay_us(LONG_DELAY);
		_sdspi_transfer(cfg, 0, 0, CMD_FRAME_SIZE);
		_sdspi_deassert_cs(cfg);

		//Write block size and address are fixed to BLOCK_SIZE (512)
		attr->address_size = BLOCK_SIZE;
		attr->write_block_size = attr->address_size;

		//This is from CSD C_Size and TRANS_SPEED
		if( _sdspi_csd(cfg, buffer) < 0 ){
			errno = EIO;
			return -1;
		}
		attr->num_write_blocks = (((buffer[7] & 63) << 16) + (buffer[8] << 8) + buffer[9] ) * 1024; //csize * 1024 (csize is size / 512K)
		attr->bitrate = 25*1000000;  //TRAN_SPEED should always be 25MHz

		//need to read Status to get AU_Size, ERASE_SIZE, ERASE_TIMEOUT
		if( _sdspi_status(cfg, buffer) < 0 ){
			errno = EIO;
			return -2;
		}
		attr->erase_block_size = (16*1024) << ((buffer[10]-1) >> 4);
		//ERASE_TIMEOUT divided by ERASE_SIZE
		erase_size = ((buffer[11] << 8) + buffer[12]);
		if( erase_size == 0 ){
			attr->erase_block_time = -1;
			attr->erase_device_time = -1;
		} else {
			erase_timeout = ((buffer[13] << 8) + buffer[14]);
			attr->erase_block_time = erase_timeout / erase_size;
			attr->erase_device_time =
					attr->erase_block_time * attr->num_write_blocks * attr->write_block_size / attr->erase_block_size;

		}
		return 0;

	default:
		return mcu_spi_ioctl(cfg, request, ctl);
	}
	return 0;
}

int sdspi_close(const device_cfg_t * cfg){
	return 0;
}


int _sdspi_erase_blocks(const device_cfg_t * cfg, uint32_t block_num, uint32_t end_block){
	sdspi_r_t r;

	//cmd32, 33, then 38
	r.r1 = _sdspi_cmd_r1(cfg, SDSPI_CMD32_ERASE_WR_BLK_START, block_num, 0);
	if( r.r1.u8 != 0 ){
		return -1;
	}
	r.r1 = _sdspi_cmd_r1(cfg, SDSPI_CMD33_ERASE_WR_BLK_END, end_block, 0);
	if( r.r1.u8 != 0 ){
		return -1;
	}
	r.r1 = _sdspi_cmd_r1(cfg, SDSPI_CMD38_ERASE, 0, 0);
	if( r.r1.u8 != 0 ){
		return -1;
	}

	return 0;
}

int _sdspi_busy(const device_cfg_t * cfg){
	uint8_t c;
	_sdspi_assert_cs(cfg);
	_mcu_core_delay_us(LONG_DELAY);
	c = mcu_spi_swap(cfg->periph.port, (void*)0xFF);
	_sdspi_deassert_cs(cfg);
	return (c == 0x00);
}

int _sdspi_status(const device_cfg_t * cfg, uint8_t * buf){
	sdspi_r_t resp;
	int ret;
	uint8_t tmp[CMD_FRAME_SIZE];

	resp.r1 = _sdspi_cmd_r1(cfg, SDSPI_CMD55_APP_CMD, 0, tmp);
	if( resp.r1.u8 != 0x00 ){
		errno = EIO;
		return -1;
	}

	resp.r1 = _sdspi_cmd_r1(cfg, SDSPI_CMD13_SD_STATUS, 0, tmp);
	if( resp.r1.u8 != 0x00 ){
		errno = EIO;
		return -1;
	}

	//now read the data
	ret = _sdspi_read_data(cfg, buf, sizeof(_sdspi_status_t), SDSPI_START_BLOCK_TOKEN, tmp);
	if( ret < 0 ){
		errno = EIO;
	}

	return ret;
}

int _sdspi_csd(const device_cfg_t * cfg, uint8_t * buf){
	sdspi_r_t resp;
	int ret;
	uint8_t tmp[CMD_FRAME_SIZE];

	resp.r1 = _sdspi_cmd_r1(cfg, SDSPI_CMD9_SEND_CSD, 0, tmp);
	if( resp.r1.u8 != 0x00 ){
		errno = EIO;
		return -1;
	}

	//now read the data
	ret = _sdspi_read_data(cfg, buf, sizeof(sdspi_csd_t), SDSPI_START_BLOCK_TOKEN, tmp);
	if( ret < 0 ){
		errno = EIO;
	}

	return ret;
}

int _sdspi_send_cmd(const device_cfg_t * cfg, uint8_t cmd, uint32_t arg, uint8_t * response){
	uint8_t buffer[CMD_FRAME_SIZE];
	int i;
	int ret;
	int retries;
	sdspi_r_t resp;
	memset(buffer, 0xFF, CMD_FRAME_SIZE);
	buffer[0] = 0x40 | cmd;
	buffer[1] = arg >> 24;
	buffer[2] = arg >> 16;
	buffer[3] = arg >> 8;
	buffer[4] = arg;
	buffer[5] = crc7(0, buffer, 5);

	retries = 0;
	do {
		//read the response
		_sdspi_assert_cs(cfg);
		_mcu_core_delay_us(LONG_DELAY);
		_sdspi_transfer(cfg, buffer, response, CMD_FRAME_SIZE);
		_mcu_core_delay_us(LONG_DELAY);
		_sdspi_deassert_cs(cfg);

		ret = 0;
		if( response != 0 ){
			for(i=0; i < CMD_FRAME_SIZE; i++){
				resp.u8[0] = response[i];
				if( resp.r1.start == 0 ){
					ret = response[i];
					break;
				}
			}
		}
		retries++;
	} while( (resp.r1.crc_error == 1) && (retries < 10) );

	return ret;
}

int _sdspi_parse_data(uint8_t * dest, int nbyte, int count, uint8_t token, uint8_t * response){
	int i;
	for(i=0; i < CMD_FRAME_SIZE; i++){
		if( count >= 0 ){
			dest[count++] = response[i];
		}

		if( (response[i] == token) && (count < 0) ){
			//got the token -- start copying data to *data
			count = 0;
		}

		if( count == nbyte ){
			break;
		}
	}

	return count;
}

int _sdspi_read_data(const device_cfg_t * cfg, void * data, int nbyte, uint8_t token, uint8_t * first_response){
	//look through the first response for the data token
	int timeout;
	uint8_t response[CMD_FRAME_SIZE];
	int count;

	count = _sdspi_parse_data((uint8_t*)data, nbyte, -1, token, first_response);

	timeout = 0;
	while( count < nbyte ){
		_sdspi_assert_cs(cfg);
		_mcu_core_delay_us(LONG_DELAY);
		if( count >= 0 ){
			_sdspi_transfer(cfg, 0, (uint8_t*)data + count, nbyte - count);
			count = nbyte;
		} else {
			_sdspi_transfer(cfg, 0, response, CMD_FRAME_SIZE);
			count = _sdspi_parse_data((uint8_t*)data, nbyte, count, token, response);
		}
		_sdspi_deassert_cs(cfg);


		timeout++;
		if( timeout > 100 ){
			return -1;
		}
	}

	_sdspi_assert_cs(cfg);
	_mcu_core_delay_us(LONG_DELAY);
	_sdspi_transfer(cfg, 0, response, CMD_FRAME_SIZE); //gobble up any checksum
	_sdspi_deassert_cs(cfg);

	//verify the checksum on data read

	return count;
}

int _sdspi_transfer(const device_cfg_t * cfg, const uint8_t * data_out, uint8_t * data_in, int nbyte){
	int i;
	for(i=0; i < nbyte; i++){
		if( data_out == 0 ){
			if( data_in != 0 ){
				data_in[i] = mcu_spi_swap(cfg->periph.port, (void*)0xFF);
			} else {
				mcu_spi_swap(cfg->periph.port, (void*)0xFF);
			}
		} else if( data_in == 0) {
			mcu_spi_swap(cfg->periph.port, (void*)(ssize_t)data_out[i]);
		} else {
			data_in[i] = mcu_spi_swap(cfg->periph.port, (void*)(ssize_t)data_out[i]);
		}
	}
	return nbyte;
}

int _sdspi_parse_response(uint8_t * response, int num, sdspi_r_t * r, uint32_t * arg){
	int i;
	uint8_t * arg_u8 = (uint8_t *)arg;
	for(i=0; i < CMD_FRAME_SIZE; i++){
		if( (response[i] & 0x80) == 0 ){
			//this is the start of the response
			switch(num){
			case 1:
				r->u8[0] = response[i];
				return true;
			case 2:
				r->u8[0] = response[i];
				r->u8[1] = response[i+1];
				return true;
			case 3:
				memcpy(r, response+i, sizeof(sdspi_r3_t));
				if( arg != 0 ){
					//convert to little endian
					arg_u8[0] = r->r3.arg[3];
					arg_u8[1] = r->r3.arg[2];
					arg_u8[2] = r->r3.arg[1];
					arg_u8[3] = r->r3.arg[0];
				}

				return true;
			}
		}
	}
	return false;
}

sdspi_r1_t _sdspi_cmd_r1(const device_cfg_t * cfg, uint8_t cmd, uint32_t arg, uint8_t * r){
	uint8_t tmp[CMD_FRAME_SIZE];
	uint8_t * response;
	if( r == 0 ){
		response = tmp;
	} else {
		response = r;
	}
	sdspi_r_t ret;
	_sdspi_send_cmd(cfg, cmd, arg, response);
	memset(&r, 0xFF, sizeof(sdspi_r_t));
	if( _sdspi_parse_response(response, 1, &ret, 0) == false ){
		memset(&ret, 0xFF, sizeof(sdspi_r_t));
	}
	return ret.r1;
}

/*
sdspi_r2_t _sdspi_cmd_r2(const device_cfg_t * cfg, uint8_t cmd, uint32_t arg, uint8_t * r){
	uint8_t tmp[CMD_FRAME_SIZE];
	uint8_t * response;
	if( r == 0 ){
		response = tmp;
	} else {
		response = r;
	}
	sdspi_r_t ret;
	_sdspi_send_cmd(cfg, cmd, arg, response);
	memset(&ret, 0xFF, sizeof(sdspi_r_t));
	if( _sdspi_parse_response(response, 2, &ret, 0) == false ){
		memset(&ret, 0xFF, sizeof(sdspi_r_t));
	}
	return ret.r2;
}
 */

sdspi_r3_t _sdspi_cmd_r3(const device_cfg_t * cfg, uint8_t cmd, uint32_t arg){
	uint8_t response[CMD_FRAME_SIZE];
	sdspi_r_t r;
	_sdspi_send_cmd(cfg, cmd, arg, response);
	memset(&r, 0xFF, sizeof(sdspi_r_t));
	if( _sdspi_parse_response(response, 3, &r, 0) == false ){
		memset(&r, 0xFF, sizeof(sdspi_r_t));
	}
	return r.r3;
}




/*! @} */


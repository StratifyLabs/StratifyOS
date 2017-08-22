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


#include <device/sd_spi_local.h>
#include <errno.h>
#include <unistd.h>
#include "cortexm/cortexm.h"
#include "mcu/pio.h"
#include "mcu/spi.h"
#include "mcu/core.h"
#include "mcu/crc.h"
#include "mcu/debug.h"
#include "mcu/wdt.h"

#include "device/sd_spi.h"
#include "mcu/core.h"
#include "cortexm/task.h"

#define BLOCK_SIZE 512
#define CMD_FRAME_SIZE 16

#define LONG_DELAY 250
#define SHORT_DELAY 100

#define FLAG_PROTECTED (1<<0)
#define FLAG_SDSC (1<<1)


static int _sd_spi_is_sdsc(const devfs_handle_t * cfg);

static int _sd_spi_erase_blocks(const devfs_handle_t * cfg, uint32_t block_num, uint32_t end_block);
static int _sd_spi_busy(const devfs_handle_t * cfg);
static int _sd_spi_status(const devfs_handle_t * cfg, uint8_t * buf);
static int _sd_spi_csd(const devfs_handle_t * cfg, uint8_t * buf);

static sd_spi_r1_t _sd_spi_cmd_r1(const devfs_handle_t * cfg, uint8_t cmd, uint32_t arg, uint8_t * r);
//static sd_spi_r2_t _sd_spi_cmd_r2(const devfs_handle_t * cfg, uint8_t cmd, uint32_t arg, uint8_t * r);
static sd_spi_r3_t _sd_spi_cmd_r3(const devfs_handle_t * cfg, uint8_t cmd, uint32_t arg);


static int _sd_spi_send_cmd(const devfs_handle_t * cfg, uint8_t cmd, uint32_t arg, uint8_t * response);
static int _sd_spi_parse_response(uint8_t * response, int num, sd_spi_r_t * r, uint32_t * arg);
static int _sd_spi_parse_data(uint8_t * dest, int nbyte, int count, uint8_t token, uint8_t * response);
static int _sd_spi_read_data(const devfs_handle_t * cfg, void * data, int nbyte, uint8_t token, uint8_t * first_response);
//static int _sd_spi_write_data(const devfs_handle_t * cfg, const void * data, int nbyte);

static int _sd_spi_transfer(const devfs_handle_t * cfg, const uint8_t * data_out, uint8_t * data_in, int nbyte);

static int _sd_spi_try_read(const devfs_handle_t * handle, int first);

static void _sd_spi_deassert_cs(const devfs_handle_t * handle){
	const sd_spi_config_t * config = handle->config;
	devfs_handle_t pio_handle;
	pio_handle.port = config->cs.port;
	pio_handle.config = 0;

	mcu_pio_setmask(&pio_handle, (void*)(ssize_t)(1<<config->cs.pin));
}

static void _sd_spi_assert_cs(const devfs_handle_t * handle){
	const sd_spi_config_t * config = handle->config;
	devfs_handle_t pio_handle;
	pio_handle.port = config->cs.port;
	pio_handle.config = 0;

	mcu_pio_clrmask(&pio_handle, (void*)(ssize_t)(1<<config->cs.pin));
}

static void _sd_spi_state_callback(const devfs_handle_t * handle, int err, int nbyte);


int sd_spi_open(const devfs_handle_t * handle){
	int err;
	pio_attr_t attr;
	//sd_spi_state_t * state = (sd_spi_state_t*)cfg->state;
	const sd_spi_config_t * config = handle->config;
	devfs_handle_t pio_handle;
	pio_handle.port = config->cs.port;
	pio_handle.config = 0;

	/*
	spi_attr_t spi_cfg;
	spi_cfg.pin_assign = cfg->pin_assign;
	spi_cfg.width = cfg->pcfg.spi.width;
	spi_cfg.mode = cfg->pcfg.spi.mode;
	spi_cfg.format = cfg->pcfg.spi.format;
	spi_cfg.bitrate = cfg->bitrate;
	spi_cfg.master = SPI_ATTR_MASTER;
	 */
	err = mcu_spi_open(handle);
	if ( err < 0 ){
		return err;
	}

	if( (err = mcu_spi_ioctl(handle, I_SPI_SETATTR, (void*)&(config->spi.attr))) < 0 ){
		return err;
	}

	_sd_spi_deassert_cs(handle);
	attr.o_pinmask = (1<<config->cs.pin);
	attr.o_flags = PIO_FLAG_SET_OUTPUT | PIO_FLAG_IS_DIRONLY;
	mcu_pio_setattr(&pio_handle, &attr);

	//The device is ready to use
	return 0;
}


void _sd_spi_state_callback(const devfs_handle_t * handle, int err, int nbyte){
	sd_spi_state_t * state = handle->state;
	if( state->handler.callback ){
		*(state->nbyte) = nbyte;
		if( nbyte < 0 ){
			struct _reent * reent = task_table[state->op.tid].reent;
			reent->_errno = err;
		}
		state->handler.callback(state->handler.context, 0);
		state->handler.callback = 0;
	}
}

static int continue_spi_read(void * handle, const mcu_event_t * ignore){
	//data has been read -- complete the operation
	int err = 0;
	sd_spi_state_t * state = ((const devfs_handle_t*)handle)->state;
	u16 checksum;
	u16 checksum_calc;

	if( state->count < 0 ){

		state->timeout++;
		if( state->timeout > 5000 ){
			//failed to read the data
			_sd_spi_deassert_cs(handle);

			_sd_spi_state_callback(handle, EIO, -2);
			return 0;
		}

		//try again to try the start of the data
		return _sd_spi_try_read(handle, 0);

	} else {
		//the read is complete

		_sd_spi_transfer(handle, 0, state->cmd, CMD_FRAME_SIZE); //gobble up the CRC
		checksum = (state->cmd[0] << 8) + state->cmd[1];
		checksum_calc = mcu_calc_crc16(0x0000, (const uint8_t *)state->buf, (size_t)*(state->nbyte));
		if( checksum != checksum_calc ){
			*(state->nbyte) = -1;
			err = EINVAL;
		}

		//execute the callback
		_sd_spi_state_callback(handle, err, *(state->nbyte));
	}

	return 0;
}

int _sd_spi_try_read(const devfs_handle_t * handle, int first){
	int ret;
	sd_spi_state_t * state = handle->state;
	state->count = _sd_spi_parse_data((uint8_t*)state->buf, *(state->nbyte), -1, SDSPI_START_BLOCK_TOKEN, state->cmd);
	if( state->count >= 0 ){
		state->op.nbyte = *(state->nbyte) - state->count;
		state->op.buf = (void*)&(state->buf[state->count]);
	} else {
		state->op.nbyte = CMD_FRAME_SIZE;
		state->op.buf = state->cmd;
	}
	state->op.handler.context = (void*)handle;
	state->op.handler.callback = continue_spi_read;


	if( first != 0 ){
		//send the command for the first time
		ret = mcu_spi_read(handle, &(state->op));
		return ret;
	}

	if( (ret = mcu_spi_read(handle, &(state->op))) != 0 ){
		_sd_spi_state_callback(handle, EINVAL, -5);
		return 0;
	}
	return 1;
}

int sd_spi_read(const devfs_handle_t * handle, devfs_async_t * rop){
	//first write the header command
	sd_spi_state_t * state = handle->state;
	sd_spi_r1_t r1;
	u32 loc;

	if( rop->nbyte != BLOCK_SIZE ){
		errno = EINVAL;
		return -1;
	}

	if( _sd_spi_busy(handle) != 0 ){
		errno = EBUSY;
		return -1;
	}

	state->handler.context = rop->handler.context;
	state->handler.callback = rop->handler.callback;
	state->nbyte = &(rop->nbyte);
	state->buf = rop->buf;
	state->timeout = 0;
	state->op.tid = rop->tid;

	if( _sd_spi_is_sdsc(handle) ){
		loc = rop->loc*BLOCK_SIZE;
	} else {
		loc = rop->loc;
	}

	r1 = _sd_spi_cmd_r1(handle, SDSPI_CMD17_READ_SINGLE_BLOCK, loc, state->cmd);
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

	_sd_spi_assert_cs(handle);
	cortexm_delay_us(LONG_DELAY);

	return _sd_spi_try_read(handle, 1);
}

static int continue_spi_write(void * handle, const mcu_event_t * ignore){
	sd_spi_state_t * state = ((const devfs_handle_t *)handle)->state;

	uint16_t checksum;

	//calculate and write the checksum
	checksum = mcu_calc_crc16(0x0000, (const uint8_t*)state->buf, *(state->nbyte));

	//finish the write
	state->cmd[0] = checksum >> 8;
	state->cmd[1] = checksum;
	state->cmd[2] = 0xFF;
	state->cmd[3] = 0xFF;
	state->cmd[4] = 0xFF;
	_sd_spi_transfer(handle, state->cmd, state->cmd, 5); //send dummy CRC
	_sd_spi_deassert_cs(handle);

	if( (state->cmd[2] & 0x1F) == 0x05 ){
		//data was accepted
		_sd_spi_state_callback(handle, 0, *(state->nbyte));
	} else {
		//data was not accepted
		_sd_spi_state_callback(handle, EIO, -1);
	}

	return 0;
}


int sd_spi_write(const devfs_handle_t * handle, devfs_async_t * wop){

	sd_spi_state_t * state = handle->state;
	sd_spi_r1_t r1;
	u32 loc;


	if( wop->nbyte != BLOCK_SIZE ){
		errno = EINVAL;
		return -1;
	}

	//check to see if device is busy
	if( _sd_spi_busy(handle) != 0 ){
		errno = EBUSY;
		return -1;
	}

	state->handler.context = wop->handler.context;
	state->handler.callback = wop->handler.callback;
	state->nbyte = &(wop->nbyte);
	state->buf = wop->buf;
	state->timeout = 0;

	if( _sd_spi_is_sdsc(handle) ){
		loc = wop->loc*BLOCK_SIZE;
	} else {
		loc = wop->loc;
	}

	r1 = _sd_spi_cmd_r1(handle, SDSPI_CMD24_WRITE_SINGLE_BLOCK, loc, state->cmd);
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

	_sd_spi_assert_cs(handle);
	cortexm_delay_us(LONG_DELAY);
	_sd_spi_transfer(handle, state->cmd, 0, 2);

	state->op.nbyte = wop->nbyte;
	state->op.buf = wop->buf;
	state->op.handler.context = (void*)handle;
	state->op.handler.callback = continue_spi_write;
	state->op.tid = wop->tid;

	return mcu_spi_write(handle, &(state->op));
}

int sd_spi_ioctl(const devfs_handle_t * handle, int request, void * ctl){
	sd_spi_state_t * state = (sd_spi_state_t*)handle->state;
	const sd_spi_config_t * config = handle->config;
	drive_info_t * info = ctl;
	drive_attr_t * attr = ctl;
	sd_spi_r_t resp;
	u32 o_flags;
	spi_attr_t spi_attr;
	int timeout;

	u16 erase_size;
	u16 erase_timeout;

	u8 buffer[64];

	switch(request){
	case I_DRIVE_SETATTR:
		o_flags = attr->o_flags;
		if( o_flags & (DRIVE_FLAG_ERASE_BLOCKS|DRIVE_FLAG_ERASE_DEVICE) ){
			if( state->flags & FLAG_PROTECTED ){
				errno = EROFS;
				return -1;
			}

			if( o_flags & DRIVE_FLAG_ERASE_BLOCKS ){

				//erase blocks in a sequence
				if( _sd_spi_busy(handle) != 0 ){
					errno = EBUSY;
					return -1;
				}
				return _sd_spi_erase_blocks(handle, attr->start, attr->end);
			}
		}

		if( o_flags & DRIVE_FLAG_INIT ){
			state->flags = 0;

			memcpy(&spi_attr, &(config->spi.attr), sizeof(spi_attr_t));
			spi_attr.freq = 400000;

			if( mcu_spi_setattr(handle, &spi_attr) < 0 ){
				mcu_debug_printf("SD_SPI: setattr failed\n");
				return -2;
			}


			cortexm_delay_us(500);

			//init sequence
			//apply at least 74 init clocks with DI and CS high
			_sd_spi_deassert_cs(handle);
			cortexm_delay_us(LONG_DELAY);
			_sd_spi_transfer(handle, 0, 0, CMD_FRAME_SIZE*6);
			cortexm_delay_us(LONG_DELAY);


			cortexm_delay_us(500);

			resp.r1 = _sd_spi_cmd_r1(handle, SDSPI_CMD0_GO_IDLE_STATE, 0, 0);
			if( resp.r1.start == 1 ){
				errno = EIO;
				mcu_debug_printf("SD_SPI: Failed GO IDLE\n");
				return -4;
			}

			if( resp.r1.idle != 1 ){
				errno = EIO;
				mcu_debug_printf("SD_SPI: Failed No IDLE 0x%X\n", resp.r1.u8);
				return -5;
			}

			//send CMD8
			resp.r3 = _sd_spi_cmd_r3(handle, SDSPI_CMD8_SEND_IF_COND, 0x01AA);
			if( resp.r3.r1.u8 != 0x01 ){
				errno = EIO;
				mcu_debug_printf("SD_SPI: Failed NO IF COND\n");
				return -6;
			}

			//disable write protection (SD Cards only)
			resp.r1 = _sd_spi_cmd_r1(handle, SDSPI_CMD28_SET_WRITE_PROT, 0, 0);
			if( resp.r1.u8 == 0x01 ){
				state->flags |= FLAG_SDSC;

				//set block len
				resp.r1 = _sd_spi_cmd_r1(handle, SDSPI_CMD16_SET_BLOCKLEN, BLOCK_SIZE, 0);
				if( resp.r1.u8 != 0x01 ){
					errno = EIO;
					mcu_debug_printf("SD_SPI: Failed NO 16 BLOCK LEN\n");
					return -7;
				}
			}

			//enable checksums
			resp.r1 = _sd_spi_cmd_r1(handle, SDSPI_CMD59_CRC_ON_OFF, 0xFFFFFFFF, 0);
			if( resp.r1.u8 != 0x01 ){
				errno = EIO;
				mcu_debug_printf("SD_SPI: Failed NO 59\n");
				return -7;
			}

			timeout = 0;

			const int timeout_value = 2000;

			do {
				resp.r1 = _sd_spi_cmd_r1(handle, SDSPI_CMD55_APP_CMD, 0, 0);  //send 55
				if( resp.r1.u8 != 0x01 ){
					errno = EIO;
					mcu_debug_printf("SD_SPI: Failed NO 55\n");
					return -7;
				}

				resp.r1 = _sd_spi_cmd_r1(handle, SDSPI_ACMD41_SD_SEND_OP_COND, 1<<30, 0);  //indicate that HC is supported
				if( (resp.r1.u8 != 0x01) && (resp.r1.u8 != 0x00) ){  //this takes awhile to return to zero
					errno = EIO;
					mcu_debug_printf("SD_SPI: Failed HC?\n");
					return -8;
				}
				timeout++;
				//reset wdt
				mcu_wdt_priv_reset(0);
			} while( (resp.r1.u8 != 0x00) && (timeout < timeout_value) );

			if( timeout == timeout_value ){
				errno = EIO;
				mcu_debug_printf("SD_SPI: Failed TIMEOUT\n");
				return -9;
			}

			memcpy(&spi_attr, &(config->spi.attr), sizeof(spi_attr_t));
			if( mcu_spi_setattr(handle, &spi_attr) < 0 ){
				mcu_debug_printf("SD_SPI: Failed BITRATE\n");
				return -10;
			}

			_sd_spi_assert_cs(handle);
			cortexm_delay_us(LONG_DELAY);
			_sd_spi_transfer(handle, 0, 0, CMD_FRAME_SIZE);
			_sd_spi_deassert_cs(handle);

			mcu_debug_printf("SD_SPI: INIT SUCCESS 0x%lX\n", state->flags);

			return 0;


		}

		break;

	case I_SDSPI_STATUS:
		break;

	case I_DRIVE_ISBUSY:
		return _sd_spi_busy(handle);

	case I_DRIVE_GETINFO:

		if( _sd_spi_busy(handle) != 0 ){
			errno = EBUSY;
			return -1;
		}

		_sd_spi_assert_cs(handle);
		cortexm_delay_us(LONG_DELAY);
		_sd_spi_transfer(handle, 0, 0, CMD_FRAME_SIZE);
		_sd_spi_deassert_cs(handle);

		info->o_flags = DRIVE_FLAG_ERASE_BLOCKS|DRIVE_FLAG_INIT;

		//Write block size and address are fixed to BLOCK_SIZE
		info->address_size = BLOCK_SIZE;
		info->write_block_size = info->address_size;

		//This is from CSD C_Size and TRANS_SPEED
		if( _sd_spi_csd(handle, buffer) < 0 ){
			errno = EIO;
			return -1;
		}

		uint32_t block_len;
		uint32_t c_size;
		uint32_t c_mult;

		if( _sd_spi_is_sdsc(handle) ){
			c_size = (((buffer[6] & 0x03) << 10) + (buffer[7] << 2) + (buffer[8] >> 6));
			c_mult = 1<<(((buffer[9] & 0x03) << 1) + (buffer[10] >> 7) + 2);
			block_len = 1 << (buffer[5] & 0x0F);
			info->num_write_blocks = (c_size+1) * c_mult * block_len / BLOCK_SIZE;
		} else {
			c_size = (((buffer[7] & 63) << 16) + (buffer[8] << 8) + buffer[9] );
			info->num_write_blocks = (c_size+1) * 1024*512 / BLOCK_SIZE;  //capacity = (C_SIZE+1)*512KByte -- block is capacity / BLOCK_SIZE
		}
		info->bitrate = 25*1000000;  //TRAN_SPEED should always be 25MHz

		//need to read Status to get AU_Size, ERASE_SIZE, ERASE_TIMEOUT
		if( _sd_spi_status(handle, buffer) < 0 ){
			errno = EIO;
			return -2;
		}
		info->erase_block_size = (16*1024) << ((buffer[10]-1) >> 4);
		//ERASE_TIMEOUT divided by ERASE_SIZE
		erase_size = ((buffer[11] << 8) + buffer[12]);
		if( erase_size == 0 ){
			info->erase_block_time = -1;
			info->erase_device_time = -1;
		} else {
			erase_timeout = ((buffer[13] << 8) + buffer[14]);
			info->erase_block_time = erase_timeout / erase_size;
			info->erase_device_time = info->erase_block_time * info->num_write_blocks * info->write_block_size / info->erase_block_size;
		}
		return 0;

	default:
		return mcu_spi_ioctl(handle, request, ctl);
	}
	return 0;
}

int sd_spi_close(const devfs_handle_t * cfg){
	return 0;
}

int _sd_spi_is_sdsc(const devfs_handle_t * cfg){
	sd_spi_state_t * state = (sd_spi_state_t*)cfg->state;

	if( state->flags & FLAG_SDSC ){
		return 1;
	}

	return 0;

}

int _sd_spi_erase_blocks(const devfs_handle_t * cfg, uint32_t block_num, uint32_t end_block){
	sd_spi_r_t r;

	if( _sd_spi_is_sdsc(cfg) ){
		block_num*=BLOCK_SIZE;
		end_block*=BLOCK_SIZE;
	}

	//cmd32, 33, then 38
	r.r1 = _sd_spi_cmd_r1(cfg, SDSPI_CMD32_ERASE_WR_BLK_START, block_num, 0);
	if( r.r1.u8 != 0 ){
		return -1;
	}
	r.r1 = _sd_spi_cmd_r1(cfg, SDSPI_CMD33_ERASE_WR_BLK_END, end_block, 0);
	if( r.r1.u8 != 0 ){
		return -1;
	}
	r.r1 = _sd_spi_cmd_r1(cfg, SDSPI_CMD38_ERASE, 0, 0);
	if( r.r1.u8 != 0 ){
		return -1;
	}

	return 0;
}

int _sd_spi_busy(const devfs_handle_t * cfg){
	uint8_t c;
	_sd_spi_assert_cs(cfg);
	cortexm_delay_us(LONG_DELAY);
	c = mcu_spi_swap(cfg, (void*)0xFF);
	_sd_spi_deassert_cs(cfg);
	return (c == 0x00);
}

int _sd_spi_status(const devfs_handle_t * cfg, uint8_t * buf){
	sd_spi_r_t resp;
	int ret;
	uint8_t tmp[CMD_FRAME_SIZE];

	resp.r1 = _sd_spi_cmd_r1(cfg, SDSPI_CMD55_APP_CMD, 0, tmp);
	if( resp.r1.u8 != 0x00 ){
		errno = EIO;
		return -1;
	}

	resp.r1 = _sd_spi_cmd_r1(cfg, SDSPI_CMD13_SD_STATUS, 0, tmp);
	if( resp.r1.u8 != 0x00 ){
		errno = EIO;
		return -1;
	}

	//now read the data
	ret = _sd_spi_read_data(cfg, buf, sizeof(_sd_spi_status_t), SDSPI_START_BLOCK_TOKEN, tmp);
	if( ret < 0 ){
		errno = EIO;
	}

	return ret;
}

int _sd_spi_csd(const devfs_handle_t * cfg, uint8_t * buf){
	sd_spi_r_t resp;
	int ret;
	uint8_t tmp[CMD_FRAME_SIZE];

	resp.r1 = _sd_spi_cmd_r1(cfg, SDSPI_CMD9_SEND_CSD, 0, tmp);
	if( resp.r1.u8 != 0x00 ){
		errno = EIO;
		return -1;
	}

	//now read the data
	ret = _sd_spi_read_data(cfg, buf, sizeof(sd_spi_csd_t), SDSPI_START_BLOCK_TOKEN, tmp);
	if( ret < 0 ){
		errno = EIO;
	}

	return ret;
}

int _sd_spi_send_cmd(const devfs_handle_t * cfg, uint8_t cmd, uint32_t arg, uint8_t * response){
	uint8_t buffer[CMD_FRAME_SIZE];
	int i;
	int ret;
	int retries;
	sd_spi_r_t resp;
	memset(buffer, 0xFF, CMD_FRAME_SIZE);
	buffer[0] = 0x40 | cmd;
	buffer[1] = arg >> 24;
	buffer[2] = arg >> 16;
	buffer[3] = arg >> 8;
	buffer[4] = arg;
	buffer[5] = mcu_calc_crc7(0, buffer, 5);

	retries = 0;
	do {
		//read the response
		_sd_spi_assert_cs(cfg);
		cortexm_delay_us(LONG_DELAY);
		_sd_spi_transfer(cfg, buffer, response, CMD_FRAME_SIZE);
		cortexm_delay_us(LONG_DELAY);
		_sd_spi_deassert_cs(cfg);

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

int _sd_spi_parse_data(uint8_t * dest, int nbyte, int count, uint8_t token, uint8_t * response){
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

int _sd_spi_read_data(const devfs_handle_t * cfg, void * data, int nbyte, uint8_t token, uint8_t * first_response){
	//look through the first response for the data token
	int timeout;
	uint8_t response[CMD_FRAME_SIZE];
	int count;

	count = _sd_spi_parse_data((uint8_t*)data, nbyte, -1, token, first_response);

	timeout = 0;
	while( count < nbyte ){
		_sd_spi_assert_cs(cfg);
		cortexm_delay_us(LONG_DELAY);
		if( count >= 0 ){
			_sd_spi_transfer(cfg, 0, (uint8_t*)data + count, nbyte - count);
			count = nbyte;
		} else {
			_sd_spi_transfer(cfg, 0, response, CMD_FRAME_SIZE);
			count = _sd_spi_parse_data((uint8_t*)data, nbyte, count, token, response);
		}
		_sd_spi_deassert_cs(cfg);


		timeout++;
		if( timeout > 100 ){
			return -1;
		}
	}

	_sd_spi_assert_cs(cfg);
	cortexm_delay_us(LONG_DELAY);
	_sd_spi_transfer(cfg, 0, response, CMD_FRAME_SIZE); //gobble up any checksum
	_sd_spi_deassert_cs(cfg);

	//verify the checksum on data read

	return count;
}

int _sd_spi_transfer(const devfs_handle_t * cfg, const uint8_t * data_out, uint8_t * data_in, int nbyte){
	int i;
	for(i=0; i < nbyte; i++){
		if( data_out == 0 ){
			if( data_in != 0 ){
				data_in[i] = mcu_spi_swap(cfg, (void*)0xFF);
			} else {
				mcu_spi_swap(cfg, (void*)0xFF);
			}
		} else if( data_in == 0) {
			mcu_spi_swap(cfg, (void*)(ssize_t)data_out[i]);
		} else {
			data_in[i] = mcu_spi_swap(cfg, (void*)(ssize_t)data_out[i]);
		}
	}
	return nbyte;
}

int _sd_spi_parse_response(uint8_t * response, int num, sd_spi_r_t * r, uint32_t * arg){
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
				memcpy(r, response+i, sizeof(sd_spi_r3_t));
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

sd_spi_r1_t _sd_spi_cmd_r1(const devfs_handle_t * cfg, uint8_t cmd, uint32_t arg, uint8_t * r){
	uint8_t tmp[CMD_FRAME_SIZE];
	uint8_t * response;
	if( r == 0 ){
		response = tmp;
	} else {
		response = r;
	}
	sd_spi_r_t ret;
	_sd_spi_send_cmd(cfg, cmd, arg, response);
	memset(&r, 0xFF, sizeof(sd_spi_r_t));
	if( _sd_spi_parse_response(response, 1, &ret, 0) == false ){
		memset(&ret, 0xFF, sizeof(sd_spi_r_t));
	}
	return ret.r1;
}

/*
sd_spi_r2_t _sd_spi_cmd_r2(const devfs_handle_t * cfg, uint8_t cmd, uint32_t arg, uint8_t * r){
	uint8_t tmp[CMD_FRAME_SIZE];
	uint8_t * response;
	if( r == 0 ){
		response = tmp;
	} else {
		response = r;
	}
	sd_spi_r_t ret;
	_sd_spi_send_cmd(cfg, cmd, arg, response);
	memset(&ret, 0xFF, sizeof(sd_spi_r_t));
	if( _sd_spi_parse_response(response, 2, &ret, 0) == false ){
		memset(&ret, 0xFF, sizeof(sd_spi_r_t));
	}
	return ret.r2;
}
 */

sd_spi_r3_t _sd_spi_cmd_r3(const devfs_handle_t * cfg, uint8_t cmd, uint32_t arg){
	uint8_t response[CMD_FRAME_SIZE];
	sd_spi_r_t r;
	_sd_spi_send_cmd(cfg, cmd, arg, response);
	memset(&r, 0xFF, sizeof(sd_spi_r_t));
	if( _sd_spi_parse_response(response, 3, &r, 0) == false ){
		memset(&r, 0xFF, sizeof(sd_spi_r_t));
	}
	return r.r3;
}




/*! @} */


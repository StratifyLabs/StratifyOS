/* Copyright 2011-2018 Tyler Gilbert; 
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


#include "sos/sos.h"

#include <errno.h>
#include <unistd.h>
#include "cortexm/cortexm.h"
#include "drive_sdspi_local.h"
#include "mcu/pio.h"
#include "mcu/spi.h"
#include "mcu/core.h"
#include "mcu/crc.h"
#include "mcu/debug.h"
#include "mcu/wdt.h"

#include "device/drive_sdspi.h"
#include "mcu/core.h"
#include "cortexm/task.h"

#define BLOCK_SIZE 512
#define CMD_FRAME_SIZE 16

#define LONG_DELAY 500
#define SHORT_DELAY 100

#define FLAG_PROTECTED (1<<0)
#define FLAG_SDSC (1<<1)


static int is_sdsc(const devfs_handle_t * handle);

static int erase_blocks(const devfs_handle_t * handle, uint32_t block_num, uint32_t end_block);
static int is_busy(const devfs_handle_t * handle);
static int get_status(const devfs_handle_t * handle, uint8_t * buf);
static int exec_csd(const devfs_handle_t * handle, uint8_t * buf);

static drive_sdspi_r1_t exec_cmd_r1(const devfs_handle_t * handle, uint8_t cmd, uint32_t arg, uint8_t * r);
//static drive_sdspi_r2_t _drive_sdspi_cmd_r2(const devfs_handle_t * handle, uint8_t cmd, uint32_t arg, uint8_t * r);
static drive_sdspi_r3_t exec_cmd_r3(const devfs_handle_t * handle, uint8_t cmd, uint32_t arg);


static int send_cmd(const devfs_handle_t * handle, uint8_t cmd, uint32_t arg, uint8_t * response);
static int parse_response(uint8_t * response, int num, drive_sdspi_r_t * r, uint32_t * arg);
static int parse_data(uint8_t * dest, int nbyte, int count, uint8_t token, uint8_t * response);
static int read_data(const devfs_handle_t * handle, void * data, int nbyte, uint8_t token, uint8_t * first_response);
//static int _drive_sdspi_write_data(const devfs_handle_t * handle, const void * data, int nbyte);

static int spi_transfer(const devfs_handle_t * handle, const uint8_t * data_out, uint8_t * data_in, int nbyte);

static int try_read(const devfs_handle_t * handle, int first);
static int continue_spi_read(void * handle, const mcu_event_t * ignore);
static int continue_spi_write(void * handle, const mcu_event_t * ignore);

static void deassert_chip_select(const devfs_handle_t * handle){
	const drive_sdspi_config_t * config = handle->config;
	devfs_handle_t pio_handle;
	pio_handle.port = config->cs.port;
	pio_handle.config = 0;

	mcu_pio_setmask(&pio_handle, (void*)(ssize_t)(1<<config->cs.pin));
}

static void assert_chip_select(const devfs_handle_t * handle){
	const drive_sdspi_config_t * config = handle->config;
	devfs_handle_t pio_handle;
	pio_handle.port = config->cs.port;
	pio_handle.config = 0;

	mcu_pio_clrmask(&pio_handle, (void*)(ssize_t)(1<<config->cs.pin));
}

static void state_callback(const devfs_handle_t * handle, int err, int nbyte);


int drive_sdspi_open(const devfs_handle_t * handle){
	int err;
	pio_attr_t attr;
	const drive_sdspi_config_t * config = handle->config;
	devfs_handle_t pio_handle;
	pio_handle.port = config->cs.port;
	pio_handle.config = 0;

	err = mcu_spi_open(handle);
	if ( err < 0 ){
		return err;
	}

	if( (err = mcu_spi_ioctl(handle, I_SPI_SETATTR, (void*)&(config->spi.attr))) < 0 ){
		return err;
	}

	deassert_chip_select(handle);
	attr.o_pinmask = (1<<config->cs.pin);
	attr.o_flags = PIO_FLAG_SET_OUTPUT | PIO_FLAG_IS_DIRONLY;
	mcu_pio_setattr(&pio_handle, &attr);

	//The device is ready to use
	return 0;
}


void state_callback(const devfs_handle_t * handle, int err, int nbyte){
	drive_sdspi_state_t * state = handle->state;
	if( state->handler.callback ){
		*(state->nbyte) = nbyte;
		if( nbyte < 0 ){
			struct _reent * reent = sos_task_table[state->op.tid].reent;
			reent->_errno = err;
		}
		state->handler.callback(state->handler.context, 0);
		state->handler.callback = 0;
	}
}

int continue_spi_read(void * handle, const mcu_event_t * ignore){
	//data has been read -- complete the operation
	int err = 0;
	drive_sdspi_state_t * state = ((const devfs_handle_t*)handle)->state;
	u16 checksum;
	u16 checksum_calc;

	if( state->count < 0 ){

		state->timeout++;
		if( state->timeout > 5000 ){
			//failed to read the data
			deassert_chip_select(handle);
			state_callback(handle, EIO, -2);
			return 0;
		}

		//try again to try the start of the data
		return try_read(handle, 0);

	} else {
		//the read is complete
		spi_transfer(handle, 0, state->cmd, CMD_FRAME_SIZE); //gobble up the CRC
		checksum = (state->cmd[0] << 8) + state->cmd[1];
		checksum_calc = mcu_calc_crc16(0x0000, 0x1021, (const uint8_t *)state->buf, (size_t)*(state->nbyte));
		if( checksum != checksum_calc ){
			mcu_debug_printf("Bad checksum 0x%04X != 0x%04X\n", checksum, checksum_calc);
			*(state->nbyte) = -1;
			err = EINVAL;
		}

		//execute the callback
		state_callback(handle, err, *(state->nbyte));
	}

	return 0;
}

int try_read(const devfs_handle_t * handle, int first){
	int ret;
	drive_sdspi_state_t * state = handle->state;
	state->count = parse_data((uint8_t*)state->buf, *(state->nbyte), -1, SDSPI_START_BLOCK_TOKEN, state->cmd);
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
		if( ret != 0 ){
			mcu_debug_printf("SPI READ FAILED (%d,%d)", SYSFS_GET_RETURN(ret), SYSFS_GET_RETURN_ERRNO(ret));
		}
		return ret;
	}

	if( (ret = mcu_spi_read(handle, &(state->op))) != 0 ){
		state_callback(handle, EINVAL, -5);
		mcu_debug_printf("BAD SPI READ\n");
		return 0;
	}
	return 1;
}

int drive_sdspi_read(const devfs_handle_t * handle, devfs_async_t * rop){
	//first write the header command
	drive_sdspi_state_t * state = handle->state;
	drive_sdspi_r1_t r1;
	u32 loc;

	if( rop->nbyte != BLOCK_SIZE ){
		return SYSFS_SET_RETURN(EINVAL);
	}

	if( is_busy(handle) != 0 ){
		return SYSFS_SET_RETURN(EBUSY);
	}

	state->handler.context = rop->handler.context;
	state->handler.callback = rop->handler.callback;
	state->nbyte = &(rop->nbyte);
	state->buf = rop->buf;
	state->timeout = 0;
	state->op.tid = rop->tid;

	if( is_sdsc(handle) ){
		loc = rop->loc*BLOCK_SIZE;
	} else {
		loc = rop->loc;
	}

	r1 = exec_cmd_r1(handle, SDSPI_CMD17_READ_SINGLE_BLOCK, loc, state->cmd);
	if( r1.u8 != 0x00 ){
		if( (r1.param_error) ){
			return SYSFS_SET_RETURN(EINVAL);
		}

		if( (r1.addr_error) ){
			return SYSFS_SET_RETURN(EINVAL);
		}

		if( (r1.erase_sequence_error) ){
			return SYSFS_SET_RETURN(EINVAL);
		}

		if( (r1.crc_error) ){
			return SYSFS_SET_RETURN(EINVAL);
		}

		if( (r1.illegal_command) ){
			return SYSFS_SET_RETURN(EINVAL);
		}

		return SYSFS_SET_RETURN(EIO);
	}

	assert_chip_select(handle);
	cortexm_delay_us(LONG_DELAY);



	return try_read(handle, 1);
}

int continue_spi_write(void * handle, const mcu_event_t * ignore){
	MCU_UNUSED_ARGUMENT(ignore);
	drive_sdspi_state_t * state = ((const devfs_handle_t *)handle)->state;

	uint16_t checksum;

	//calculate and write the checksum
	checksum = mcu_calc_crc16(0x0000,  0x1021, (const uint8_t*)state->buf, *(state->nbyte));

	//finish the write
	state->cmd[0] = checksum >> 8;
	state->cmd[1] = checksum;
	state->cmd[2] = 0xFF;
	state->cmd[3] = 0xFF;
	state->cmd[4] = 0xFF;
	spi_transfer(handle, state->cmd, state->cmd, 5); //send dummy CRC
	deassert_chip_select(handle);


	if( (state->cmd[2] & 0x1F) == 0x05 ){
		//data was accepted
		state_callback(handle, 0, *(state->nbyte));
	} else {
		//data was not accepted
		state_callback(handle, EIO, -1);
	}

	return 0;
}


int drive_sdspi_write(const devfs_handle_t * handle, devfs_async_t * wop){
	drive_sdspi_state_t * state = handle->state;
	drive_sdspi_r1_t r1;
	u32 loc;


	if( wop->nbyte != BLOCK_SIZE ){
		return SYSFS_SET_RETURN(EINVAL);
	}

	//check to see if device is busy
	if( is_busy(handle) != 0 ){
		return SYSFS_SET_RETURN(EBUSY);
	}

	state->handler.context = wop->handler.context;
	state->handler.callback = wop->handler.callback;
	state->nbyte = &(wop->nbyte);
	state->buf = wop->buf;
	state->timeout = 0;

	if( is_sdsc(handle) ){
		loc = wop->loc*BLOCK_SIZE;
	} else {
		loc = wop->loc;
	}

	r1 = exec_cmd_r1(handle, SDSPI_CMD24_WRITE_SINGLE_BLOCK, loc, state->cmd);
	if( r1.u8 != 0x00 ){
		if( (r1.addr_error) || (r1.param_error) ){
			return SYSFS_SET_RETURN(EINVAL);
		}

		return SYSFS_SET_RETURN(EIO);
	}

	state->cmd[0] = 0xFF;  //busy byte
	state->cmd[1] = SDSPI_START_BLOCK_TOKEN;

	assert_chip_select(handle);
	cortexm_delay_us(LONG_DELAY);
	spi_transfer(handle, state->cmd, 0, 2);

	state->op.nbyte = wop->nbyte;
	state->op.buf = wop->buf;
	state->op.handler.context = (void*)handle;
	state->op.handler.callback = continue_spi_write;
	state->op.tid = wop->tid;


	return mcu_spi_write(handle, &(state->op));
}

int drive_sdspi_ioctl(const devfs_handle_t * handle, int request, void * ctl){
	drive_sdspi_state_t * state = (drive_sdspi_state_t*)handle->state;
	const drive_sdspi_config_t * config = handle->config;
	drive_info_t * info = ctl;
	drive_attr_t * attr = ctl;
	drive_sdspi_r_t resp;
	u32 o_flags;
	int timeout;

	char spi_config[config->spi_config_size];
	spi_attr_t * spi_attr_p = ( spi_attr_t *)spi_config;

	u16 erase_size;
	u16 erase_timeout;

	u8 buffer[64];

	switch(request){
		case I_DRIVE_SETATTR:
			o_flags = attr->o_flags;
			if( o_flags & (DRIVE_FLAG_ERASE_BLOCKS|DRIVE_FLAG_ERASE_DEVICE) ){
				if( state->flags & FLAG_PROTECTED ){
					return SYSFS_SET_RETURN(EROFS);
				}

				if( o_flags & DRIVE_FLAG_ERASE_BLOCKS ){

					//erase blocks in a sequence
					if( is_busy(handle) != 0 ){
						return SYSFS_SET_RETURN(EBUSY);
					}
					int result = erase_blocks(handle, attr->start, attr->end);
					if( result < 0 ){
						return result;
					}
					return attr->end - attr->start;
				}
			}

			if( o_flags & DRIVE_FLAG_INIT ){
				state->flags = 0;

				memcpy(spi_config, &(config->spi), config->spi_config_size);
				spi_attr_p->freq = 400000;

				if( mcu_spi_setattr(handle, spi_config) < 0 ){
					mcu_debug_printf("SD_SPI: setattr failed\n");
					return SYSFS_SET_RETURN(EIO);
				}


				cortexm_delay_us(500);

				//init sequence
				//apply at least 74 init clocks with DI and CS high
				for(u32 i=0; i < 5; i++){
					deassert_chip_select(handle);
					cortexm_delay_us(LONG_DELAY*i);
					spi_transfer(handle, 0, 0, CMD_FRAME_SIZE*12);
					cortexm_delay_us(LONG_DELAY*i);

					cortexm_delay_us(LONG_DELAY*i);

					resp.r1 = exec_cmd_r1(handle, SDSPI_CMD0_GO_IDLE_STATE, 0, 0);
					if( resp.r1.start == 1 ){
						mcu_debug_printf("SD_SPI: Failed GO IDLE 0x%X\n", resp.r1.u8);
						if( i == 4 ){
							return SYSFS_SET_RETURN(EIO);
						}
					}

					if( resp.r1.idle != 1 ){
						mcu_debug_printf("SD_SPI: Failed No IDLE 0x%X\n", resp.r1.u8);
						if( i == 4 ){
							return SYSFS_SET_RETURN(EIO);
						}
					}
				}

				//send CMD8
				resp.r3 = exec_cmd_r3(handle, SDSPI_CMD8_SEND_IF_COND, 0x01AA);
				if( resp.r3.r1.u8 != 0x01 ){
					mcu_debug_printf("SD_SPI: Failed NO IF COND(0x%X)\n", resp.r3.r1.u8);
					return SYSFS_SET_RETURN(EIO);
				}

				//disable write protection (SD Cards only)
				resp.r1 = exec_cmd_r1(handle, SDSPI_CMD28_SET_WRITE_PROT, 0, 0);
				if( resp.r1.u8 == 0x01 ){
					state->flags |= FLAG_SDSC;

					//set block len
					resp.r1 = exec_cmd_r1(handle, SDSPI_CMD16_SET_BLOCKLEN, BLOCK_SIZE, 0);
					if( resp.r1.u8 != 0x01 ){
						mcu_debug_printf("SD_SPI: Failed NO 16 BLOCK LEN\n");
						return SYSFS_SET_RETURN(EIO);
					}
				}

				//enable checksums
				resp.r1 = exec_cmd_r1(handle, SDSPI_CMD59_CRC_ON_OFF, 0xFFFFFFFF, 0);
				if( resp.r1.u8 != 0x01 ){
					mcu_debug_printf("SD_SPI: Failed NO 59\n");
					return SYSFS_SET_RETURN(EIO);
				}

				timeout = 0;

				const int timeout_value = 2000;

				do {
					resp.r1 = exec_cmd_r1(handle, SDSPI_CMD55_APP_CMD, 0, 0);  //send 55
					if( resp.r1.u8 != 0x01 ){
						mcu_debug_printf("SD_SPI: Failed NO 55\n");
						return SYSFS_SET_RETURN(EIO);
					}

					resp.r1 = exec_cmd_r1(handle, SDSPI_ACMD41_SD_SEND_OP_COND, 1<<30, 0);  //indicate that HC is supported
					if( (resp.r1.u8 != 0x01) && (resp.r1.u8 != 0x00) ){  //this takes awhile to return to zero
						mcu_debug_printf("SD_SPI: Failed HC?\n");
						return SYSFS_SET_RETURN(EIO);
					}
					timeout++;
					//reset wdt
					mcu_wdt_root_reset(0);
				} while( (resp.r1.u8 != 0x00) && (timeout < timeout_value) );

				if( timeout == timeout_value ){
					mcu_debug_printf("SD_SPI: Failed TIMEOUT\n");
					return SYSFS_SET_RETURN(EIO);
				}

				//set with default attributes as intended by the system
				memcpy(spi_config, &(config->spi), config->spi_config_size);
				if( mcu_spi_setattr(handle, spi_config) < 0 ){
					mcu_debug_printf("SD_SPI: Failed BITRATE\n");
					return SYSFS_SET_RETURN(EIO);
				}

				assert_chip_select(handle);
				cortexm_delay_us(LONG_DELAY);
				spi_transfer(handle, 0, 0, CMD_FRAME_SIZE);
				deassert_chip_select(handle);

				mcu_debug_printf("SD_SPI: INIT SUCCESS 0x%lX\n", state->flags);

				return 0;


			}

			break;

		case I_DRIVE_ISBUSY:
			return is_busy(handle);

		case I_DRIVE_GETINFO:

			if( is_busy(handle) != 0 ){
				return SYSFS_SET_RETURN(EBUSY);
			}

			assert_chip_select(handle);
			cortexm_delay_us(LONG_DELAY);
			spi_transfer(handle, 0, 0, CMD_FRAME_SIZE);
			deassert_chip_select(handle);

			info->o_flags = DRIVE_FLAG_ERASE_BLOCKS|DRIVE_FLAG_INIT;

			//Write block size and address are fixed to BLOCK_SIZE
			info->addressable_size = BLOCK_SIZE;
			info->write_block_size = info->addressable_size;

			//This is from CSD C_Size and TRANS_SPEED
			if( exec_csd(handle, buffer) < 0 ){
				return SYSFS_SET_RETURN(EIO);
			}

			uint32_t block_len;
			uint32_t c_size;
			uint32_t c_mult;

			if( is_sdsc(handle) ){
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
			if( get_status(handle, buffer) < 0 ){
				return SYSFS_SET_RETURN(EIO);
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

int drive_sdspi_close(const devfs_handle_t * handle){
	return 0;
}

int is_sdsc(const devfs_handle_t * handle){
	drive_sdspi_state_t * state = (drive_sdspi_state_t*)handle->state;

	if( state->flags & FLAG_SDSC ){
		return 1;
	}

	return 0;

}

int erase_blocks(const devfs_handle_t * handle, uint32_t block_num, uint32_t end_block){
	drive_sdspi_r_t r;

	if( is_sdsc(handle) ){
		block_num*=BLOCK_SIZE;
		end_block*=BLOCK_SIZE;
	}

	//cmd32, 33, then 38
	r.r1 = exec_cmd_r1(handle, SDSPI_CMD32_ERASE_WR_BLK_START, block_num, 0);
	if( r.r1.u8 != 0 ){
		return -1;
	}
	r.r1 = exec_cmd_r1(handle, SDSPI_CMD33_ERASE_WR_BLK_END, end_block, 0);
	if( r.r1.u8 != 0 ){
		return -1;
	}
	r.r1 = exec_cmd_r1(handle, SDSPI_CMD38_ERASE, 0, 0);
	if( r.r1.u8 != 0 ){
		return -1;
	}

	return 0;
}

int is_busy(const devfs_handle_t * handle){
	uint8_t c;
	assert_chip_select(handle);
	cortexm_delay_us(LONG_DELAY);
	c = mcu_spi_swap(handle, (void*)0xFF);
	deassert_chip_select(handle);
	return (c == 0x00);
}

int get_status(const devfs_handle_t * handle, uint8_t * buf){
	drive_sdspi_r_t resp;
	int ret;
	uint8_t tmp[CMD_FRAME_SIZE];

	resp.r1 = exec_cmd_r1(handle, SDSPI_CMD55_APP_CMD, 0, tmp);
	if( resp.r1.u8 != 0x00 ){
		return SYSFS_SET_RETURN(EIO);
	}

	resp.r1 = exec_cmd_r1(handle, SDSPI_CMD13_SD_STATUS, 0, tmp);
	if( resp.r1.u8 != 0x00 ){
		return SYSFS_SET_RETURN(EIO);
	}

	//now read the data
	ret = read_data(handle, buf, sizeof(_drive_sdspi_status_t), SDSPI_START_BLOCK_TOKEN, tmp);
	if( ret < 0 ){
		return SYSFS_SET_RETURN(EIO);
	}

	return ret;
}

int exec_csd(const devfs_handle_t * handle, uint8_t * buf){
	drive_sdspi_r_t resp;
	int ret;
	uint8_t tmp[CMD_FRAME_SIZE];

	resp.r1 = exec_cmd_r1(handle, SDSPI_CMD9_SEND_CSD, 0, tmp);
	if( resp.r1.u8 != 0x00 ){
		return SYSFS_SET_RETURN(EIO);
	}

	//now read the data
	ret = read_data(handle, buf, sizeof(drive_sdspi_csd_t), SDSPI_START_BLOCK_TOKEN, tmp);
	if( ret < 0 ){
		return SYSFS_SET_RETURN(EIO);
	}

	return ret;
}

int send_cmd(const devfs_handle_t * handle, uint8_t cmd, uint32_t arg, uint8_t * response){
	uint8_t buffer[CMD_FRAME_SIZE];
	int i;
	int ret;
	int retries;
	drive_sdspi_r_t resp;
	//memset(&resp, 0, sizeof(resp));
	memset(buffer, 0xFF, CMD_FRAME_SIZE);
	buffer[0] = 0x40 | cmd;
	buffer[1] = arg >> 24;
	buffer[2] = arg >> 16;
	buffer[3] = arg >> 8;
	buffer[4] = arg;
	buffer[5] = mcu_calc_crc7(0, 0x09, buffer, 5);

	resp.r1.crc_error = 1;

	retries = 0;
	do {
		//read the response
		assert_chip_select(handle);
		cortexm_delay_us(LONG_DELAY);
		spi_transfer(handle, buffer, response, CMD_FRAME_SIZE);
		cortexm_delay_us(LONG_DELAY);
		deassert_chip_select(handle);

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

int parse_data(uint8_t * dest, int nbyte, int count, uint8_t token, uint8_t * response){
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

int read_data(const devfs_handle_t * handle, void * data, int nbyte, uint8_t token, uint8_t * first_response){
	//look through the first response for the data token
	int timeout;
	uint8_t response[CMD_FRAME_SIZE];
	int count;

	count = parse_data((uint8_t*)data, nbyte, -1, token, first_response);

	timeout = 0;
	while( count < nbyte ){
		assert_chip_select(handle);
		cortexm_delay_us(LONG_DELAY);
		if( count >= 0 ){
			spi_transfer(handle, 0, (uint8_t*)data + count, nbyte - count);
			count = nbyte;
		} else {
			spi_transfer(handle, 0, response, CMD_FRAME_SIZE);
			count = parse_data((uint8_t*)data, nbyte, count, token, response);
		}
		deassert_chip_select(handle);


		timeout++;
		if( timeout > 100 ){
			return -1;
		}
	}

	assert_chip_select(handle);
	cortexm_delay_us(LONG_DELAY);
	spi_transfer(handle, 0, response, CMD_FRAME_SIZE); //gobble up any checksum
	deassert_chip_select(handle);

	//verify the checksum on data read

	return count;
}

int spi_transfer(const devfs_handle_t * handle, const uint8_t * data_out, uint8_t * data_in, int nbyte){
	int i;
	for(i=0; i < nbyte; i++){
		if( data_out == 0 ){
			if( data_in != 0 ){
				data_in[i] = mcu_spi_swap(handle, (void*)0xFF);
			} else {
				mcu_spi_swap(handle, (void*)0xFF);
			}
		} else if( data_in == 0) {
			mcu_spi_swap(handle, (void*)(ssize_t)data_out[i]);
		} else {
			data_in[i] = mcu_spi_swap(handle, (void*)(ssize_t)data_out[i]);
		}
	}
	return nbyte;
}

int parse_response(uint8_t * response, int num, drive_sdspi_r_t * r, uint32_t * arg){
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
					memcpy(r, response+i, sizeof(drive_sdspi_r3_t));
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

drive_sdspi_r1_t exec_cmd_r1(const devfs_handle_t * handle, uint8_t cmd, uint32_t arg, uint8_t * r){
	uint8_t tmp[CMD_FRAME_SIZE];
	uint8_t * response;
	if( r == 0 ){
		response = tmp;
	} else {
		response = r;
	}
	drive_sdspi_r_t ret;
	send_cmd(handle, cmd, arg, response);
	memset(&r, 0xFF, sizeof(drive_sdspi_r_t));
	if( parse_response(response, 1, &ret, 0) == false ){
		memset(&ret, 0xFF, sizeof(drive_sdspi_r_t));
	}
	return ret.r1;
}

/*
drive_sdspi_r2_t _drive_sdspi_cmd_r2(const devfs_handle_t * handle, uint8_t cmd, uint32_t arg, uint8_t * r){
	 uint8_t tmp[CMD_FRAME_SIZE];
	 uint8_t * response;
	 if( r == 0 ){
		  response = tmp;
	 } else {
		  response = r;
	 }
	 drive_sdspi_r_t ret;
	 _drive_sdspi_send_cmd(handle, cmd, arg, response);
	 memset(&ret, 0xFF, sizeof(drive_sdspi_r_t));
	 if( _drive_sdspi_parse_response(response, 2, &ret, 0) == false ){
		  memset(&ret, 0xFF, sizeof(drive_sdspi_r_t));
	 }
	 return ret.r2;
}
 */

drive_sdspi_r3_t exec_cmd_r3(const devfs_handle_t * handle, uint8_t cmd, uint32_t arg){
	uint8_t response[CMD_FRAME_SIZE];
	drive_sdspi_r_t r;
	send_cmd(handle, cmd, arg, response);
	memset(&r, 0xFF, sizeof(drive_sdspi_r_t));
	if( parse_response(response, 3, &r, 0) == false ){
		memset(&r, 0xFF, sizeof(drive_sdspi_r_t));
	}
	return r.r3;
}




/*! @} */


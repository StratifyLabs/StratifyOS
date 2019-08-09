/* Copyright 2011; 
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
 * Please see http://www.coactionos.com/license.html for
 * licensing information.
 */

/*! \addtogroup LINK
 * @{
 *
 */

#include <errno.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include "mcu/arch.h"
#include "sos/sos.h"
#include "cortexm/cortexm.h"
#include "mcu/core.h"
#include "mcu/debug.h"
#include "mcu/flash.h"
#include "boot_link.h"
#include "boot_config.h"

#define FLASH_PORT 0

static bool is_erased = false;
const devfs_handle_t flash_dev = { .port = 0 };


static int read_flash(link_transport_driver_t * driver, int loc, int nbyte);
static int read_flash_callback(void * context, void * buf, int nbyte);

typedef struct {
	int err;
	int nbyte;
	link_op_t op;
	link_reply_t reply;
} link_data_t;


static void boot_link_cmd_none(link_transport_driver_t * driver, link_data_t * args);
static void boot_link_cmd_readserialno(link_transport_driver_t * driver, link_data_t * args);
static void boot_link_cmd_ioctl(link_transport_driver_t * driver, link_data_t * args);
static void boot_link_cmd_read(link_transport_driver_t * driver, link_data_t * args);

static void boot_link_cmd_reset_bootloader(link_transport_driver_t * driver, link_data_t * args);
static void erase_flash(link_transport_driver_t * driver);
static void boot_link_cmd_reset(link_transport_driver_t * driver, link_data_t * args);


void (* const boot_link_cmd_func_table[LINK_BOOTLOADER_CMD_TOTAL])(link_transport_driver_t *, link_data_t*) = {
		boot_link_cmd_none,
		boot_link_cmd_readserialno,
		boot_link_cmd_ioctl,
		boot_link_cmd_read
		};

void * boot_link_update(void * arg){
	int err;

	link_transport_driver_t * driver = arg;
	link_data_t data;

	if( (driver->handle = driver->open(0, 0)) == LINK_PHY_ERROR){
		return 0;
	}

	data.op.cmd = 0;
	err = 0;

	dstr("Enter update loop\n");
	while(1){
		//Wait for data to arrive on the USB
		while( 1 ){

			if ( (err = link_transport_slaveread(driver, &data.op, sizeof(link_op_t), NULL, NULL)) < 0 ){
				dstr("e:"); dint(err); dstr("\n");
				continue;
				mcu_core_reset(0, 0);
			}

			if( err > 0 ){
				break;
			}

		}

		dstr("EXEC CMD: "); dint(data.op.cmd); dstr("\n");
		if ( data.op.cmd < LINK_BOOTLOADER_CMD_TOTAL ){
			data.reply.err_number = 0;
			data.reply.err = 0;
			boot_link_cmd_func_table[data.op.cmd](driver, &data);
		} else {
			data.reply.err = -1;
			data.reply.err_number = EINVAL;
		}

		//send the reply
		if( data.op.cmd != 0 ){
			link_transport_slavewrite(driver, &data.reply, sizeof(data.reply), NULL, NULL);
			data.op.cmd = 0;
		}

	}
	return NULL;
}

void boot_link_cmd_none(link_transport_driver_t * driver, link_data_t * args){
	return;
}

void boot_link_cmd_readserialno(link_transport_driver_t * driver, link_data_t * args){
	char serialno[LINK_PACKET_DATA_SIZE];
	u32 tmp[4];
	int i, j;
	char * p = serialno;
	memset(serialno, 0, LINK_PACKET_DATA_SIZE);


	mcu_core_getserialno((mcu_sn_t*)tmp);
	for(j=3; j >= 0; j--){
		for(i=0; i < 8; i++){
			*p++ = htoc((tmp[j] >> 28) & 0x0F);
			tmp[j] <<= 4;
		}
	}

	args->reply.err = strlen(serialno);
	args->reply.err_number = 0;

	if( link_transport_slavewrite(driver, &args->reply, sizeof(args->reply), NULL, NULL) < 0 ){
		return;
	}

	if( link_transport_slavewrite(driver, serialno, args->reply.err, NULL, NULL) < 0 ){
		return;
	}

	args->op.cmd = 0; //reply was already sent

	boot_event(BOOT_EVENT_READ_SERIALNO, 0);

}

void boot_link_cmd_ioctl(link_transport_driver_t * driver, link_data_t * args){
	int err;
	int size;
	size = _IOCTL_SIZE(args->op.ioctl.request);
	bootloader_attr_t attr;
	bootloader_writepage_t wattr;
	static boot_event_flash_t event_args;

	dstr("IOCTL REQ: "); dhex(args->op.ioctl.request); dstr("\n");

	switch(args->op.ioctl.request){
		case I_BOOTLOADER_ERASE:
			dstr("erase\n");
			//the erase takes awhile -- so send the reply a little early
			link_transport_slavewrite(driver, &args->reply, sizeof(args->reply), NULL, NULL);
			//set this to zero so caller doesn't execute the slavewrite again
			args->op.cmd = 0;

			erase_flash(driver);
			is_erased = true;

			event_args.abort = 0;
			event_args.bytes = 0;
			event_args.total = -1;

			dstr("erd\n");
			return;

		case I_BOOTLOADER_GETINFO:
			//write data to io_buf
			dstr("info\n");
			attr.version = BCDVERSION;
			mcu_core_getserialno((mcu_sn_t*)(attr.serialno));

			attr.startaddr = boot_board_config.program_start_addr;
			attr.hardware_id = boot_board_config.id;

			err = link_transport_slavewrite(driver, &attr, size, NULL, NULL);
			if ( err == -1 ){
				args->op.cmd = 0;
				args->reply.err = -1;
			} else {
				args->reply.err = 0;
			}

			break;

		case I_BOOTLOADER_RESET:
			dstr("rst\n");
			if( args->op.ioctl.arg == 0 ){
				boot_event(BOOT_EVENT_RESET, 0);
				boot_link_cmd_reset(driver, args);
			} else {
				boot_event(BOOT_EVENT_RESET_BOOTLOADER, 0);
				boot_link_cmd_reset_bootloader(driver, args);
			}
			break;
		case I_BOOTLOADER_WRITEPAGE:
			err = link_transport_slaveread(driver, &wattr, size, NULL, NULL);
			if( err < 0 ){
				dstr("failed to read data\n");
				break;
			}

			dstr("w:"); dhex(wattr.addr); dstr(":"); dint(wattr.nbyte); dstr("\n");

			args->reply.err = mcu_flash_writepage(FLASH_PORT, (flash_writepage_t*)&wattr);
			if( args->reply.err < 0 ){
				dstr("Failed to write flash:"); dhex(args->reply.err); dstr("\n");
			}

			event_args.increment = wattr.nbyte;
			event_args.bytes += event_args.increment;
			boot_event(BOOT_EVENT_FLASH_WRITE, &event_args);
			break;
		default:
			args->reply.err_number = EINVAL;
			args->reply.err = -1;
			break;
	}

	if ( args->reply.err < 0 ){
		args->reply.err_number = errno;
	}

}

void boot_link_cmd_read(link_transport_driver_t * driver, link_data_t * args){
	args->reply.err = read_flash(driver, args->op.read.addr, args->op.read.nbyte);
	dint(args->reply.err); dstr("\n");
	return;
}

void boot_link_cmd_write(link_transport_driver_t * driver, link_data_t * args){
	args->reply.err = -1;
	return;
}

void erase_flash(link_transport_driver_t * driver){
	int page = 0;
	int result;
	boot_event_flash_t args;
	args.abort = 0;
	args.bytes = -1;
	args.total = -1;
	args.increment = -1;

	while( mcu_flash_erasepage(FLASH_PORT, (void*)page++) != 0 ){
		//these are the bootloader pages and won't be erased
		sos_led_svcall_enable(0);
		driver->wait(1);
		sos_led_svcall_disable(0);
		args.bytes = page;
		boot_event(BOOT_EVENT_FLASH_ERASE, &args);
	}
	page--;

	dstr("erase:"); dint(page); dstr("\n");

	//erase the flash pages -- ends when an erase on an invalid page is attempted
	while( (result = mcu_flash_erasepage(FLASH_PORT, (void*)page++)) == 0 ){
		sos_led_svcall_enable(0);
		driver->wait(1);
		sos_led_svcall_disable(0);
		args.bytes = page;
		boot_event(BOOT_EVENT_FLASH_ERASE, &args);
	}

	dstr("result:"); dint(SYSFS_GET_RETURN_ERRNO(result)); dstr("\n");

	sos_led_svcall_enable(0);
}

void boot_link_cmd_reset(link_transport_driver_t * driver, link_data_t * args){
	sos_led_svcall_disable(0);
	u32 * dfu_sw_req;
	driver->wait(500);
	driver->close(&(driver->handle));
	dfu_sw_req = (u32*)boot_board_config.sw_req_loc;
	*dfu_sw_req = 0;
	cortexm_reset(NULL);
	//the program never arrives here
}

void boot_link_cmd_reset_bootloader(link_transport_driver_t * driver, link_data_t * args){
	u32 * dfu_sw_req;
	driver->wait(500);
	driver->close(&(driver->handle));
	dfu_sw_req = (u32*)boot_board_config.sw_req_loc;
	*dfu_sw_req = boot_board_config.sw_req_value;
	cortexm_reset(NULL);
	//the program never arrives here
}

int read_flash_callback(void * context, void * buf, int nbyte){
	int * loc;
	int ret;
	loc = context;
	ret = mcu_sync_io(&flash_dev, mcu_flash_read, *loc, buf, nbyte, O_RDWR);
	if( ret > 0 ){
		*loc += ret;
	}
	return ret;
}

int read_flash(link_transport_driver_t * driver, int loc, int nbyte){
	return link_transport_slavewrite(driver, NULL, nbyte, read_flash_callback, &loc);
}


/*! @} */


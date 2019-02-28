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

/*! \addtogroup LINK
 * @{
 *
 */

//#include "config.h"

#include <stdbool.h>
#include <sys/fcntl.h> //Defines the flags
#include <errno.h>
#include <dirent.h>
#include <pthread.h>

#include "../process/process_start.h"
#include "../scheduler/scheduler_local.h"

#include "sos/sos.h"
#include "mcu/flash.h"
#include "mcu/debug.h"

#include "sos/link.h"
#include "trace.h"

#define SERIAL_NUM_WIDTH 3

static int read_device(link_transport_driver_t * driver, int fildes, int size);
static int write_device(link_transport_driver_t * driver, int fildes, int size);
static int read_device_callback(void * context, void * buf, int nbyte);
static int write_device_callback(void * context, void * buf, int nbyte);
static void translate_link_stat(struct link_stat * dest, struct stat * src);

typedef struct {
	link_op_t op;
	link_reply_t reply;
} link_data_t;

static void link_cmd_none(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_readserialno(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_ioctl(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_read(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_write(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_open(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_close(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_link(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_unlink(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_lseek(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_stat(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_fstat(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_mkdir(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_rmdir(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_opendir(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_readdir(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_closedir(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_rename(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_chown(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_chmod(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_exec(link_transport_driver_t * driver, link_data_t * args);
static void link_cmd_mkfs(link_transport_driver_t * driver, link_data_t * args);


void (* const link_cmd_func_table[LINK_CMD_TOTAL])(link_transport_driver_t *, link_data_t*) = {
		link_cmd_none,
		link_cmd_readserialno,
		link_cmd_ioctl,
		link_cmd_read,
		link_cmd_write,
		link_cmd_open,
		link_cmd_close,
		link_cmd_link,
		link_cmd_unlink,
		link_cmd_lseek,
		link_cmd_stat,
		link_cmd_fstat,
		link_cmd_mkdir,
		link_cmd_rmdir,
		link_cmd_opendir,
		link_cmd_readdir,
		link_cmd_closedir,
		link_cmd_rename,
		link_cmd_chown,
		link_cmd_chmod,
		link_cmd_exec,
		link_cmd_mkfs
		};


void * link_update(void * arg){
	int err;
	link_transport_driver_t * driver = arg;
	link_data_t data;
	char packet_data[LINK_MAX_TRANSFER_SIZE];
	data.op.cmd = 0;
	err = 0;

	mcu_debug_log_info(MCU_DEBUG_LINK, "Open link driver");
	if( (driver->handle = driver->open(NULL, 0)) == LINK_PHY_ERROR){
		mcu_debug_log_error(MCU_DEBUG_LINK, "failed to init phy");
		return 0;
	}

	mcu_debug_log_info(MCU_DEBUG_LINK, "start link update");
	while(1){

		//Wait for data to arrive on the link transport device
		while( 1 ){

			if ( (err = link_transport_slaveread(driver, &packet_data, LINK_MAX_TRANSFER_SIZE, NULL, NULL)) <= 0 ){
				mcu_debug_log_warning(MCU_DEBUG_LINK, "slave read error %d", err);
				driver->flush(driver->handle);
				continue;
			}

			memcpy(&data.op, packet_data, sizeof(data.op));

			if( err > 0 ){
				break;
			} else {

			}

		}

		if ( data.op.cmd < LINK_CMD_TOTAL ){
			//mcu_debug_printf("cmd:%d\n", data.op.cmd);
			link_cmd_func_table[data.op.cmd](driver, &data);
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

	mcu_debug_user_printf("Link quit\n");
	return NULL;
}

void link_cmd_none(link_transport_driver_t * driver, link_data_t * args){
}

void link_cmd_isbootloader(link_transport_driver_t * driver, link_data_t * args){
	args->reply.err = 0; //this is not the bootloader
}

static void root_get_serialno(void * dest) MCU_ROOT_EXEC_CODE;
void root_get_serialno(void * dest){
	core_info_t info;
	int i, j;
	char * p = dest;
	mcu_core_getinfo(0, &info);
	for(j=SERIAL_NUM_WIDTH; j >= 0; j--){
		for(i=0; i < 8; i++){
			*p++ = htoc((info.serial.sn[j] >> 28) & 0x0F);
			info.serial.sn[j] <<= 4;
		}
	}
}

void link_cmd_readserialno(link_transport_driver_t * driver, link_data_t * args){
	char serialno[LINK_PACKET_DATA_SIZE];
	memset(serialno, 0, LINK_PACKET_DATA_SIZE);
	cortexm_svcall(root_get_serialno, serialno);

	args->reply.err = strlen(serialno);
	args->reply.err_number = 0;

	if( link_transport_slavewrite(driver, &args->reply, sizeof(args->reply), NULL, NULL) < 0 ){
		return;
	}

	if( link_transport_slavewrite(driver, serialno, args->reply.err, NULL, NULL) < 0 ){
		return;
	}

	args->op.cmd = 0; //reply was already sent
}

void link_cmd_open(link_transport_driver_t * driver, link_data_t * args){
	char path[PATH_MAX];
	errno = 0;
	args->reply.err = link_transport_slaveread(driver, path, args->op.open.path_size, NULL, NULL);
	args->reply.err = open(path, args->op.open.flags, args->op.open.mode);
	if ( args->reply.err < 0 ){
		mcu_debug_log_error(MCU_DEBUG_LINK, "Failed to open %s (%d)", path, errno);
		args->reply.err_number = errno;
	}
}

void link_cmd_link(link_transport_driver_t * driver, link_data_t * args){
	char path[PATH_MAX];
	char path_new[PATH_MAX];
	args->reply.err = link_transport_slaveread(driver, path, args->op.symlink.path_size_old, NULL, NULL); //send final ack

	if ( args->reply.err == args->op.symlink.path_size_old ){
		args->reply.err = link_transport_slaveread(driver, path_new, args->op.symlink.path_size_new, NULL, NULL); //don't send final ack
		args->reply.err = link(path, path_new);
		args->reply.err = 0;
		if ( args->reply.err < 0 ){
			mcu_debug_log_error(MCU_DEBUG_LINK, "Failed to link %s (%d)", path, errno);
			args->reply.err_number = errno;
		}
	}
}

void link_cmd_ioctl(link_transport_driver_t * driver, link_data_t * args){
	int err;
	u16 size;
	errno = 0;
	size = _IOCTL_SIZE(args->op.ioctl.request);
	char io_buf[size];
	if ( _IOCTL_IOCTLW(args->op.ioctl.request) != 0 ){ //this means data is being sent over the bulk interrupt
		if( link_transport_slaveread(driver, io_buf, size, NULL, NULL) == -1 ){
			args->op.cmd = 0;
			args->reply.err = -1;
			return;
		}
	}

	if( args->op.ioctl.fildes != driver->handle ){
		if ( _IOCTL_IOCTLRW(args->op.ioctl.request) == 0 ){
			//This means the third argument is just an integer
			args->reply.err = ioctl(args->op.ioctl.fildes, args->op.ioctl.request, args->op.ioctl.arg);
		} else {
			//This means a read or write is happening and the pointer should be passed
			if( io_buf != NULL ){
				args->reply.err = ioctl(args->op.ioctl.fildes, args->op.ioctl.request, io_buf);
			} else {
				args->reply.err = -1;
				args->reply.err_number = ENOMEM;
			}
		}
	} else {
		args->reply.err = -1;
		args->reply.err_number = EBADF;
	}

	//Check to see if this is a read operation and data must be sent back to the host
	if ( _IOCTL_IOCTLR(args->op.ioctl.request) != 0 ){
		//If the ioctl function wrote data to the ctl argument, pass the data over the link
		err = link_transport_slavewrite(driver, io_buf, size, NULL, NULL);
		if ( err == -1 ){
			args->op.cmd = 0;
			args->reply.err = -1;
		}
	}

	if ( args->reply.err < 0 ){
		mcu_debug_log_error(MCU_DEBUG_LINK, "Failed to ioctl (%d)", errno);
		args->reply.err_number = errno;
	}

}

void link_cmd_read(link_transport_driver_t * driver, link_data_t * args){
	if( args->op.read.fildes != driver->handle ){
		errno = 0;
		args->reply.err = read_device(driver, args->op.read.fildes, args->op.read.nbyte);
	}  else {
		args->reply.err = -1;
		args->reply.err_number = EBADF;
	}
	if ( args->reply.err < 0 ){
		mcu_debug_log_error(MCU_DEBUG_LINK, "Failed to read (%d)", errno);
		args->reply.err_number = errno;
	}
}

void link_cmd_write(link_transport_driver_t * driver, link_data_t * args){
	if( args->op.write.fildes != driver->handle ){
		errno = 0;
		args->reply.err = write_device(driver, args->op.write.fildes, args->op.write.nbyte);
	}  else {
		args->reply.err = -1;
		args->reply.err_number = EBADF;
	}
	if ( args->reply.err < 0 ){
		mcu_debug_log_error(MCU_DEBUG_LINK, "Failed to write (%d)", errno);
		args->reply.err_number = errno;
	}
}

void link_cmd_close(link_transport_driver_t * driver, link_data_t * args){
	if( args->op.ioctl.fildes != driver->handle ){
		args->reply.err = close(args->op.close.fildes);
	}  else {
		args->reply.err = -1;
		args->reply.err_number = EBADF;
	}
	if ( args->reply.err < 0 ){
		mcu_debug_log_error(MCU_DEBUG_LINK, "Failed to close (%d)", errno);
		args->reply.err_number = errno;
	}
}

void link_cmd_unlink(link_transport_driver_t * driver, link_data_t * args){
	char path[PATH_MAX];
	if( link_transport_slaveread(driver, path, args->op.unlink.path_size, NULL, NULL) < 0 ){
		return;
	}
	args->reply.err = unlink(path);
	if ( args->reply.err < 0 ){
		mcu_debug_log_error(MCU_DEBUG_LINK, "Failed to unlink (%d)", errno);
		args->reply.err_number = errno;
	}

}

void link_cmd_lseek(link_transport_driver_t * driver, link_data_t * args){
	args->reply.err = lseek(args->op.lseek.fildes, args->op.lseek.offset, args->op.lseek.whence);
	if ( args->reply.err < 0 ){
		args->reply.err_number = errno;
	}
}

void link_cmd_stat(link_transport_driver_t * driver, link_data_t * args){
	char path[PATH_MAX];
	int err;
	struct stat st;
	struct link_stat lst;

	link_transport_slaveread(driver, path, args->op.stat.path_size, NULL, NULL);

	args->reply.err = stat(path, &st);
	if (args->reply.err < 0 ){
		mcu_debug_log_error(MCU_DEBUG_LINK, "Failed to stat (%d)", errno);
		args->reply.err_number = errno;
	}

	translate_link_stat(&lst, &st);
	args->op.cmd = 0;


	//Send the reply
	link_transport_slavewrite(driver, &args->reply, sizeof(link_reply_t), NULL, NULL);
	if ( args->reply.err < 0 ){
		return;
	}

	//now send the data
	err = link_transport_slavewrite(driver, &lst, sizeof(struct link_stat), NULL, NULL);
	if ( err < -1 ){
		return;
	}

}

void link_cmd_fstat(link_transport_driver_t * driver, link_data_t * args){
	int err;
	struct stat st;
	struct link_stat lst;
	args->reply.err = fstat(args->op.fstat.fildes, (struct stat*)&st);
	if ( args->reply.err < 0 ){
		args->reply.err_number = errno;
	}

	translate_link_stat(&lst, &st);
	args->op.cmd = 0;

	//Send the reply
	link_transport_slavewrite(driver, &args->reply, sizeof(link_reply_t), NULL, NULL);
	if ( args->reply.err < -1 ){
		return;
	}


	//now send the data
	err = link_transport_slavewrite(driver, &lst, sizeof(struct link_stat), NULL, NULL);
	if ( err < -1 ){
		return;
	}

}

void link_cmd_mkdir(link_transport_driver_t * driver, link_data_t * args){
	char path[PATH_MAX];
	link_transport_slaveread(driver, path, args->op.mkdir.path_size, NULL, NULL);
	args->reply.err = mkdir(path, args->op.mkdir.mode);
	if ( args->reply.err < 0 ){
		args->reply.err_number = errno;
	}

}

void link_cmd_rmdir(link_transport_driver_t * driver, link_data_t * args){
	char path[PATH_MAX];
	link_transport_slaveread(driver, path, args->op.rmdir.path_size, NULL, NULL);
	args->reply.err = rmdir(path);
	if ( args->reply.err < 0 ){
		args->reply.err_number = errno;
	}

}

void link_cmd_opendir(link_transport_driver_t * driver, link_data_t * args){
	char path[PATH_MAX];
	link_transport_slaveread(driver, path, args->op.opendir.path_size, NULL, NULL);
	args->reply.err = (int)opendir(path);
	if ( args->reply.err  == 0 ){
		mcu_debug_log_error(MCU_DEBUG_LINK, "Failed to open dir %s (%d)", path, errno);
		args->reply.err_number = errno;
	}
}

void link_cmd_readdir(link_transport_driver_t * driver, link_data_t * args){
	struct dirent de;
	struct link_dirent lde;

	args->reply.err = readdir_r((DIR*)args->op.readdir.dirp, &de, NULL);

	if ( args->reply.err < 0 ){
		args->reply.err = -1;
		args->reply.err_number = errno;
		if ( errno != 2 ){
			mcu_debug_log_error(MCU_DEBUG_LINK, "Failed to read dir (%d)", errno);
		}
	}

	args->op.cmd = 0;

	lde.d_ino = de.d_ino;
	strcpy(lde.d_name, de.d_name);

	if( link_transport_slavewrite(driver, &args->reply, sizeof(link_reply_t), NULL, NULL) < 0 ){
		return;
	}

	if( args->reply.err < 0 ){
		return;
	}

	link_transport_slavewrite(driver, &lde, sizeof(struct link_dirent), NULL, NULL);
}

void link_cmd_closedir(link_transport_driver_t * driver, link_data_t * args){
	args->reply.err = closedir((DIR*)args->op.closedir.dirp);
	if ( args->reply.err < 0 ){
		args->reply.err_number = errno;
	}
}


void link_cmd_rename(link_transport_driver_t * driver, link_data_t * args){
	char path_new[PATH_MAX];
	char path[PATH_MAX];
	args->reply.err = 0;
	if( link_transport_slaveread(driver, path, args->op.rename.old_size, NULL, NULL) != args->op.rename.old_size){
		args->reply.err = -1;
	}
	if( link_transport_slaveread(driver, path_new, args->op.rename.new_size, NULL, NULL) != args->op.rename.new_size){
		args->reply.err = -1;
	}
	if( args->reply.err < 0 ){
		return;
	}
	args->reply.err = rename(path, path_new);
	if ( args->reply.err < 0 ){
		args->reply.err_number = errno;
	}

}

void link_cmd_chown(link_transport_driver_t * driver, link_data_t * args){
	char path[PATH_MAX];
	if( link_transport_slaveread(driver, path, args->op.chown.path_size, NULL, NULL) != args->op.chown.path_size){
		args->reply.err = -1;
	}
	if( args->reply.err < 0 ){
		return;
	}
	args->reply.err = chown(path, args->op.chown.uid, args->op.chown.gid);
	if ( args->reply.err < 0 ){
		args->reply.err_number = errno;
	}

}

void link_cmd_chmod(link_transport_driver_t * driver, link_data_t * args){
	char path[PATH_MAX];
	if( link_transport_slaveread(driver, path, args->op.chmod.path_size, NULL, NULL) != args->op.chmod.path_size){
		args->reply.err = -1;
	}
	if( args->reply.err < 0 ){
		return;
	}
	args->reply.err = chmod(path, args->op.chmod.mode);
	if ( args->reply.err < 0 ){
		args->reply.err_number = errno;
	}
}


void link_cmd_exec(link_transport_driver_t * driver, link_data_t * args){
	char path[LINK_PATH_ARG_MAX];
	memset(path, 0, LINK_PATH_ARG_MAX);
	args->reply.err = link_transport_slaveread(driver, path, args->op.exec.path_size, NULL, NULL);
	args->reply.err = process_start(path, NULL, 0);
	if ( args->reply.err < 0 ){
		mcu_debug_log_error(MCU_DEBUG_LINK, "Failed to exec %s (%d)", path, errno);
		args->reply.err_number = errno;
	}
}

void link_cmd_mkfs(link_transport_driver_t * driver, link_data_t * args){
	char path[PATH_MAX];
	memset(path, 0, PATH_MAX);
	args->reply.err = link_transport_slaveread(driver, path, args->op.exec.path_size, NULL, NULL);
	args->reply.err = mkfs(path);
	if ( args->reply.err < 0 ){
		mcu_debug_log_error(MCU_DEBUG_LINK, "Failed to exec %s (%d)", path, errno);
		args->reply.err_number = errno;
	}
}

int read_device_callback(void * context, void * buf, int nbyte){
	int * fildes;
	int ret;
	fildes = context;
	ret = read(*fildes, buf, nbyte);
	return ret;
}

int write_device_callback(void * context, void * buf, int nbyte){
	int * fildes;
	int ret;
	fildes = context;
	ret =  write(*fildes, buf, nbyte);
	return ret;
}

int read_device(link_transport_driver_t * driver, int fildes, int nbyte){
	return link_transport_slavewrite(driver, NULL, nbyte, read_device_callback, &fildes);
}

int write_device(link_transport_driver_t * driver, int fildes, int nbyte){
	return link_transport_slaveread(driver, NULL, nbyte, write_device_callback, &fildes);
}

void translate_link_stat(struct link_stat * dest, struct stat * src){
	dest->st_dev = src->st_dev;
	dest->st_ino = src->st_ino;
	dest->st_mode = src->st_mode;
	dest->st_uid = src->st_uid;
	dest->st_gid = src->st_gid;
	dest->st_rdev = src->st_rdev;
	dest->st_size = src->st_size;
	dest->st_mtime_ = src->st_mtime;
	dest->st_ctime_ = src->st_ctime;
	dest->st_blksize = src->st_blksize;
	dest->st_blocks = src->st_blocks;
}



/*! @} */


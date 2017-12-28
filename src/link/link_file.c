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

#include <string.h>
#include <stdarg.h>

#include "link_local.h"

int link_open(link_transport_mdriver_t * driver, const char * path, int flags, ...){
	link_op_t op;
	link_reply_t reply;
	link_mode_t mode;
	int err;
	va_list ap;


	if ( flags & LINK_O_CREAT ){
		va_start(ap, flags);
		mode = va_arg(ap, link_mode_t);
		va_end(ap);
	} else {
		mode = 0;
	}

	link_debug(LINK_DEBUG_MESSAGE, "open %s 0%o 0x%X using 0x%llX", path, mode, flags, (u64)driver->dev.handle);


	op.open.cmd = LINK_CMD_OPEN;
	op.open.path_size = strlen(path) + 1;
	op.open.flags = flags;
	op.open.mode = mode;

	link_debug(LINK_DEBUG_MESSAGE, "Write open op (0x%lX)", (long unsigned int)driver->dev.handle);
	err = link_transport_masterwrite(driver, &op, sizeof(link_open_t));
	if ( err < 0 ){
		link_error("failed to write open op with handle %llX", (u64)driver->dev.handle);
		return link_handle_err(driver, err);
	}

	//Send the path on the bulk out endpoint
	link_debug(LINK_DEBUG_MESSAGE, "Write open path (%d bytes)", op.open.path_size);
	err = link_transport_masterwrite(driver, path, op.open.path_size);
	if ( err < 0 ){
		link_error("failed to write path");
		return link_handle_err(driver, err);
	}



	//read the reply to see if the file opened correctly
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		link_error("failed to read the reply");
		return link_handle_err(driver, err);
	}

	if ( reply.err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to ioctl file (%d)", link_errno);
	}
	return reply.err;
}


int link_ioctl(link_transport_mdriver_t * driver, int fildes, int request, ...){
	void * argp;
	int arg;
	va_list ap;
	if ( _IOCTL_IOCTLRW(request) ){
		va_start(ap, request);
		argp = va_arg(ap, void*);
		va_end(ap);
		arg = 0;
	} else {
		va_start(ap, request);
		arg = va_arg(ap, int);
		va_end(ap);
		argp = NULL;
	}

	return link_ioctl_delay(driver, fildes, request, argp, arg, 0);
}


int link_ioctl_delay(link_transport_mdriver_t * driver, int fildes, int request, void * argp, int arg, int delay){
	int rw_size;
	link_op_t op;
	link_reply_t reply;

	int err;

	link_debug(LINK_DEBUG_MESSAGE, "Request is 0x%X", (unsigned int)request);

	rw_size = _IOCTL_SIZE(request);

	link_debug(LINK_DEBUG_MESSAGE, "Sending IOCTL request %c %d r:%X w:%X", _IOCTL_IDENT(request),
			_IOCTL_SIZE(request),
			_IOCTL_IOCTLR(request) != 0,
			_IOCTL_IOCTLW(request) != 0);

	//execute the request
	op.ioctl.fildes = fildes;
	op.ioctl.cmd = LINK_CMD_IOCTL;
	op.ioctl.request = request;
	op.ioctl.arg = arg;
	err = link_transport_masterwrite(driver, &op, sizeof(link_ioctl_t));
	if ( err < 0 ){
		link_error("failed to write op");
		return link_handle_err(driver, err);
	}

	if( _IOCTL_IOCTLW(request) ){
		//need to write data to the bulk endpoint (argp)
		link_debug(LINK_DEBUG_MESSAGE, "Sending IOW data");
		err = link_transport_masterwrite(driver, argp, rw_size);
		if ( err < 0 ){
			link_error("failed to write IOW data");
			return link_handle_err(driver, err);
		}
	}

	if( delay > 0 ){
		link_debug(LINK_DEBUG_MESSAGE, "Delay for %dms", delay);
		driver->dev.wait(delay);
	}

	if( _IOCTL_IOCTLR(request) ){
		//need to read data from the bulk endpoint
		link_debug(LINK_DEBUG_MESSAGE, "Getting IOR data %d bytes", rw_size);
		err = link_transport_masterread(driver, argp, rw_size);
		link_debug(LINK_DEBUG_MESSAGE, "Getting IOR data done (%d)", err);

		if ( err < 0 ){
			link_error("failed to read IOR data");
			return link_handle_err(driver, err);
		}

		if( err != rw_size ){
			if (err == sizeof(reply) ){
				link_errno = reply.err_number;
				link_error("failed to read IOR data -- bad size %d", link_errno);
				memcpy(&reply, argp, sizeof(reply));
				return reply.err;
			}

			return link_handle_err(driver, LINK_PROT_ERROR);
		}

	}

	//Get the reply
	link_debug(LINK_DEBUG_MESSAGE, "Read reply");
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		link_error("failed to read reply");
		return link_handle_err(driver, err);
	}

	link_errno = reply.err_number;
	link_debug(LINK_DEBUG_MESSAGE, "Replied with %d (%d)", reply.err, link_errno);
	if ( reply.err < 0 ){
		link_debug(LINK_DEBUG_WARNING, "Failed to ioctl file (%d)", link_errno);
	}
	return reply.err;
}


int link_read(link_transport_mdriver_t * driver, int fildes, void * buf, int nbyte){
	link_op_t op;
	link_reply_t reply;
	int err;

	op.read.cmd = LINK_CMD_READ;
	op.read.fildes = fildes;
	op.read.nbyte = nbyte;


	link_debug(LINK_DEBUG_MESSAGE, "write read op");
	err = link_transport_masterwrite(driver, &op, sizeof(link_read_t));
	if ( err < 0 ){
		link_error("failed to write op");
		return link_handle_err(driver, err);
	}

	link_debug(LINK_DEBUG_MESSAGE, "read data from the file %d", nbyte);
	err = link_transport_masterread(driver, buf, nbyte);
	if ( err < 0 ){
		link_error("failed to read data");
		return link_handle_err(driver, err);
	}

	link_debug(LINK_DEBUG_MESSAGE, "read %d of %d bytes", err, nbyte);
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		link_error("failed to read reply");
		return link_handle_err(driver, err);
	}

	if ( reply.err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to read file (%d)", link_errno);
	}

	return reply.err;
}

int link_write(link_transport_mdriver_t * driver, int fildes, const void * buf, int nbyte){
	link_op_t op;
	link_reply_t reply;
	int err;
	if ( driver == NULL ){
		return LINK_TRANSFER_ERR;
	}

	op.write.cmd = LINK_CMD_WRITE;
	op.write.fildes = fildes;
	op.write.nbyte = nbyte;

	link_debug(LINK_DEBUG_MESSAGE, "Send op (fildes %d nbyte %d)", op.write.fildes, op.write.nbyte);
	err = link_transport_masterwrite(driver, &op, sizeof(link_write_t));
	if ( err < 0 ){
		link_error("failed to write op");
		return link_handle_err(driver, err);
	}

	link_debug(LINK_DEBUG_MESSAGE, "Write data");
	err = link_transport_masterwrite(driver, buf, nbyte);
	if ( err < 0 ){
		link_error("failed to write data");
		return link_handle_err(driver, err);
	}

	//read the reply to see if the file wrote correctly
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		link_error("failed to read reply");
		return link_handle_err(driver, err);
	}

	if ( reply.err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to write file (%d)", link_errno);
	}
	return reply.err;
}

int link_close(link_transport_mdriver_t * driver, int fildes){
	link_op_t op;
	link_reply_t reply;
	int err;
	op.close.cmd = LINK_CMD_CLOSE;
	op.close.fildes = fildes;
	if ( driver == NULL ){
		return LINK_TRANSFER_ERR;
	}
	link_debug(LINK_DEBUG_MESSAGE, "Send Op to close fildes:%d", fildes);
	err = link_transport_masterwrite(driver, &op, sizeof(link_close_t));
	if ( err < 0 ){
		link_error("failed to write op");
		return link_handle_err(driver, err);
	}

	link_debug(LINK_DEBUG_MESSAGE, "Read Reply");
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		link_error("failed to read reply");
		return link_handle_err(driver, err);
	}

	if ( reply.err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to close file (%d)", link_errno);
	}

	link_debug(LINK_DEBUG_MESSAGE, "close complete %d", reply.err);

	return reply.err;
}

int link_symlink(link_transport_mdriver_t * driver, const char * old_path, const char * new_path){
	link_op_t op;
	link_reply_t reply;
	int len;
	int err;
	if ( driver == NULL ){
		return LINK_TRANSFER_ERR;
	}
	op.symlink.cmd = LINK_CMD_LINK;
	op.symlink.path_size_old = strlen(old_path) + 1;
	op.symlink.path_size_new = strlen(new_path) + 1;

	link_debug(LINK_DEBUG_MESSAGE, "Write link op (0x%lX)", (long unsigned int)driver->dev.handle);
	err = link_transport_masterwrite(driver, &op, sizeof(link_symlink_t));
	if ( err < 0 ){
		return err;
	}

	//Send the path on the bulk out endpoint
	link_debug(LINK_DEBUG_MESSAGE, "Write open path (%d bytes)", op.symlink.path_size_old);
	len = link_transport_masterwrite(driver, old_path, op.symlink.path_size_old);
	if (len != op.symlink.path_size_old ){
		return -1;
	}

	len = link_transport_masterwrite(driver, new_path, op.symlink.path_size_new);
	if ( len < 0 ){
		return LINK_TRANSFER_ERR;
	}

	//read the reply to see if the file opened correctly
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		return err;
	}

	if ( reply.err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to symlink (%d)", link_errno);
	}
	return reply.err;
}

int link_unlink(link_transport_mdriver_t * driver, const char * path){
	link_op_t op;
	link_reply_t reply;
	int len;
	int err;
	if ( driver == NULL ){
		return LINK_TRANSFER_ERR;
	}

	link_debug(LINK_DEBUG_MESSAGE, "unlink %s", path);

	op.unlink.cmd = LINK_CMD_UNLINK;
	op.unlink.path_size = strlen(path) + 1;

	err = link_transport_masterwrite(driver, &op, sizeof(link_unlink_t));
	if ( err < 0 ){
		return err;
	}

	//Send the path on the bulk out endpoint
	len = link_transport_masterwrite(driver, path, op.unlink.path_size);
	if ( len < 0 ){
		return LINK_TRANSFER_ERR;
	}
	//read the reply to see if the file opened correctly
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		return err;
	}

	if ( reply.err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to unlink file (%d)", link_errno);
	}
	return reply.err;
}


int link_lseek(link_transport_mdriver_t * driver, int fildes, s32 offset, int whence){
	link_op_t op;
	link_reply_t reply;
	int err;

	if ( driver == NULL ){
		return LINK_TRANSFER_ERR;
	}
	op.lseek.cmd = LINK_CMD_LSEEK;
	op.lseek.fildes = fildes;
	op.lseek.offset = offset;
	op.lseek.whence = whence;

	link_errno = 0;
	link_debug(LINK_DEBUG_MESSAGE, "seek command 0x%X (0x%lX)", fildes, (long unsigned int)driver->dev.handle);

	err = link_transport_masterwrite(driver, &op, sizeof(link_lseek_t));
	if ( err < 0 ){
		return err;
	}

	link_debug(LINK_DEBUG_MESSAGE, "wait for reply");
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		return err;
	}

	link_debug(LINK_DEBUG_MESSAGE, "device returned %d %d", reply.err, reply.err_number);
	if ( reply.err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to lseek file (%d)", link_errno);
	}
	return reply.err;
}

int link_stat(link_transport_mdriver_t * driver, const char * path, struct link_stat * buf){
	link_op_t op;
	link_reply_t reply;
	int len;
	int err;

	if ( driver == NULL ){
		return LINK_TRANSFER_ERR;
	}

	link_debug(LINK_DEBUG_MESSAGE, "stat %s", path);

	op.stat.cmd = LINK_CMD_STAT;
	op.stat.path_size = strlen(path) + 1;

	link_debug(LINK_DEBUG_MESSAGE, "send op %d path size %d", op.stat.cmd, op.stat.path_size);
	err = link_transport_masterwrite(driver, &op, sizeof(link_stat_t));
	if ( err < 0 ){
		return err;
	}

	//Send the path on the bulk out endpoint
	link_debug(LINK_DEBUG_MESSAGE, "write stat path");
	len = link_transport_masterwrite(driver, path, op.stat.path_size);
	if ( len < 0 ){
		return LINK_TRANSFER_ERR;
	}

	//Get the reply
	link_debug(LINK_DEBUG_MESSAGE, "read stat reply");
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		link_error("failed to read reply");
		return err;
	}

	link_debug(LINK_DEBUG_MESSAGE, "stat reply %d", reply.err);
	if ( reply.err == 0 ){
		//Read bulk in as the size of the the new data
		link_debug(LINK_DEBUG_MESSAGE, "read stat data");
		err = link_transport_masterread(driver, buf, sizeof(struct link_stat));
		if ( err < 0 ){
			return err;
		}
	} else {
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to stat file (%d)", link_errno);
	}

	return reply.err;

}

int link_fstat(link_transport_mdriver_t * driver, int fildes, struct link_stat * buf){
	link_op_t op;
	link_reply_t reply;
	int err;

	if ( driver == NULL ){
		return LINK_TRANSFER_ERR;
	}

	op.fstat.cmd = LINK_CMD_FSTAT;
	op.fstat.fildes = fildes;

	err = link_transport_masterwrite(driver, &op, sizeof(link_fstat_t));
	if ( err < 0 ){
		return err;
	}

	//Get the reply
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		return err;
	}

	if ( reply.err == 0 ){
		//Read bulk in as the size of the the new data
		err = link_transport_masterread(driver, buf, sizeof(struct link_stat));
		if ( err < 0 ){
			return err;
		}

	} else {
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to fstat file (%d)", link_errno);
	}

	return reply.err;
}

int link_rename(link_transport_mdriver_t * driver, const char * old_path, const char * new_path){
	link_op_t op;
	link_reply_t reply;
	int len;
	int err;
	if ( driver == NULL ){
		return LINK_TRANSFER_ERR;
	}

	link_debug(LINK_DEBUG_MESSAGE, "rename %s to %s", old_path, new_path);

	op.rename.cmd = LINK_CMD_RENAME;
	op.rename.old_size = strlen(old_path) + 1;
	op.rename.new_size = strlen(new_path) + 1;

	err = link_transport_masterwrite(driver, &op, sizeof(link_rename_t));
	if ( err < 0 ){
		return err;
	}

	//Send the old path on the bulk out endpoint
	len = link_transport_masterwrite(driver, old_path, op.rename.old_size);
	if ( len < 0 ){
		return LINK_TRANSFER_ERR;
	}

	//Send the new path on the bulk out endpoint
	len = link_transport_masterwrite(driver, new_path, op.rename.new_size);
	if ( len < 0 ){
		return LINK_TRANSFER_ERR;
	}

	//read the reply to see if the file opened correctly
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		return err;
	}

	if ( reply.err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to rename file (%d)", link_errno);
	}
	return reply.err;
}

int link_chown(link_transport_mdriver_t * driver, const char * path, int owner, int group){
	link_op_t op;
	link_reply_t reply;
	int len;
	int err;

	if ( driver == NULL ){
		return LINK_TRANSFER_ERR;
	}

	link_debug(LINK_DEBUG_MESSAGE, "chown %s %d %d", path, owner, group);

	op.chown.cmd = LINK_CMD_CHOWN;
	op.chown.path_size = strlen(path) + 1;
	op.chown.uid = owner;
	op.chown.gid = group;

	link_debug(LINK_DEBUG_MESSAGE, "Write op");
	err = link_transport_masterwrite(driver, &op, sizeof(op));
	if ( err < 0 ){
		return err;
	}

	//Send the path on the bulk out endpoint
	link_debug(LINK_DEBUG_MESSAGE, "Write path");
	len = link_transport_masterwrite(driver, path, op.chown.path_size);
	if ( len < 0 ){
		return LINK_TRANSFER_ERR;
	}

	//read the reply to see if the file opened correctly
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to chown (%d)", link_errno);
	}

	return reply.err;
}

int link_chmod(link_transport_mdriver_t * driver, const char * path, int mode){
	link_op_t op;
	link_reply_t reply;
	int len;
	int err;

	if ( driver == NULL ){
		return -1;
	}

	link_debug(LINK_DEBUG_MESSAGE, "chmod %s %d", path, mode);

	op.chmod.cmd = LINK_CMD_CHMOD;
	op.chmod.path_size = strlen(path) + 1;
	op.chmod.mode = mode;

	link_debug(LINK_DEBUG_MESSAGE, "Write op");
	err = link_transport_masterwrite(driver, &op, sizeof(op));
	if ( err < 0 ){
		return err;
	}

	//Send the path on the bulk out endpoint
	link_debug(LINK_DEBUG_MESSAGE, "Write path");
	len = link_transport_masterwrite(driver, path, op.chown.path_size);
	if ( len < 0 ){
		return LINK_TRANSFER_ERR;
	}

	//read the reply to see if the file opened correctly
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to chmod (%d)", link_errno);
	}

	return reply.err;
}




/*! @} */


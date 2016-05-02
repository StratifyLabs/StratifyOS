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
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "link_flags.h"


int link_exec(link_phy_t handle, const char * file){
	link_op_t op;
	link_reply_t reply;
	int len;
	int err;

	link_debug(LINK_DEBUG_MESSAGE, "exec %s", file);

	op.exec.cmd = LINK_CMD_EXEC;
	op.exec.path_size = strlen(file) + 1;


	link_debug(LINK_DEBUG_MESSAGE, "Write op (0x%lX)", (long unsigned int)link_dev);
	err = link_transport_masterwrite(handle, &op, sizeof(link_open_t));
	if ( err < 0 ){
		return err;
	}

	//Send the path on the bulk out endpoint
	link_debug(LINK_DEBUG_MESSAGE, "Write exec path (%d bytes)", op.exec.path_size);
	len = link_transport_masterwrite(handle, file, op.exec.path_size);
	if ( len < 0 ){
		link_debug(1, "Failed to write bulk output");
		return LINK_TRANSFER_ERR;
	}

	//read the reply to see if the file opened correctly
	err = link_transport_masterread(handle, &reply, sizeof(reply));
	if ( err < 0 ){
		link_error("Failed to read the reply");
		return err;
	}

	if( reply.err < 0 ){
		link_errno = reply.err_number;
		link_debug(LINK_DEBUG_WARNING, "Failed to exec (%d)", link_errno);
	}

	return reply.err;

}

int link_kill_pid(link_phy_t handle, int pid, int signo){
	int fd;
	int err_ioctl;
	int err;
	sys_killattr_t killattr;

	fd = link_open(handle, "/dev/sys", LINK_O_RDWR);
	if ( fd < 0 ){
		link_error("failed to open /dev/sys");
		return link_handle_err(handle, fd);
	}

	killattr.id = pid;
	killattr.si_signo = signo;
	killattr.si_sigcode = LINK_SI_USER;
	killattr.si_sigvalue = 0;

	err_ioctl = link_ioctl(handle, fd, I_SYS_KILL, &killattr);
	if( err_ioctl == LINK_PHY_ERROR ){
		link_error("failed to I_SYS_KILL");
		return err_ioctl;
	}

	if( err_ioctl == LINK_PROT_ERROR ){
		if( link_handle_err(handle, err_ioctl) == LINK_PHY_ERROR ){
			return LINK_PHY_ERROR;
		}
	}

	if( (err = link_close(handle, fd)) < 0 ){
		link_error("failed to close fd");
		return err;
	}

	return err_ioctl;
}







/*! @} */

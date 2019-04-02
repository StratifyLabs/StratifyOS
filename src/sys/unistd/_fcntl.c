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

/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include <fcntl.h>
#include <stdarg.h>
#include "unistd_local.h"
#include "unistd_fs.h"
#include "sos/sos.h"

#include "mcu/debug.h"

/*! \details This function performs various operations on open files such as:
 * - F_DUPFD: duplicate a file descriptor
 * - F_GETFD:  get the file descriptor flags
 * - F_SETFD:  set the file descriptor flags
 * - F_GETOWN: get the file descriptor owner process ID.
 *
 * \param fildes The file descriptor
 * \param cmd The operation to perform
 *
 * \return Zero on success or -1 on error with errno (see \ref errno) set to:
 *  - EBADF:  invalid file descriptor
 *  - ENOTSUP:  \a cmd is not supported for the file descriptor
 *
 */
int fcntl(int fildes, int cmd, ...);

/*! \cond */
int _fcntl(int fildes, int cmd, ...){
	int tmp;
	int flags;
	va_list ap;

	va_start(ap, cmd);
	tmp = va_arg(ap, int);
	va_end(ap);

	if( FILDES_IS_SOCKET(fildes) ){
		  if( sos_board_config.socket_api != 0 ){
            return sos_board_config.socket_api->fcntl(fildes & ~FILDES_SOCKET_FLAG, cmd, tmp);
        }
        errno = EBADF;
        return -1;
    }

	fildes = u_fildes_is_bad(fildes);
	if ( fildes < 0 ){
		errno = EBADF;
		return -1;
	}

	flags = get_flags(fildes);

	switch(cmd){
	/*
	case F_DUPFD:

		new_fildes = u_new_open_file(tmp);
		if ( new_fildes == -1 ){
			return -1;
		}

		dup_open_file(new_fildes, fildes);
		return new_fildes;
		*/

	case F_GETFL:
		return flags;

	case F_SETFL:
		set_flags(fildes, tmp);
		break;

	case F_GETFD:
	case F_SETFD:
	case F_DUPFD:
	case F_GETOWN:
	case F_SETOWN:
	case F_GETLK:
	case F_SETLK:
	case F_SETLKW:
		errno = ENOTSUP;
		break;
	default:
		errno = EINVAL;
		break;
	}

	return -1;
}
/*! \endcond */

/*! @} */

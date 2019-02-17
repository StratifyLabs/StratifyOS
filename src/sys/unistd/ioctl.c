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

#include "config.h"

#include "sos/ioctl.h"
#include "mcu/mcu.h"
#include <errno.h>
#include <stdarg.h>
#include "unistd_local.h"
#include "sos/sos.h"
#include "mcu/core.h"
#include "device/sys.h"


/*! \details This function performs a control request on the device
 * associated with \a fildes. \a request is specific to the device.
 * The value of \a request determines what value should be passed
 * as the \a ctl argument.
 *
 * \param fildes The file descriptor returned by \ref open()
 * \param request The request to the device.
 *
 * \return The number of bytes actually read of -1 with errno (see \ref errno) set to:
 * - EBADF:  \a fildes is bad
 * - EIO:  IO error
 * - EAGAIN:  O_NONBLOCK is set for \a fildes and the device is busy
 *
 */
int ioctl(int fildes, int request, ...) {
	void * ctl;
	va_list ap;
	va_start(ap, request);
	ctl = va_arg(ap, void*);
	va_end(ap);

	if( FILDES_IS_SOCKET(fildes) ){
		if( sos_board_config.socket_api != 0 ){
			mcu_debug_printf("socket API IOCTL 0x%X 0x%X %p (%d)\n", fildes, request, ctl, fildes);
			return sos_board_config.socket_api->ioctl(fildes & ~FILDES_SOCKET_FLAG, request, ctl);
		}
		errno = EBADF;
		return -1;
	}

	fildes = u_fildes_is_bad(fildes);
	if ( fildes < 0 ){
		//check to see if fildes is a socket
		errno = EBADF;
		return -1;
	}

	return sysfs_file_ioctl(get_open_file(fildes), request, ctl);
}


/*! @} */

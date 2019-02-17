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

/*! \file
 */

#include "sos/fs/sysfs.h"
#include "mcu/mcu.h"
#include "unistd_fs.h"
#include "mcu/debug.h"
#include "sos/sos.h"
#include "unistd_local.h"

/*! \details This function closes the file associated
 * with the specified descriptor.
 * \param fildes The File descriptor \a fildes.
 * \return Zero on success or -1 on error with errno (see \ref errno) set to:
 *  - EBADF:  Invalid file descriptor
 */
int close(int fildes);

/*! \cond */
int _close(int fildes) {
	//Close the file if it's open
	int ret;

	 if( FILDES_IS_SOCKET(fildes) ){
        if( sos_board_config.socket_api != 0 ){
            return sos_board_config.socket_api->close(fildes & ~FILDES_SOCKET_FLAG);
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

	ret = sysfs_file_close(get_open_file(fildes));

	u_reset_fildes(fildes);
	return ret;
}
/*! \endcond */



/*! @} */


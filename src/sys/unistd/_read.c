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

#include "unistd_local.h"
#include  "unistd_fs.h"
#include "sos/sos.h"

/*! \details This function reads \a nbyte bytes from \a fildes to the memory
 * location pointed to by \a buf.
 *
 * read() is always a synchronous call which is either blocking or non-blocking
 * depending on the value of O_NONBLOCK for \a fildes.
 *
 * \param fildes The file descriptor returned by \ref open()
 * \param buf A pointer to the destination memory (process must have write access)
 * \param nbyte The number of bytes to read
 *
 * \return The number of bytes actually read of -1 with errno (see \ref errno) set to:
 * - EBADF:  \a fildes is bad
 * - EACCESS:  \a fildes is on in O_WRONLY mode
 * - EIO:  IO error
 * - EAGAIN:  O_NONBLOCK is set for \a fildes and no new data is available
 *
 *
 */
int read(int fildes, void *buf, size_t nbyte);

/*! \cond */
int _read(int fildes, void *buf, size_t nbyte){
	sysfs_file_t * file;

	if( FILDES_IS_SOCKET(fildes) ){
		  if( sos_board_config.socket_api != 0 ){
            return sos_board_config.socket_api->read(fildes & ~FILDES_SOCKET_FLAG, buf, nbyte);
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

	if ( (get_flags(fildes) & O_ACCMODE) == O_WRONLY ){
		errno = EACCES;
		return -1;
	}

	file = get_open_file(fildes);

	//initialize the file offset location
	return sysfs_file_read(file, buf, nbyte);

}
/*! \endcond */


/*! @} */

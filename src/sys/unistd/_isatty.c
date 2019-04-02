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
#include "unistd_fs.h"

/*! \details This function checks to see if \a fildes is associated
 * with a terminal device.
 *
 * \return Zero on success or -1 on error with errno (see \ref errno) set to:
 *  - ENOTTY:  \a fildes is not associated with a terminal device
 *  - EBADF:  \a fildes is invalid
 *
 */
int isatty(int fildes);

/*! \cond */
int _isatty(int fildes){

	fildes = u_fildes_is_bad(fildes);
	if ( fildes < 0 ){
		return -1;
	}

	return 0;
}
/*! \endcond */

/*! @} */


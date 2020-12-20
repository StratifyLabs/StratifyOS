// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md



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


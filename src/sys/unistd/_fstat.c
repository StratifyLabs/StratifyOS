// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md



/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include "unistd_local.h"
#include  "unistd_fs.h"


/*! \details This function gets various file statistics for
 * the specified file descriptor.
 *
 * \return Zero on success or -1 on error with errno (see \ref errno) set to:
 *  - EBADF:  \a fildes is invalid
 *  - EINVAL: \a buf is NULL
 *
 */
int fstat(int fildes, struct stat *buf);

/*! \cond */
int _fstat(int fildes, struct stat *buf){
	void * handle;
	const sysfs_t * fs;

	fildes = u_fildes_is_bad(fildes);
	if ( fildes < 0 ){
		return -1;
	}

	handle = get_handle(fildes);
	memset (buf, 0, sizeof (* buf));
	fs = get_fs(fildes);
	if ( fs != NULL ){
        int ret = fs->fstat(fs->config, handle, buf);
        SYSFS_PROCESS_RETURN(ret);
        return ret;
	} else {
		errno = EBADF;
		return -1;
	}
}
/*! \endcond */

/*! @} */


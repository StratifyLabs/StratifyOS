// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include  "unistd_fs.h"

/*! \details This function gets various file statistics for
 * a given file name.
 *
 * \return Zero on success or -1 on error with errno (see \ref errno) set to:
 * - ENAMETOOLONG: \a path exceeds PATH_MAX or a component of \a path exceeds NAME_MAX
 * - ENOENT: \a path does not exist
 * - EACCES: search permission is denied for a component of \a path
 *
 */
int stat(const char * path, struct stat *buf);

/*! \cond */
int _stat(const char * path, struct stat *buf){
	const sysfs_t * fs;

	if ( sysfs_ispathinvalid(path) == true ){
		return -1;
	}

	fs = sysfs_find(path, true);
	if ( fs != NULL ){
        int ret = fs->stat(fs->config, sysfs_stripmountpath(fs, path), buf);
        SYSFS_PROCESS_RETURN(ret);
        return ret;
	}
	errno = ENOENT;
	return -1;
}
/*! \endcond */

/*! @} */



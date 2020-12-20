// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include <string.h>
#include  "unistd_fs.h"

/*! \details This function changes the mode of the specified file
 * or directory.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EIO:  IO Error
 * - ENAMETOOLONG: \a path exceeds PATH_MAX or a component of \a path exceeds NAME_MAX
 * - ENOENT: \a path does not exist
 * - EACCES: search permission is denied for a component of \a path
 *
 *
 */
int chmod(const char *path, mode_t mode){
	const sysfs_t * fs;

	if ( sysfs_ispathinvalid(path) == true ){
		return -1;
	}

	fs = sysfs_find(path, true);
	if ( fs != NULL ){
        int ret = fs->chmod(fs->config, sysfs_stripmountpath(fs, path), mode);
        SYSFS_PROCESS_RETURN(ret);
        return ret;
	}
	errno = ENOENT;
	return -1;
}

/*! @} */


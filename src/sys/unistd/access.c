// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup unistd
 * @{
 */

/*! \file
 */

#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/syslimits.h>
#include <string.h>
#include <stdio.h>

#include "sos/sos.h"
#include "sos/fs/sysfs.h"


/*! \details This function checks to see if the specified access (\a amode)
 * is allowed for \a path.
 *
 * \return Zero on success (ie \a amode is allowed) or -1 with errno (see \ref errno) set to:
 * - ENAMETOOLONG: \a path exceeds PATH_MAX or a component of \a path exceeds NAME_MAX
 * - ENOENT: \a path does not exist
 * - EACCES: \a amode is not allowed for \a path or search permission is denied for a component of \a path
 *
 */
int access(const char * path, int amode){
	const sysfs_t * fs;
	struct stat st;
	int ret;

	//Do a safe check of the path string length
	if ( sysfs_ispathinvalid(path) == true ){
		return -1;
	}

	fs = sysfs_find(path, true);
	if ( fs == NULL ){
		errno = ENOENT;
		return -1;
	}

	if ( (ret = fs->stat(fs->config, sysfs_stripmountpath(fs, path), &st)) < 0 ){
		SYSFS_PROCESS_RETURN(ret);
		return ret;
	}

	return sysfs_access(st.st_mode, st.st_uid, st.st_gid, amode);
}


/*! @} */


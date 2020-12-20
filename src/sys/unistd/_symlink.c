// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include "unistd_local.h"
#include "unistd_fs.h"

/*! \details This function creates a hard link between \a old and \a new.
 *
 * \return Zero on success or -1 on error with errno (see \ref errno) set to:
 *  - ENOTSUP:  operation not supported
 *
 */
int symlink(const char *old, const char *new);

/*! \cond */
int _symlink(const char *old, const char *new){
	const sysfs_t * fs_old;
	const sysfs_t * fs_new;

	if ( sysfs_ispathinvalid(old) == true ){
		return -1;
	}

	if ( sysfs_ispathinvalid(new) == true ){
		return -1;
	}

	fs_old = sysfs_find(old, true);
	fs_new = sysfs_find(new, true);

	if ( (fs_old == fs_new) && (fs_new != NULL) ){
		return fs_new->symlink(&fs_new->config,
				sysfs_stripmountpath(fs_old, old),
				sysfs_stripmountpath(fs_new, new)
				);
	}

	errno = ENOTSUP;
	return -1;
}
/*! \endcond */

/*! @} */


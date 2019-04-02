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



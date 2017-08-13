/* Copyright 2011-2016 Tyler Gilbert; 
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

/*! \addtogroup UNI_FILDES
 * @{
 */

/*! \file */

#include  "unistd_fs.h"

/*! \details This function is equivalent to \ref stat() except
 * path refers to a symbolic link.
 *
 * \return Zero on success or -1 on error with errno (see \ref ERRNO) set to:
 * - ENAMETOOLONG: \a path exceeds PATH_MAX or a component of \a path exceeds NAME_MAX
 * - ENOENT: \a path does not exist
 * - EACCES: search permission is denied for a component of \a path
 *
 */
int lstat(const char * path /*! The path the to symbolic link */,
		struct stat *buf /*! The destination buffer */);

int lstat(const char * path, struct stat *buf){
	const sysfs_t * fs;

	if ( sysfs_ispathinvalid(path) == true ){
		return -1;
	}

	fs = sysfs_find(path, true);
	if ( fs != NULL ){
		return fs->lstat(fs->config, sysfs_stripmountpath(fs, path), buf);
	}
	errno = ENOENT;
	return -1;
}

/*! @} */



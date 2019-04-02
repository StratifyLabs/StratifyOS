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


/*! \addtogroup directory
 * @{
 *
 * \details This interface accesses directory entries (both files and folders).  Here is an example of how to use this
 * interface.
 *
 * \code
 * #include <dirent.h>
 * #include <stdio.h>
 *
 * void show_directory(){
 * 	DIR * dirp;
 * 	struct dirent entry;
 * 	struct dirent * result;
 *
 * 	dirp = opendir("/path/to/dir");
 * 	if ( dirp == NULL ){
 * 		perror("failed to open directory");
 * 		return;
 * 	}
 *
 * 	while( readdir_r(dirp, &entry, &result) == 0 ){
 * 		printf("Directory name (number): %s (%d)\n", entry.d_name, entry.d_ino);
 * 	}
 *
 * 	if ( closedir(dirp) < 0 ){
 * 		perror("failed to close directory");
 * 	}
 *
 * }
 *
 * \endcode
 *
 *
 */

/*! \file */

#include "config.h"
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include "sos/fs/sysfs.h"

#include "dirent.h"
#include "mcu/debug.h"

/*! \cond */
static int check_ebadf(DIR * dirp){
	if ( dirp == NULL ){
		errno = EBADF;
		return -1;
	}

	if( cortexm_verify_zero_sum32(dirp, sizeof(DIR)/sizeof(u32)) == 0 ){
		errno = EBADF;
		return -1;
	}
	return 0;
}
/*! \endcond */

/*! \details Closes the directory stream specified by \a dirp.
 *
 * \return Zero or -1 with errno (see \ref errno) set to:
 * - EINVAL: \a dirp does not refere to an open directory stream
 *
 */
int closedir(DIR * dirp /*! A pointer to the open directory */){
	int ret;
	const sysfs_t * fs;

	if (check_ebadf(dirp) < 0 ){ return -1; }
	fs = dirp->fs;
	ret = fs->closedir(fs->config, &(dirp->handle));
	SYSFS_PROCESS_RETURN(ret);

	free(dirp);
	return ret;
}

/*! \details Opens a directory.
 *
 * \return a pointer to the directory or NULL with errno (see \ref errno) set to:
 * - ENOMEM: not enough memory
 * - ENOENT: \a dirname was not found
 * - EACCES: read access to \a dirname is not allowed
 * - ENAMETOOLONG: \a dirname exceeds \a PATH_MAX or an element of \a dirname exceeds \a NAME_MAX
 *
 */
DIR * opendir(const char * dirname){
	int err;
	DIR * dirp;
	const sysfs_t * fs;

	if ( sysfs_ispathinvalid(dirname) == true ){
		return NULL;
	}

	fs = sysfs_find(dirname, false); //see which filesystem we are working with first
	if ( fs == NULL ){ //if no filesystem is found for the path, return no entity
		errno = ENOENT;
		return NULL;
	}

	dirp = malloc(sizeof(DIR));
	if ( dirp == NULL ){
		errno = ENOMEM;
		return NULL;
	}

	//Open the directory and check for errors
	err = fs->opendir(fs->config, &(dirp->handle), sysfs_stripmountpath(fs, dirname));
	if ( err < 0 ){
		SYSFS_PROCESS_RETURN(err);
		free(dirp);
		return NULL;
	}

	//Initialize the DIR structure
	dirp->fs = fs;
	dirp->loc = 0;

	cortexm_assign_zero_sum32(dirp, sizeof(DIR)/sizeof(u32));

	//Return the pointer to the table
	return dirp;
}

/*! \details Reads the next directory entry in the open directory.
 * \note This function is not thread-safe nor re-entrant;  use \ref readdir_r()
 * as a thread-safe, re-entrant alternative.
 *
 * \return a pointer to a dirent or NULL with errno (see \ref errno) set to:
 * - EBADF: \a dirp is invalid
 * - ENOENT: the current position of the directory stream is invalid
 *
 */
struct dirent *readdir(DIR * dirp /*! a pointer to the directory structure */){
	static struct dirent result;
	struct dirent * tmp;
	readdir_r(dirp, &result, &tmp);
	return tmp;
}

/*! \details Reads the next directory entry in the open directory (reentrant version).
 *
 * \return a pointer to a dirent or NULL with errno (see \ref errno) set to:
 * - EBADF: \a dirp is invalid
 * - ENOENT: the current position of the directory stream is invalid
 *
 */
int readdir_r(DIR * dirp /*! a pointer to the directory structure */,
				  struct dirent * entry /*! a pointer to the destination memory */,
				  struct dirent ** result /*! this value is assigned to \a entry on success and NULL on failure */){
	int err;
	const sysfs_t * fs;
	if (check_ebadf(dirp) < 0 ){
		if ( result ){
			*result = NULL;
		}
		return -1;
	}

	fs = dirp->fs;
	err = fs->readdir_r(fs->config, dirp->handle, dirp->loc, entry);
	if ( err < 0 ){
		//errno is set by fs->readdir_r
		SYSFS_PROCESS_RETURN(err);
		if ( result ){
			*result = NULL;
		}
		return err;
	}

	dirp->loc++;
	cortexm_assign_zero_sum32(dirp, sizeof(DIR)/sizeof(u32));

	if ( result ){
		*result = entry;
	}
	return 0;
}

/*! \details Rewinds \a dirp.
 *
 */
void rewinddir(DIR * dirp /*! a pointer to the directory structure */){
	if( check_ebadf(dirp) < 0 ){ return; }
	dirp->loc = 0;
	cortexm_assign_zero_sum32(dirp, sizeof(DIR)/sizeof(u32));
}

/*! \details Seeks to the specified location in
 * the directory.
 *
 */
void seekdir(DIR * dirp /*! a pointer to the directory structure */,
				 long loc /*! the target location */){
	if( check_ebadf(dirp) < 0 ){ return; }
	dirp->loc = loc;
	cortexm_assign_zero_sum32(dirp, sizeof(DIR)/sizeof(u32));
}

/*! \details Gets the current location in the directory.
 *
 * \return The current directory location
 */
long telldir(DIR * dirp /*! a pointer to the directory structure */){
	if( check_ebadf(dirp) < 0 ){ return SYSFS_RETURN_EOF; }
	return dirp->loc;
}

/*! @} */


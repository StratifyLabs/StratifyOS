// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include "unistd_local.h"
#include "unistd_fs.h"

/*! \details This function creates a new directory.
 *
 * \param path Path to the new directory
 * \param mode Ignored
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - ENOENT:  \a path is an empty string or the parent directory cannot be found
 * - EEXIST:  \a path already exists
 * - ENOSPC:  Not enough space on the disk to add a new directory
 *
 */
int mkdir(const char *path, mode_t mode){
   const sysfs_t * fs;

   if ( sysfs_ispathinvalid(path) == true ){
      return -1;
   }

   fs = sysfs_find(path, true);
   if ( fs != NULL ){
			int ret = fs->mkdir(fs->config,
                      sysfs_stripmountpath(fs, path),
                      mode);
      SYSFS_PROCESS_RETURN(ret);
      return ret;
   }
   errno = ENOENT;
   return -1;
}

/*! @} */

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
   int ret = -1;

   if ( sysfs_ispathinvalid(path) == true ){
      return -1;
   }

   fs = sysfs_find(path, true);
   if ( fs != NULL ){
      ret = fs->mkdir(fs->config,
                      sysfs_stripmountpath(fs, path),
                      mode);
      SYSFS_PROCESS_RETURN(ret);
      return ret;
   }
   errno = ENOENT;
   return -1;
}

/*! @} */

// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include "unistd_fs.h"
#include "unistd_local.h"

/*! \details This function removes the directory specified by \a path.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - ENOENT:  \a path is an empty string or the parent directory cannot be found
 * - EEXIST:  \a path already exists
 * - ENOTDIR:  \a path is not a directory
 * - ENOTEMPTY:  \a path is not an empty directory
 *
 */
int rmdir(const char *path) {
  const sysfs_t *fs;

  if (sysfs_ispathinvalid(path) == true) {
    return -1;
  }

  fs = sysfs_find(path, true);
  if (fs != NULL) {
    int ret = fs->rmdir(fs->config, sysfs_stripmountpath(fs, path));
    SYSFS_PROCESS_RETURN(ret);
    return ret;
  }
  errno = ENOENT;
  return -1;
}

/*! @} */

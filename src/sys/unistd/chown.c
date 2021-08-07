// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include "unistd_fs.h"
#include <string.h>

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
int chown(const char *path, uid_t uid, gid_t gid) {
  const sysfs_t *fs;

  if (sysfs_ispathinvalid(path) == true) {
    return -1;
  }

  fs = sysfs_find(path, true);
  if (fs != NULL) {
    int ret = fs->chown(fs->config, sysfs_stripmountpath(fs, path), uid, gid);
    SYSFS_PROCESS_RETURN(ret);
    return ret;
  }
  errno = ENOENT;
  return -1;
}

/*! @} */

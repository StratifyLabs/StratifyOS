// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include "unistd_fs.h"

/*! \details Deletes a file or directory from the filesystem. */
int unlink(const char *name);

/*! \cond */
int _unlink(const char *name) {
  const sysfs_t *fs;

  if (sysfs_ispathinvalid(name) == true) {
    return -1;
  }

  fs = sysfs_find(name, true);
  if (fs != NULL) {
    int ret = fs->unlink(fs->config, sysfs_stripmountpath(fs, name));
    SYSFS_PROCESS_RETURN(ret);
    return ret;
  }
  errno = ENOENT;
  return -1;
}
/*! \endcond */

/*! @} */

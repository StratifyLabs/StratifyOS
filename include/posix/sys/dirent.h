// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup directory
 * @{
 */

/*! \file */

#ifndef DIRENT_H_
#define DIRENT_H_

#include <limits.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif


#if !defined __link

/*! \brief Directory Entry
 * \details This defines the data structure for a directory entry.
 *
 */
struct dirent {
	ino_t d_ino /*! \brief File Serial number */;
	char d_name[NAME_MAX+1] /*! \brief Name of entry */;
};

/*! \cond */
typedef struct {
	const void * fs;
	void * handle;
	long loc;
	 unsigned int checksum;
} DIR;
/*! \endcond */

int closedir(DIR * dirp);
DIR * opendir(const char * dirname);
struct dirent *readdir(DIR * dirp);
int readdir_r(DIR * dirp, struct dirent * entry, struct dirent ** result);
void rewinddir(DIR * dirp);
void seekdir(DIR * dirp, long loc);
long telldir(DIR * dirp);

#endif

#ifdef __cplusplus
}
#endif

#endif /* DIRENT_H_ */

/*! @} */

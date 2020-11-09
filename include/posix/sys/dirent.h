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


typedef struct {
	const void * fs;
	void * handle;
	long loc;
	 unsigned int checksum;
} DIR;

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

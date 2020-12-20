// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup unistd
 * @{
 */

#include <fcntl.h>
#include <errno.h>


int _execve(const char *path, char *const argv[], char *const envp[]){
	//This needs to manipulate the task table so that the current process is replaced by a new image
	errno = ENOTSUP;
	return -1;

}


/*! @} */





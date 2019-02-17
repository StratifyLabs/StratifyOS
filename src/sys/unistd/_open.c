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


#include <reent.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>

#include "unistd_local.h"
#include "sos/dev/sys.h"
#include "cortexm/cortexm.h"
#include "mcu/mcu.h"
#include "unistd_fs.h"
#include "../scheduler/scheduler_local.h"

/*! \cond */
void root_assign_handle(void * args){
	int * argp = args;
	int i;
	int start = *argp;
	*argp = -1;
	for(i = start; i < OPEN_MAX; i++){
		//can't be interrupted
		if ( get_handle(i) == NULL ){
			set_handle(i, (void*)1);
			*argp = i;
			return;
		}
	}
}

int u_new_open_file(int start){
	int arg = start;

	cortexm_svcall(root_assign_handle, &arg);
	if( arg < 0 ){
		errno = EMFILE;
	}
	return arg;
}

void u_reset_fildes(int fildes){
	set_handle(fildes, NULL);
}

void set_open_file(int fildes, const sysfs_t * fs, void * handle, uint16_t flags){
	set_fs(fildes, fs);
	set_handle(fildes, handle);
	set_loc(fildes, 0);
	set_flags(fildes, flags);
}
/*! \endcond */

/*! \details This function opens a file with flags being the OR'd combination of:
 * - O_RDONLY, O_WRONLY or O_RDWR
 * - O_NONBLOCK, O_CREAT, O_EXCL, O_TRUNC
 *
 * If the O_CREAT flag is set, the third argument should specify the mode as a mode_t.
 * The bits used with the mode are:
 * - S_IRWXU:  User read/write/execute
 * - S_IRUSR:  User read
 * - S_IWUSR:  User write
 * - S_IXUSR:  User execute
 * - S_IRWXG:  Group read/write/execute
 * - S_IRGRP:  Group read (groups not implemented)
 * - S_IWGRP:  Group write (groups not implemented)
 * - S_IXGRP:  Group execute (groups not implemented)
 * - S_IRWXO:  Other read/write/execute
 * - S_IROTH:  Other read
 * - S_IWOTH:  Other write
 * - S_IXOTH:  Other execute
 *
 * \return Zero on success or -1 on error with errno (see \ref errno) set to:
 *  - ENAMETOOLONG:  \a name exceeds PATH_MAX or a component of \a name exceeds NAME_MAX
 *  - ENOENT: \a name could not be found
 *  - EIO: IO error
 *  - EEXIST: \a name already exists and flags is not set to overwrite
 *  - ENOMEM: not enough memory to open another file
 *	- ENOTDIR: the path to \a name does not exist
 *	- EFBIG: size error with the file (file is likely corrupt)
 *
 *
 */
int open(const char * name, int flags, ... );

/*! \cond */
int _open(const char * name, int flags, ...) {
	int tmp;
    int ret;
	int fildes;
	va_list ap;
	int mode;
	const sysfs_t * fs;

	//Check the length of the filename
	if ( sysfs_ispathinvalid(name) == true ){
		return -1;
	}

	fs = sysfs_find(name, true); //see which filesystem we are working with first
	if ( fs == NULL ){ //if no filesystem is found for the path, return no entity
		errno = ENOENT;
		return -1;
	}

	fildes = u_new_open_file(0); //get a new file descriptor
	if ( fildes < 0 ){
		return -1;
	}

	tmp = 0;
	//check the access mode
	switch( (flags & O_ACCMODE) ){
	case O_RDONLY:
		tmp = R_OK;
		break;
	case O_WRONLY:
		tmp = W_OK;
		break;
	case O_RDWR:
		tmp = R_OK|W_OK;
		break;
	}

	if ( flags & O_CREAT ){

		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
		tmp = mode & S_IFMT;
		switch(tmp){
		case S_IFDIR:
			//This is not the correct way to create a directory (must use mkdir)
			errno = EINVAL;
			return -1;
		case 0:
			//If no format is specified then create a regular file
			mode = mode | S_IFREG;
			break;
		}
	} else {
		mode = 0;
	}

	set_open_file(fildes, fs, (void*)1, flags);

    if( (ret = sysfs_file_open(get_open_file(fildes), sysfs_stripmountpath(fs, name), mode)) <  0){
		u_reset_fildes(fildes);
        return ret;
	}

	if ( flags & O_APPEND ){
		lseek(fildes, 0, SEEK_END);
	}

	return fildes;
}

int u_fildes_is_bad(int fildes){
	if ( fildes < 0 ){
		errno = EBADF;
		return -1;
	}

	if ( fildes < OPEN_MAX ){
		//since u_new_open_file() assigns handle to 1 temporarily, a handle of 0 or 1 is invalid
		if ( ((int)get_handle(fildes) & ~(0x01)) != 0 ){
			return fildes;
		} else {
			errno = EBADF;
			return -1;
		}
	}

	if ( fildes & FILDES_STDIO_FLAG ){
		fildes = u_init_stdio(fildes);
	} else {
		return -1;
	}

	return fildes;
}

/*
typedef struct {
	int err;
	const device_t * handle;
} root_open_device_t;
static void root_open_device(void * args);
static int open_device(const char * name, int flags, int fildes);
static void set_open_file(int fildes, void * handle, uint16_t flags);

 */

int u_init_stdio(int fildes){
#if USE_STDIO != 0
	FILE * ptr;
	switch(fildes){
	case STDIN_FILENO:
		ptr = _REENT->_stdin;
		break;
	case STDOUT_FILENO:
		ptr = _REENT->_stdout;
		break;
	case STDERR_FILENO:
		ptr = _REENT->_stderr;
		break;
	default:
		errno = EBADF;
		return -1;
	}

	//threads should use the already open descriptors of the process
	if( _GLOBAL_REENT != _REENT ){
		_REENT->_stdin->_file = _GLOBAL_REENT->_stdin->_file;
		_REENT->_stdout->_file = _GLOBAL_REENT->_stdout->_file;
		_REENT->_stderr->_file = _GLOBAL_REENT->_stderr->_file;
	} else {

		if( sos_board_config.o_sys_flags & SYS_FLAG_IS_STDIO_FIFO ){
			//the main thread needs to open the STDIO directly
			_REENT->_stdin->_file = _open(sos_board_config.stdin_dev, O_RDWR);
			_REENT->_stdout->_file = _open(sos_board_config.stdout_dev, O_RDWR);
			_REENT->_stderr->_file = _open(sos_board_config.stderr_dev, O_RDWR);
		} else {
			_REENT->_stdin->_file = _open(sos_board_config.stdin_dev, O_RDWR);
			_REENT->_stdout->_file = _REENT->_stdin->_file;
			_REENT->_stderr->_file = _REENT->_stdin->_file;
		}
	}

	return ptr->_file;
#else
	return 0;
#endif
}
/*! \endcond */




/*! @} */


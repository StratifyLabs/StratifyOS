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

#ifndef UNISTD_FLAGS_H_
#define UNISTD_FLAGS_H_

#include <stdint.h>
#include <sys/lock.h>
#include <sys/stat.h>
#include "dirent.h"
#include "mcu/mcu.h"
#include "sos/fs/sysfs.h"

#define FILDES_STDIO_FLAG 0x0200
#define FILDES_SOCKET_FLAG 0x0400
#define FILDES_IS_SOCKET(x) ((x >= 0) && ((x & FILDES_SOCKET_FLAG) != 0))


int u_new_open_file(int start);


int u_init_stdio(int fildes);
int u_get_open_file(int fildes);
int u_fildes_is_bad(int fildes);
void u_reset_fildes(int fildes);

typedef struct {
	int err;
	const sysfs_t * fs;
	void * handle;
	int request;
	void * ctl;
} u_priv_attr_t;
void u_priv_ioctl(void * args) MCU_ROOT_EXEC_CODE;


static inline void * get_handle(int fildes) MCU_ALWAYS_INLINE;
void * get_handle(int fildes){
	return _global_impure_ptr->procmem_base->open_file[fildes].handle;
}

static inline void ** get_handle_ptr(int fildes) MCU_ALWAYS_INLINE;
void ** get_handle_ptr(int fildes){
	return &(_global_impure_ptr->procmem_base->open_file[fildes].handle);
}

static inline const sysfs_t * get_fs(int fildes) MCU_ALWAYS_INLINE;
const sysfs_t * get_fs(int fildes){
	return (const sysfs_t*)_global_impure_ptr->procmem_base->open_file[fildes].fs;
}

static inline int get_flags(int fildes) MCU_ALWAYS_INLINE;
int get_flags(int fildes){
	return _global_impure_ptr->procmem_base->open_file[fildes].flags;
}

static inline int get_loc(int fildes) MCU_ALWAYS_INLINE;
int get_loc(int fildes){
	return _global_impure_ptr->procmem_base->open_file[fildes].loc;
}

static inline void * get_open_file(int fildes) MCU_ALWAYS_INLINE;
void * get_open_file(int fildes){
	return (open_file_t*)&(_global_impure_ptr->procmem_base->open_file[fildes]);
}

static inline void set_loc(int fildes, int loc) MCU_ALWAYS_INLINE;
void set_loc(int fildes, int loc){
	_global_impure_ptr->procmem_base->open_file[fildes].loc = loc;
}

static inline void set_handle(int fildes, void * handle) MCU_ALWAYS_INLINE;
void set_handle(int fildes, void * handle){
	_global_impure_ptr->procmem_base->open_file[fildes].handle = handle;
}

static inline void set_fs(int fildes, const sysfs_t * fs) MCU_ALWAYS_INLINE;
void set_fs(int fildes, const sysfs_t * fs){
	_global_impure_ptr->procmem_base->open_file[fildes].fs = (void*)fs;
}

static inline void set_flags(int fildes, int flags) MCU_ALWAYS_INLINE;
void set_flags(int fildes, int flags){
	_global_impure_ptr->procmem_base->open_file[fildes].flags = flags;
}

static inline void dup_open_file(int new_fd, int old_fd) MCU_ALWAYS_INLINE;
void dup_open_file(int new_fd, int old_fd){
	memcpy( &(_global_impure_ptr->procmem_base->open_file[new_fd]),
			&(_global_impure_ptr->procmem_base->open_file[old_fd]),
			sizeof(open_file_t));
}

int unistd_update_loc(sysfs_file_t * file, int adjust);



#endif /* UNISTD_FLAGS_H_ */

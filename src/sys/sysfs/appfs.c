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


#include <stddef.h>
#include <reent.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "cortexm/task.h"
#include "mcu/core.h"
#include "mcu/mem.h"
#include "mcu/wdt.h"
#include "cortexm/mpu.h"
#include "mcu/debug.h"
#include "appfs_local.h"
#include "sos/fs/sysfs.h"
#include "../scheduler/scheduler_local.h"

#define ANALYZE_PATH_NOENT -1
#define ANALYZE_PATH_ROOT 0
#define ANALYZE_PATH_INSTALL 1
#define ANALYZE_PATH_FLASH 2
#define ANALYZE_PATH_FLASH_DIR 3
#define ANALYZE_PATH_RAM 4
#define ANALYZE_PATH_RAM_DIR 5

static void root_ioctl(void * args);

static int analyze_path(const char * path, const char ** name, int * mem_type){
	int elements;

	if ( sysfs_ispathinvalid(path) == true ){
		return -1;
	}

	*name = sysfs_getfilename(path, &elements);
	if ( strcmp(*name, "") == 0 ){
		return ANALYZE_PATH_ROOT;
	}

	if (elements == 1){
		if ( strncmp(*name, ".install", NAME_MAX) == 0 ){
			return ANALYZE_PATH_INSTALL;
		}

		if ( strncmp(*name, "flash", NAME_MAX) == 0 ){
			return ANALYZE_PATH_FLASH_DIR;
		}

		if ( strncmp(*name, "ram", NAME_MAX) == 0 ){
			return ANALYZE_PATH_RAM_DIR;
		}

		return ANALYZE_PATH_NOENT;

	} else if ( elements == 2 ){
		if ( strncmp(path, "flash/", 6) == 0 ){
			*mem_type = MEM_FLAG_IS_FLASH;
			return ANALYZE_PATH_FLASH;
		}

		if ( strncmp(path, "ram/", 4) == 0 ){
			*mem_type = MEM_FLAG_IS_RAM;
			return ANALYZE_PATH_RAM;
		}

	} else if ( elements > 2 ){
		return ANALYZE_PATH_NOENT;
	}

	return -1;

}

static bool is_free(const char * name){
	if ( strncmp(name, ".free", 5) == 0 ){
		return true;
	}
	return false;
}

static bool is_sys(const char * name){
	if ( strncmp(name, ".sys", 4) == 0 ){
		return true;
	}
	return false;
}

int appfs_init(const void * cfg){
	int i;
	mem_info_t info;
	root_load_fileinfo_t root_file_info;
	u32 buf[APPFS_RAM_USAGE_WORDS_SIZE];
	const devfs_device_t * dev;
	dev = cfg;

	//the RAM usage table needs to be initialized
	appfs_ram_initusage(buf);

	//first mark the RAM used by the OS according to the system memory size
	appfs_ram_setrange(buf, mcu_mem_getsyspage(), sos_board_config.sys_memory_size, APPFS_MEMPAGETYPE_SYS);

	//now scan each flash page to see what RAM is used by applications
	dev->driver.ioctl(&(dev->handle), I_MEM_GETINFO, &info);
	for(i=0; i < info.flash_pages; i++){
		if ( (appfs_util_getfileinfo(&root_file_info, dev, i, MEM_FLAG_IS_FLASH, NULL) == APPFS_MEMPAGETYPE_USER) && (appfs_util_isexecutable(&root_file_info.fileinfo) == true) ){

			//get the RAM page associated with the data start
			root_file_info.pageinfo.o_flags = MEM_FLAG_IS_QUERY;
			root_file_info.pageinfo.addr = (int)root_file_info.fileinfo.exec.ram_start;
			if ( dev->driver.ioctl(&(dev->handle), I_MEM_GETPAGEINFO, &root_file_info.pageinfo) < 0 ){
				continue;
			}

			appfs_ram_setrange(buf,
									 root_file_info.pageinfo.num,
									 root_file_info.fileinfo.exec.ram_size,
									 APPFS_MEMPAGETYPE_SYS);
		}
	}
	cortexm_svcall(appfs_ram_root_saveusage, buf);

	return 0;
}

int appfs_startup(const void * cfg){
	int i;
	mem_info_t info;
	root_load_fileinfo_t root_file_info;
	task_memories_t mem;
	int started;
	const devfs_device_t * dev;

	dev = cfg;

	//go through each flash page and look for programs that should be run on startup
	started = 0;
	dev->driver.ioctl(&(dev->handle), I_MEM_GETINFO, &info);
	for(i=0; i < info.flash_pages; i++){
		if (appfs_util_getfileinfo(&root_file_info, dev, i, MEM_FLAG_IS_FLASH, NULL) == APPFS_MEMPAGETYPE_USER ){
			if ( (appfs_util_isexecutable(&root_file_info.fileinfo) == true) &&
				  (root_file_info.fileinfo.exec.o_flags & APPFS_FLAG_IS_STARTUP) ){

				//start the process
				mem.code.addr = (void*)root_file_info.fileinfo.exec.code_start;
				mem.code.size = root_file_info.fileinfo.exec.code_size;
				mem.data.addr = (void*)root_file_info.fileinfo.exec.ram_start;
				mem.data.size = root_file_info.fileinfo.exec.ram_size;

				if ( scheduler_create_process((void*)root_file_info.fileinfo.exec.startup,
														0,
														&mem,
														(void*)root_file_info.fileinfo.exec.ram_start,
														0) >= 0 ){
					started++;
					mcu_debug_log_info(MCU_DEBUG_APPFS, "Started %s", root_file_info.fileinfo.hdr.name);
				} else {
					mcu_debug_log_info(MCU_DEBUG_APPFS, "Failed to start %s", root_file_info.fileinfo.hdr.name);
				}

			}
		}
	}
	return started;
}

int appfs_mkfs(const void* cfg){
	//erase all user files in flash and ram
	return 0;
}

int appfs_open(const void * cfg, void ** handle, const char * path, int flags, int mode){
	int ret;
	appfs_handle_t * h;
	root_load_fileinfo_t args;
	int path_type;
	int mem_type;
	int size;
	const char * name;

	if ( (path_type = analyze_path(path, &name, &mem_type)) < 0 ){
		return -1;
	}

	ret = 0;

	//allocate memory for the handle
	h = malloc(sizeof(appfs_handle_t));
	if ( h == NULL ){
		//errno should be set by malloc
		return -1;
	}

	ret = 0;
	switch(path_type){
		case ANALYZE_PATH_INSTALL:
			if( (flags & O_ACCMODE) != O_WRONLY ){
				ret = SYSFS_SET_RETURN(EINVAL);
			}
			h->is_install = 1;
			break;

		case ANALYZE_PATH_RAM:
		case ANALYZE_PATH_FLASH:
			h->is_install = 0;
			if( (flags & O_ACCMODE) != O_RDONLY ){
				ret = SYSFS_SET_RETURN(EROFS);
			}
			break;
		default:
			ret = SYSFS_SET_RETURN(ENOENT);
	}

	if( ret == 0 ){
		if ( path_type != ANALYZE_PATH_INSTALL ){
			//check to see if the file exists
			if( appfs_util_lookupname(cfg, name, &args, mem_type, &size) == 0 ){  //the file already exists
				//open the existing file
				h->type.reg.beg_addr = args.pageinfo.addr;
				h->type.reg.page = args.pageinfo.num;
				h->type.reg.o_flags = args.pageinfo.o_flags;
				h->type.reg.size = size;
				h->type.reg.mode = args.fileinfo.hdr.mode;
			} else { //the file does not already exist
				ret = SYSFS_SET_RETURN(ENOENT);
			}
		}
	}

	if ( ret == -1 ){
		free(h);
		h = NULL;
	}
	*handle = h;
	return ret;
}

int appfs_unlink(const void* cfg, const char * path){
	//this will erase the page associated with the filename -- system files are read-only
	root_load_fileinfo_t args;
	int start_page;
	int tmp;
	int end_page;
	int size_deleted;
	const char * name;
	int mem_type;

	if( analyze_path(path, &name, &mem_type) < 0 ){
		return -1;
	}

	//sys and free files cannot be deleted
	if ( is_sys(name) || is_free(name) ){
		return SYSFS_SET_RETURN(ENOTSUP);
	}

	//see if path exists
	if ( appfs_util_lookupname(cfg, name, &args, mem_type, NULL) < 0 ){
		return SYSFS_SET_RETURN(ENOENT);
	}


	//executable files are deleted based on the header file
	if ( mem_type == MEM_FLAG_IS_FLASH ){
		start_page = args.pageinfo.num;
		size_deleted = args.pageinfo.size;  //start with the first page


		//need to read the size of each consecutive page until the size is met
		while(size_deleted < (args.fileinfo.exec.code_size + args.fileinfo.exec.data_size) ){
			if ( appfs_util_getpageinfo(cfg, &args.pageinfo) ){
				return -1;
			}
			size_deleted += args.pageinfo.size;
			args.pageinfo.num++;
		}

		end_page = args.pageinfo.num;

		appfs_util_erasepages(cfg, start_page, end_page);


	} else {
		//The Ram size is the code size + the data size round up to the next power of 2 to account for memory protection
		tmp = mpu_getnextpowerof2(args.fileinfo.exec.code_size + args.fileinfo.exec.data_size);
		if ( appfs_ram_setusage(args.pageinfo.num, tmp, APPFS_MEMPAGETYPE_FREE) < 0 ){
			return SYSFS_SET_RETURN(EIO);
		}
	}

	//check to see if the file is a data file (in this case no RAM is used)
	if( args.fileinfo.exec.signature != APPFS_CREATE_SIGNATURE ){
		//remove the application from the RAM usage table
		args.pageinfo.addr = (int)args.fileinfo.exec.ram_start;
		args.pageinfo.o_flags = MEM_FLAG_IS_QUERY;

		if ( appfs_util_getpageinfo(cfg, &args.pageinfo) ){
			return SYSFS_SET_RETURN(EIO);
		}

		if ( appfs_ram_setusage(args.pageinfo.num, args.fileinfo.exec.ram_size, APPFS_MEMPAGETYPE_FREE) < 0 ){
			return SYSFS_SET_RETURN(EIO);
		}
	}

	return 0;
}

int appfs_fstat(const void* cfg, void * handle, struct stat * st){
	appfs_handle_t * h;
	root_load_fileinfo_t args;
	int size;

	h = handle;

	memset(st, 0, sizeof(struct stat));

	if ( h->is_install == true ){
		st->st_mode = S_IFBLK | 0222;
		return 0;
	}

	if ( appfs_util_getfileinfo(&args, cfg, h->type.reg.page, h->type.reg.o_flags, &size) < 0 ){
		return -1;
	}

	st->st_ino = args.pageinfo.num;
	st->st_blksize = args.pageinfo.size;
	if( args.fileinfo.hdr.mode == 0444 ){ //this is a read only data file -- peel off the header from the size
		size -= sizeof(appfs_file_t);
	}
	st->st_size = size;
	st->st_blocks = 1;
	st->st_uid = 0;
	st->st_gid = 0;
	st->st_mode = args.fileinfo.hdr.mode | S_IFREG;

	return 0;
}

int appfs_stat(const void* cfg, const char * path, struct stat * st){
	root_load_fileinfo_t args;
	appfs_handle_t handle;
	int path_type;
	int mem_type;
	int size;
	const char * name;

	if ( (path_type = analyze_path(path, &name, &mem_type)) < 0 ){
		return SYSFS_SET_RETURN(ENOENT);
	}

	switch(path_type){
		case ANALYZE_PATH_ROOT:
			st->st_size = 0;
			st->st_blocks = 1;
			st->st_uid = 0;
			st->st_gid = 0;
			st->st_mode = S_IFDIR | 0777;
			return 0;
		case ANALYZE_PATH_INSTALL:
			st->st_size = 0;
			st->st_blocks = 1;
			st->st_uid = 0;
			st->st_gid = 0;
			handle.is_install = true;
			break;
		case ANALYZE_PATH_FLASH_DIR:
			st->st_size = 0;
			st->st_blocks = 1;
			st->st_uid = 0;
			st->st_gid = 0;
			st->st_mode = S_IFDIR | 0777;
			return 0;
		case ANALYZE_PATH_RAM_DIR:
			st->st_size = 0;
			st->st_blocks = 1;
			st->st_uid = 0;
			st->st_gid = 0;
			st->st_mode = S_IFDIR | 0777;
			return 0;

		case ANALYZE_PATH_RAM:
		case ANALYZE_PATH_FLASH:
			handle.is_install = 0;
			//see if the path matches the name of any pages
			if ( appfs_util_lookupname(cfg, name, &args, mem_type, &size) < 0 ){
				return SYSFS_SET_RETURN(ENOENT);
			}
			if( args.fileinfo.hdr.mode == 0444 ){
				size -= sizeof(appfs_file_t);
			}
			handle.type.reg.mode = args.fileinfo.hdr.mode;
			handle.type.reg.size = size;
			handle.type.reg.beg_addr = args.pageinfo.addr;
			handle.type.reg.page = args.pageinfo.num;
			handle.type.reg.o_flags = args.pageinfo.o_flags;
	}

	return appfs_fstat(cfg, &handle, st);
}

void root_read(void * args){
	sysfs_read_t * p = args;

	devfs_async_t async;
	const devfs_device_t * dev;
	appfs_handle_t * h;

	h = p->handle;
	dev = p->config;

	memset(&async, 0, sizeof(async));
	async.tid = task_get_current();
	async.buf = p->buf;
	async.nbyte = p->nbyte;
	async.flags = p->flags;
	async.loc = p->loc;

	if( h->type.reg.mode == 0444 ){
		async.loc += sizeof(appfs_file_t);
	}

	if ( async.loc >= h->type.reg.size ){
		p->ret = -1;
		return; //return EOF
	}

	//read should not go past the end of the file
	if ( (async.loc + async.nbyte) >= (h->type.reg.size) ){
		async.nbyte = h->type.reg.size - async.loc;
	}

	async.loc = (int)h->type.reg.beg_addr + async.loc;
	p->ret = dev->driver.read(&(dev->handle), &async);
}

int appfs_read(const void * cfg, void * handle, int flags, int loc, void * buf, int nbyte){
	sysfs_read_t args;
	args.config = cfg;
	args.handle = handle;
	args.flags = flags;
	args.loc = loc;
	args.buf = buf;
	args.nbyte = nbyte;

	cortexm_svcall(root_read, &args);
	return args.ret;

}

int appfs_write(const void * cfg, void * handle, int flags, int loc, const void * buf, int nbyte){
	return SYSFS_SET_RETURN(EROFS);
}


int appfs_close(const void* cfg, void ** handle){
	//close a file
	appfs_handle_t * h = (appfs_handle_t*)*handle;
	free(h);
	h = NULL;
	return 0;
}

int appfs_opendir(const void* cfg, void ** handle, const char * path){
	if ( strncmp(path, "", PATH_MAX) == 0 ){
		*handle = (void*)0;
	} else if ( strcmp(path, "flash") == 0 ){
		*handle = (void*)1;
	} else if ( strcmp(path, "ram") == 0 ){
		*handle = (void*)2;
	} else {
		return SYSFS_SET_RETURN(ENOENT);
	}
	return 0;
}



void root_ioctl(void * args){
	sysfs_ioctl_t * a = args;
	appfs_handle_t * h = a->handle;
	int request = a->request;
	appfs_installattr_t * attr;
	const appfs_file_t * f;

	appfs_info_t * info;
	void * ctl = a->ctl;
	a->ret = -1;

	mcu_wdt_reset();

	info = ctl;
	attr = ctl;
	switch( request ){

		//INSTALL and CREATE only with with the special .install file
		case I_APPFS_INSTALL:
			if( !h->is_install ){
				a->ret = SYSFS_SET_RETURN(ENOTSUP);
			} else {
				a->ret = appfs_util_root_writeinstall(a->cfg, h, attr);
				mcu_core_invalidate_instruction_cache();
			}
			break;

		case I_APPFS_CREATE:
			if( !h->is_install ){
				a->ret = SYSFS_SET_RETURN(ENOTSUP);
			} else {
				a->ret = appfs_util_root_create(a->cfg, h, attr);
				mcu_core_invalidate_data_cache();
			}
			break;

			//These calls work with the specific applications
		case I_APPFS_FREE_RAM:
			if( h->is_install ){
				a->ret = SYSFS_SET_RETURN(ENOTSUP);
			} else {
				a->ret =  appfs_util_root_free_ram(a->cfg, h);
			}
			break;

		case I_APPFS_RECLAIM_RAM:
			if( h->is_install ){
				a->ret = SYSFS_SET_RETURN(ENOTSUP);
			} else {
				a->ret = appfs_util_root_reclaim_ram(a->cfg, h);
			}
			break;

		case I_APPFS_GETINFO:
			if( h->is_install ){
				a->ret = SYSFS_SET_RETURN(ENOTSUP);
			} else {
				f = appfs_util_getfile(h);
				info->mode = f->hdr.mode;
				info->o_flags = f->exec.o_flags;
				info->signature = f->exec.signature;
				info->version = f->hdr.version;
				info->ram_size = f->exec.ram_size;
				memcpy(info->id, f->hdr.id, LINK_NAME_MAX);
				memcpy(info->name, f->hdr.name, LINK_NAME_MAX);
				a->ret = 0;
			}
			break;

		default:
			a->ret = SYSFS_SET_RETURN(EINVAL);
			break;
	}

}

int appfs_ioctl(const void * cfg, void * handle, int request, void * ctl){
	sysfs_ioctl_t args;
	args.cfg = cfg;
	args.handle = handle;
	args.request = request;
	args.ctl = ctl;
	cortexm_svcall(root_ioctl, &args);
	return args.ret;

}


static int readdir_mem(const void* cfg, int loc, struct dirent * entry, int type){
	//this needs to load page number loc and see what the file is
	root_load_fileinfo_t args;

	if ( appfs_util_getfileinfo(&args, cfg, loc, type, NULL) < 0 ){
		return SYSFS_SET_RETURN(ENOENT);
	}

	strncpy(entry->d_name, args.fileinfo.hdr.name, NAME_MAX);
	entry->d_ino = loc;
	return 0;
}


static int readdir_root(const void * cfg, int loc, struct dirent * entry){
	switch(loc){
		case 0:
			strncpy(entry->d_name, ".install", NAME_MAX);
			break;
		case 1:
			strncpy(entry->d_name, "flash", NAME_MAX);
			break;
		case 2:
			strncpy(entry->d_name, "ram", NAME_MAX);
			break;
		default:
			return SYSFS_SET_RETURN(ENOENT);
	}
	entry->d_ino = loc;
	return 0;
}


int appfs_readdir_r(const void* cfg, void * handle, int loc, struct dirent * entry){
	//this needs to load page number loc and see what the file is
	int h;
	h = (int)handle;
	switch(h){
		case 0:
			return readdir_root(cfg, loc, entry);
		case 1:
			return readdir_mem(cfg, loc, entry, MEM_FLAG_IS_FLASH);
		case 2:
			return readdir_mem(cfg, loc, entry, MEM_FLAG_IS_RAM);
		default:
			return -1;
	}
}



int appfs_closedir(const void* cfg, void ** handle){
	//this doesn't need to do anything at all
	return 0;
}


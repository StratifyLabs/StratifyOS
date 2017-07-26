/* Copyright 2011-2016 Tyler Gilbert; 
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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */

/*! \addtogroup SYSCALLS_PROCESS
 * @{
 */

/*! \file */

#include "config.h"

#include "sos/fs/devfs.h"
#include "cortexm/task.h"
#include "mcu/debug.h"

#include <fcntl.h>
#include <errno.h>
#include "sos/fs/sysfs.h"
#include <sys/syscalls/process/process_start.h>

#include "../../sched/sched_flags.h"
#include "../../sysfs/appfs.h"

static int reent_is_free(struct _reent * reent);

#if defined UNIQUE_PROCESS_NAMES
static uint8_t launch_count = 0;
#endif


int process_start(const char *path_arg,
		char *const envp[]
){
#if SINGLE_PROCESS == 0
	int fd;
	int err;
	appfs_file_t startup;
	task_memories_t mem;
	char tmp_path[PATH_MAX+1];
	char * p;
	char * path;
	char * process_path;
	int len;

	//find name - strip arguments
	memset(tmp_path, 0, PATH_MAX+1);
	strncpy(tmp_path, path_arg, PATH_MAX);
	path = strtok_r(tmp_path, sysfs_whitespace, &p);

	if( path == 0 ){
		mcu_debug_user_printf("can't get path from %s\n", path_arg);
		errno = EINVAL;
		return -1;
	}

	len = strlen(path_arg);

	if ( access(path, X_OK) < 0 ){
		mcu_debug_user_printf("no exec access\n");
		return -1;
	}

	//Open the program
	mcu_debug_user_printf("process_start:%s\n", path);
#if MCU_DEBUG
	usleep(10*1000);
#endif
	fd = open(path, O_RDONLY);
	if ( fd < 0 ){
		//The open() call set the errno already
		return -1;
	}

	//Read the program header
	err = read(fd, &startup, sizeof(appfs_file_t));
	if ( err != sizeof(appfs_file_t) ){
		//The read() function sets the errno already
		close(fd);
		return -1;
	}

	//verify the signature
	if( appfs_util_isexecutable(&startup) == false ){
		errno = ENOEXEC;
		return -1;
	}

	mem.code.addr = (void*)startup.exec.code_start;
	mem.code.size = startup.exec.code_size;
	mem.data.addr = (void*)startup.exec.ram_start;
	mem.data.size = startup.exec.ram_size;

	//check to see if the process is already running
	if( !reent_is_free((void*)startup.exec.ram_start) ){
		errno = ENOTSUP;
		close(fd);
		return -1;
	}

	//The program is loaded and ready to execute
	close(fd);


	process_path = _malloc_r(task_table[0].global_reent, len+1);
	if( process_path == 0 ){
		mcu_debug_user_printf("couldn't alloc path argument in shared mem\n");
		return -1;
	}
	strcpy(process_path, path_arg);

	err = sched_new_process((void*)startup.exec.startup,
			process_path,
			&mem,
			(void*)startup.exec.ram_start);

	mcu_debug_user_printf("process_start:returned %d\n", err);

	return err;
#else
	errno = ENOTSUP;
	return -1;
#endif
}



#if SINGLE_PROCESS == 0
int reent_is_free(struct _reent * reent){
	int i;
	for(i=0; i < task_get_total(); i++){
		if ( (reent == task_table[i].reent) && task_enabled(i) ){
			return 0;
		}
	}
	return 1;
}
#endif


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

#include "crt_common.h"

#include <signal.h>
#include <reent.h>
#include <sys/lock.h>
#include <sys/reent.h>
#include <pthread.h>

#include "mcu/core.h"
#include "cortexm/task.h"

#define SCHED_DEFAULT_STACKGUARD_SIZE 32
extern void (*_ctors)();
extern int _ctors_size;
extern void (*_dtors)();
extern int _dtors_size;
extern int main(int argc, char * const argv[]);

static void constructors();
static void destructors();

int crt_common(char * path_arg, int * ret, const char * name){
	task_setstackguard(pthread_self(), &_ebss, SCHED_DEFAULT_STACKGUARD_SIZE);
	int argc;
	char ** argv;

	//Zero out the BSS section
	memset(&_bss,
			0,
			(uint32_t)((char*)&_ebss - (char*)&_bss)
	);

	_REENT->procmem_base = (proc_mem_t*)&_ebss;
	_REENT->procmem_base->proc_name = name;
	_REENT->procmem_base->size = 0;
	_REENT->procmem_base->sigactions = NULL;
	memset(_REENT->procmem_base->open_file, 0, sizeof(open_file_t)*OPEN_MAX);

	//Initialize the global mutexes
	__lock_init_recursive_global(__malloc_lock_object);
	__lock_init_global(__tz_lock_object);
	__lock_init_recursive_global(__atexit_lock);
	__lock_init_recursive_global(__sfp_lock);
	__lock_init_recursive_global(__sinit_lock);
	__lock_init_recursive_global(__env_lock_object);


	//import argv in to the process memory
	argv = crt_import_argv(path_arg, &argc);

	_REENT->procmem_base->siginfos = malloc(sizeof(siginfo_t)*32);

	//Initialize STDIO
    __sinit(_GLOBAL_REENT);
    write(stdout->_file, 0, 0); //forces stdin, stdout, and stderr to open

	//Execute main
	constructors();
	*ret = main(argc, argv);
	destructors();

	return 0;
}

void constructors(){
	int i;
	for(i=0; i < (int)&_ctors_size; i++){
		(&_ctors)[i]();
	}
}
void destructors(){
	int i;
	for(i=0; i < (int)&_dtors_size; i++){
		(&_dtors)[i]();
	}
}




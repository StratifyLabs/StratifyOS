/* Copyright 2011-2017 Tyler Gilbert;
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


#include <stdlib.h>
#include <sos/fs/sysfs.h>
#include <string.h>

#include "../scheduler/scheduler_local.h"
#include "mcu/core.h"
#include "cortexm/mpu.h"
#include "cortexm/task.h"
#include "mcu/debug.h"

typedef struct {
	int code_size;
	int data_size;
} root_load_data_t;

static void svcall_load_data(void * args) MCU_ROOT_EXEC_CODE;
void svcall_load_data(void * args){
	CORTEXM_SVCALL_ENTER();

	root_load_data_t * p = args;
	int size;
	void * code_addr;
	u32 code_size;
	void * src_addr;
	void * dest_addr;

	//sanity check the size
	size =  sos_task_table[ task_get_current() ].mem.data.size;
	if( p->data_size < size ){ //p->data_size is the total amount of RAM available to the application
		size = p->data_size;
	}

	code_size = p->code_size;
	if( code_size > sos_task_table[ task_get_current() ].mem.code.size ){
		//fatal error here?
		return;
	}

	dest_addr = sos_task_table[ task_get_current() ].mem.data.address;
	code_addr = sos_task_table[ task_get_current() ].mem.code.address;
	code_size = p->code_size;
	src_addr = (u8*)code_addr + code_size;
	memcpy(dest_addr, src_addr, size);
}

//global_reent but it can't be removed without spinning the application signature
void crt_load_data(void * global_reent, int code_size, int data_size){
	root_load_data_t args;
	args.code_size = code_size;
	args.data_size = data_size;
	cortexm_svcall(svcall_load_data, &args);
}

char ** const crt_import_argv(char * path_arg, int * argc){
	char ** argv;
	char * arg_buffer;
	char * p;
	char * next;

	int count;
	int len;

	*argc = 0;

	if( path_arg == 0 ){
		return 0;
	}

	//this needs to be strnlen -- security
	len = strnlen(path_arg,255) + 1;

	count = 0;
	next = strtok_r(path_arg, sysfs_whitespace, &p);
	while( next ){
		next = strtok_r(0, sysfs_whitespace, &p);
		count++;
	}

	u32 argv_size = sizeof(char*)*(count+1);
	argv = malloc(argv_size + len);
	if( argv == 0 ){
		//since we couldn't allocate memory in the application, free the memory allocated on global
		if( path_arg ){
			_free_r(sos_task_table[0].global_reent, path_arg);
		}
		return 0;
	}

	arg_buffer = ((char*)argv) + argv_size;
	memcpy(arg_buffer, path_arg, len);
	arg_buffer[len-1]=0; //len includes the zero terminator -- need to zero terminate just in case

	//free the path_arg passed from shared system memory
	_free_r(sos_task_table[0].global_reent, path_arg);

	count = 0;
	argv[count++] = arg_buffer;
	int is_next = 0;
	for(u32 i=0; i < len-1; i++){
		if( isspace(arg_buffer[i]) ){
			arg_buffer[i] = 0;
		}
		if( arg_buffer[i] == 0 ){
			is_next = 1;
		} else if( is_next && arg_buffer[i] ){
			argv[count++] = arg_buffer + i;
			is_next = 0;
		}
	}

	*argc = count;

	return argv;
}


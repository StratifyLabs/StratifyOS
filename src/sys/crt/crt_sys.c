/*
 * crt_sys.c
 *
 *  Created on: Jan 23, 2014
 *      Author: tgil
 */


#include <stdlib.h>
#include <sos/fs/sysfs.h>
#include <string.h>

#include "../sched/sched_flags.h"
#include "mcu/core.h"
#include "mcu/mpu.h"
#include "mcu/task.h"
#include "mcu/debug.h"

typedef struct {
	int code_size;
	int data_size;
} priv_load_data_t;

static void priv_load_data(void * args) MCU_PRIV_EXEC_CODE;
void priv_load_data(void * args){

	priv_load_data_t * p = args;
	int size;
	void * code_addr;
	uint32_t code_size;
	void * src_addr;
	void * dest_addr;

	//sanity check the size
	size =  mpu_size((uint32_t)task_table[ task_get_current() ].mem.data.size);
	if( p->data_size < size ){
		size = p->data_size;
	}

	code_size = p->code_size;
	if( code_size > mpu_size((uint32_t)task_table[ task_get_current() ].mem.code.size) ){
		return;
	}

	dest_addr = mpu_addr((uint32_t)task_table[ task_get_current() ].mem.data.addr);
	code_addr = mpu_addr((uint32_t)task_table[ task_get_current() ].mem.code.addr);
	code_size = p->code_size;
	src_addr = code_addr + code_size;
	memcpy(dest_addr, src_addr, size);
}

void crt_load_data(void * global_reent, int code_size, int data_size){
	priv_load_data_t args;
	args.code_size = code_size;
	args.data_size = data_size;
	mcu_core_privcall(priv_load_data, &args);
}

char ** const crt_import_argv(char * path_arg, int * argc){
	char ** argv;
	char * arg_buffer;
	char * p;
	char * next;

	int count;
	int len;

	if( path_arg == 0 ){
		*argc = 0;
		return 0;
	}

	len = strlen(path_arg);


	arg_buffer = malloc(len);
	if( arg_buffer == 0 ){
		_free_r(task_table[0].global_reent, path_arg);
		return 0;
	}

	strcpy(arg_buffer, path_arg);

	count = 0;
	next = strtok_r(path_arg, sysfs_whitespace, &p);
	while( next ){
		next = strtok_r(0, sysfs_whitespace, &p);
		count++;
	}

	//free the path_arg passed from shared system memory
	_free_r(task_table[0].global_reent, path_arg);

	argv = malloc(sizeof(char*)*count);
	if( argv == 0 ){
		return 0;
	}

	count = 0;
	argv[0] = strtok_r(arg_buffer, sysfs_whitespace, &p);
	while(argv[count] != 0 ){
		count++;
		argv[count] = strtok_r(0, sysfs_whitespace, &p);
	}

	*argc = count;
	return argv;
}


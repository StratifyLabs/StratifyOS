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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * 
 */

#include <cortex_m/task_local.h>
#include "mcu/mcu.h"
#include "cortexm/mpu.h"
#include "mcu/debug.h"
#include "mcu/core.h"
#include "device/sys.h"

static int init_os_memory_protection(task_memories_t * os_mem);
int task_mpu_calc_protection(task_memories_t * mem);


int task_init_mpu(void * system_memory, int system_memory_size){
	task_memories_t os_mem;
	int err;

	//Calculate the device specific memory protection regions
	mpu_dev_init();

	//Memory Protection
	os_mem.code.addr = &_text;
	os_mem.code.size = (char*)&_etext - (char*)&_text;
	os_mem.data.addr = system_memory;
	os_mem.data.size = system_memory_size;
	err = init_os_memory_protection(&os_mem);
	if ( err < 0 ){
		return err;
	}

	//Calculate the memory protection scheme for the OS memory
	err = task_mpu_calc_protection(&os_mem);  //This has to be after init_os_memory_protection() because it modifies os_mem
	if ( err < 0 ){
		return err;
	}
	memcpy(&(task_table[0].mem), &os_mem, sizeof(os_mem));  //Copy the OS mem to the task table

	//Turn the MPU On
	mpu_enable();

	return 0;
}

typedef struct {
	int tid;
	void * stackaddr;
	int stacksize;
} priv_setstackguard_t;

static void priv_setstackguard(void * arg){
	priv_setstackguard_t * p = arg;
	p->tid = task_root_set_stackguard(p->tid, p->stackaddr, p->stacksize);
}

int task_setstackguard(int tid, void * stackaddr, int stacksize){
	priv_setstackguard_t arg;
	arg.tid = tid;
	arg.stackaddr = stackaddr;
	arg.stacksize = stacksize;
	cortexm_svcall(priv_setstackguard, &arg);
	return arg.tid;
}

int task_root_set_stackguard(int tid, void * stackaddr, int stacksize){
	int err;
	uint32_t newaddr;
	uint32_t rbar;
	uint32_t rasr;

	if ( (uint32_t)tid < task_get_total() ){

		newaddr = (uint32_t)stackaddr;
		newaddr = (newaddr & ~(stacksize - 1)) + stacksize;


		err = mpu_calc_region(
				2,
				(void*)newaddr,
				stacksize,
				MPU_ACCESS_PRW,
				MPU_MEMORY_SRAM,
				false,
				&rbar,
				&rasr);

		if ( err ){
			return err;
		}

	} else {
		return -1;
	}

	task_table[tid].mem.stackguard.addr = (void*)rbar;
	task_table[tid].mem.stackguard.size = rasr;

	if ( tid == task_get_current() ){
		MPU->RBAR = rbar;
		MPU->RASR = rasr;
	}

	return 0;
}


int init_os_memory_protection(task_memories_t * os_mem){
	int err;

	err = mpu_dev_init();
	if ( err < 0 ){
		return err;
	}

	//Make OS System memory read-only -- region 0 -- highest priority
	err = mpu_enable_region(
			0,
			&_sys,
			(char*)&_esys - (char*)&_sys,
			MPU_ACCESS_PRW_UR,
			MPU_MEMORY_SRAM,
			false
	);
	if ( err < 0 ){
		return err;
	}


	//Make the OS flash executable and readable -- region 3
	err = mpu_enable_region(
			3,
			os_mem->code.addr,
			os_mem->code.size,
			MPU_ACCESS_PR_UR,
			MPU_MEMORY_FLASH,
			true
	);
	if ( err < 0 ){
		return err;
	}

	//Make the OS shared memory R/W -- region 5
	err = mpu_enable_region(
			5,
			os_mem->data.addr,
			os_mem->data.size,
			MPU_ACCESS_PRW_URW,
			MPU_MEMORY_SRAM,
			false
	);
	if ( err < 0 ){
		return err;
	}

	return 0;

}

int task_mpu_calc_protection(task_memories_t * mem){
	int err;
	mpu_memory_t mem_type;
	uint32_t rasr;
	uint32_t rbar;

	if ( mem->code.addr < (void*)&_data ){
		mem_type = MPU_MEMORY_FLASH;
	} else {
		mem_type = MPU_MEMORY_SRAM;
	}


	//Region 6
	err = mpu_calc_region(
			6,
			mem->code.addr,
			mem->code.size,
			MPU_ACCESS_PR_UR,
			mem_type,
			true,
			&rbar,
			&rasr
	);

	if ( err < 0 ){
		//mcu_debug_user_printf("bad code 0x%X 0x%X %d\n", mem->code.addr, mem->code.size, err);
		return err;
	}

	mem->code.addr = (void*)rbar;
	mem->code.size = rasr;

	//Region 7
	err = mpu_calc_region(
			7,
			mem->data.addr,
			mem->data.size,
			MPU_ACCESS_PRW_URW,
			MPU_MEMORY_SRAM,
			false,
			&rbar,
			&rasr
	);

	if ( err < 0 ){
		//mcu_debug_user_printf("bad data 0x%X 0x%X %d\n", mem->data.addr, mem->data.size, err);
		return err;
	}

	mem->data.addr = (void*)rbar;
	mem->data.size = rasr;

	return 0;
}

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

#include "task_local.h"
#include "cortexm/mpu.h"

//returns 1 if target and size fit in task_memory
static int is_part_of_memory(void * target, int size, volatile task_memory_t * task_memory);

static int init_os_memory_protection(task_memories_t * os_mem);
int task_mpu_calc_protection(task_memories_t * mem);

int task_validate_memory(void * target, int size){

	//most likely
	if( is_part_of_memory(target, size, &sos_task_table[task_get_current()].mem.data) ){
		return 0;
	}

	//next most likely
	if( is_part_of_memory(target, size, &sos_task_table[task_get_current()].mem.code) ){
		return 0;
	}

	//part of shared kernel memory?
	if( is_part_of_memory(target, size, &sos_task_table[0].mem.data) ){
		return 0;
	}

	//part of kernel code (const data may be read)
	if( is_part_of_memory(target, size, &sos_task_table[0].mem.code) ){
		return 0;
	}


	//target and size overflow the memory
	return -1;
}

int is_part_of_memory(void * target, int size, volatile task_memory_t * task_memory){
	u32 task_address = (u32)mpu_addr((u32)task_memory->addr);
	u32 task_size = mpu_size(task_memory->size);
	u32 target_address = (u32)target;
	u32 target_size = (u32)size;
	if( (target_address >= task_address) && (target_address+target_size <= task_address + task_size) ){
		return 1;
	}
	return 0;
}

int task_init_mpu(void * system_memory, int system_memory_size){
	task_memories_t os_mem;
	int err;

	//Calculate the device specific memory protection regions
	mpu_dev_init();

	u32 text_start = (u32)&_text & ~mcu_board_config.os_mpu_text_mask;

	//Memory Protection
	os_mem.code.addr = (void*)text_start;
	os_mem.code.size = (char*)&_etext - (char*)text_start;
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
	memcpy((void*)&(sos_task_table[0].mem), &os_mem, sizeof(os_mem));  //Copy the OS mem to the task table

	//Turn the MPU On
	mpu_enable();

	return 0;
}

typedef struct {
	int tid;
	void * stackaddr;
	int stacksize;
} root_setstackguard_t;

static void root_setstackguard(void * arg){
	root_setstackguard_t * p = arg;
	p->tid = task_root_set_stackguard(p->tid, p->stackaddr, p->stacksize);
}

int task_setstackguard(int tid, void * stackaddr, int stacksize){
	root_setstackguard_t arg;
	arg.tid = tid;
	arg.stackaddr = stackaddr;
	arg.stacksize = stacksize;
	cortexm_svcall(root_setstackguard, &arg);
	return arg.tid;
}

int task_root_set_stackguard(int tid, void * stackaddr, int stacksize){
	int err;
	u32 newaddr;
	u32 rbar;
	u32 rasr;

	if ( (u32)tid < task_get_total() ){

		newaddr = (u32)stackaddr;
		newaddr = (newaddr & ~(stacksize - 1)) + stacksize;

		err = mpu_calc_region(
					TASK_APPLICATION_STACK_GUARD_REGION,
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

	sos_task_table[tid].mem.stackguard.addr = (void*)rbar;
	sos_task_table[tid].mem.stackguard.size = rasr;

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
				TASK_SYSTEM_STACK_MPU_REGION,
				&_sys,
				(char*)&_esys - (char*)&_sys,
				MPU_ACCESS_PRW_UR,
				MPU_MEMORY_SRAM,
				0
				);
	if ( err < 0 ){
		mcu_debug_log_error(MCU_DEBUG_SYS, "Failed to init OS read-only 0x%lX to 0x%lX (%d)", (u32)&_sys, (u32)&_esys, err);
		return err;
	}


	//Make the OS flash executable and readable -- region 3
	err = mpu_enable_region(
				TASK_SYSTEM_CODE_MPU_REGION,
				os_mem->code.addr,
				os_mem->code.size,
				MPU_ACCESS_PR_UR,
				MPU_MEMORY_FLASH,
				1
				);
	if ( err < 0 ){
		mcu_debug_log_error(MCU_DEBUG_SYS, "Failed to init OS flash 0x%lX -> 0x%ld bytes (%d)", (u32)os_mem->code.addr, (u32)os_mem->code.size, err);
		return err;
	}

	//Make the OS shared memory R/W -- region 5
	err = mpu_enable_region(
				TASK_SYSTEM_DATA_MPU_REGION,
				os_mem->data.addr,
				os_mem->data.size,
				MPU_ACCESS_PRW_URW,
				MPU_MEMORY_SRAM,
				0
				);
	if ( err < 0 ){
		mcu_debug_log_error(MCU_DEBUG_SYS, "Failed to init shared mem 0x%lX -> 0x%lX bytes (%d)", (u32)os_mem->data.addr, (u32)os_mem->data.size, err);
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
				TASK_APPLICATION_CODE_MPU_REGION,
				mem->code.addr,
				mem->code.size,
				MPU_ACCESS_PR_UR,
				mem_type,
				true,
				&rbar,
				&rasr
				);

	if ( err < 0 ){
		return err;
	}

	mem->code.addr = (void*)rbar;
	mem->code.size = rasr;

	//Region 7
	err = mpu_calc_region(
				TASK_APPLICATION_DATA_MPU_REGION,
				mem->data.addr,
				mem->data.size,
				MPU_ACCESS_PRW_URW,
				MPU_MEMORY_SRAM,
				false,
				&rbar,
				&rasr
				);

	if ( err < 0 ){
		return err;
	}

	mem->data.addr = (void*)rbar;
	mem->data.size = rasr;

	return 0;
}

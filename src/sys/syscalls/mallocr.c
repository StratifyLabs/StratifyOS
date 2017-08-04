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

#include "cortexm/cortexm.h"
#include "sos/sos.h"
#include <sys/unistd.h>
#include <unistd.h>

#include "config.h"
#include "malloc_local.h"
#include "mcu/debug.h"
#include "mcu/core.h"
#include "trace.h"


static void set_last_chunk(malloc_chunk_t * chunk);
static void cleanup_memory(struct _reent * reent_ptr);
static int get_more_memory(struct _reent * reent_ptr);
static malloc_chunk_t * find_free_chunk(malloc_chunk_t * chunk, u32 num_chunks);
static int is_memory_corrupt(struct _reent * reent_ptr);


void malloc_process_fault(void * loc);

u16 malloc_calc_num_chunks(u32 size){
	int num_chunks;
	if ( size > MALLOC_DATA_SIZE ){
		num_chunks = ((size - MALLOC_DATA_SIZE) + MALLOC_CHUNK_SIZE - 1) / MALLOC_CHUNK_SIZE + 1;
	} else {
		num_chunks = 1;
	}
	return num_chunks;
}

malloc_chunk_t * find_free_chunk(malloc_chunk_t * chunk, u32 num_chunks){
	int loop_count = 0;
	int is_free;

	while( chunk->header.num_chunks != 0 ){
		is_free = malloc_chunk_is_free(chunk);

		if ( is_free == -1 ){
			return NULL;
		}

		if ( (is_free == 1)  && ( chunk->header.num_chunks >= num_chunks ) ){
			return chunk;
		}
		loop_count++;
		chunk = chunk + chunk->header.num_chunks;
	}


	//No block found to fit size
	//mcu_debug_user_printf("No chunk to fit (0x%X %d 0x%X)\n", chunk, chunk->header.num_chunks, chunk->signature);
	return NULL;
}

int is_memory_corrupt(struct _reent * reent_ptr){
	int is_free;
	malloc_chunk_t * chunk = (malloc_chunk_t *)&(reent_ptr->procmem_base->base);

	while( chunk->header.num_chunks != 0 ){
		is_free = malloc_chunk_is_free(chunk);
		if ( is_free == -1 ){
			return -1;
		}
		chunk += chunk->header.num_chunks;
	}
	return 0;
}

void cleanup_memory(struct _reent * reent_ptr){
	malloc_chunk_t * current;
	malloc_chunk_t * next;
	int next_free;
	int current_free;
	current = (malloc_chunk_t*)&(reent_ptr->procmem_base->base);
	next = current + (current->header.num_chunks);
	while( next->header.num_chunks != 0 ){
		current_free = malloc_chunk_is_free(current);
		//mcu_debug_user_printf("CC:0x%X %d %d\n", (int)current, current->header.num_chunks, current_free);
		next_free = malloc_chunk_is_free(next);
		//mcu_debug_user_printf("NC:0x%X %d %d\n", (int)next, next->header.num_chunks, next_free);

		if ( next_free == -1 ){
			return;
		}

		if ( (current_free == 1) &&
				(next_free == 1) ){ //both blocks are free
			malloc_set_chunk_free(current, current->header.num_chunks + next->header.num_chunks);
		} else {
			current = next;
		}
		next = next + next->header.num_chunks;
	}
}

malloc_chunk_t * malloc_chunk_from_addr(void * addr){
	malloc_chunk_t * chunk;
	chunk = (malloc_chunk_t *)((char*)addr - (sizeof(malloc_chunk_t) - MALLOC_DATA_SIZE));
	return chunk;
}

void malloc_free_task_r(struct _reent * reent_ptr, int task_id){
	malloc_chunk_t * chunk;
	malloc_chunk_t * next;
	if ( reent_ptr->procmem_base == NULL ){
		return;
	}

	chunk = (malloc_chunk_t *) &(reent_ptr->procmem_base->base);

	while( chunk->header.num_chunks != 0 ){
		next = chunk + chunk->header.num_chunks;
		if ( chunk->header.task_id == task_id ){
			_free_r(reent_ptr, chunk->memory);
		}
		chunk = next;
	}
}



void _free_r(struct _reent * reent_ptr, void * addr){
	int tmp;

	malloc_chunk_t * chunk;
	int is_free;
	proc_mem_t * base;
	void * b;

	if ( reent_ptr->procmem_base == NULL ){
		return;
	}

	if ( addr == NULL ){
		return;
	}

	chunk = malloc_chunk_from_addr(addr);

	//sanity check the chunk
	base = reent_ptr->procmem_base;

	b = &(reent_ptr->procmem_base->base);
	//sanity check for chunk that is not in proc mem (low side)
	if( addr < b ){
		return;
	}

	if( addr > b + reent_ptr->procmem_base->size ){
		return;
	}

	__malloc_lock(reent_ptr);
	//check for corrupt memory
	if( is_memory_corrupt(reent_ptr) < 0 ){
		return;
	}

	tmp = (unsigned int)chunk - (unsigned int)(&(base->base));
	if ( tmp % MALLOC_CHUNK_SIZE ){
		__malloc_unlock(reent_ptr);
		return;
	}

	is_free = malloc_chunk_is_free(chunk);

	if ( is_free != 0 ){  //Is the chunk in use (able to be freed)
		//This is not a valid memory allocation location
		__malloc_unlock(reent_ptr);
		return;
	}

	//mcu_debug_user_printf("f:%d 0x%X\n", _getpid(), addr);
	malloc_set_chunk_free(chunk, chunk->header.num_chunks);
	cleanup_memory(reent_ptr);
	__malloc_unlock(reent_ptr);
}


int get_more_memory(struct _reent * reent_ptr){
	malloc_chunk_t * chunk;
	chunk = (malloc_chunk_t *)_sbrk_r(reent_ptr, MALLOC_SBRK_JUMP_SIZE);
	if ( chunk == NULL ){
		return -1;
	} else {
		malloc_set_chunk_free(chunk, MALLOC_SBRK_JUMP_SIZE / MALLOC_CHUNK_SIZE);
		//mcu_debug_user_printf("MC:0x%X (%d)\n", (int)chunk, chunk->header.num_chunks);
		set_last_chunk(chunk + chunk->header.num_chunks); //mark the last block
		//mcu_debug_user_printf("LC:%d:0x%X\n", chunk_is_free(chunk + chunk->header.num_chunks), chunk + chunk->header.num_chunks);
	}
	return 0;
}

void * _malloc_r(struct _reent * reent_ptr, size_t size){
	void * alloc;
	u16 num_chunks;
	malloc_chunk_t * chunk;
	malloc_chunk_t * next;
	alloc = NULL;

	if ( reent_ptr == NULL ){
		errno = EINVAL;
		return NULL;
	}

	__malloc_lock(reent_ptr);


	if ( reent_ptr->procmem_base->size == 0 ){
		if ( get_more_memory(reent_ptr) < 0 ){
			__malloc_unlock(reent_ptr);
			errno = ENOMEM;
			return NULL;
		}
	}

	num_chunks = malloc_calc_num_chunks(size);

	//Find a free chunk that fits size -- add memory using get_more_memory() as necessary
	do {

		chunk = find_free_chunk((malloc_chunk_t *) &(reent_ptr->procmem_base->base), num_chunks);
		if ( chunk == NULL ){

			//See if the memory is corrupt
			if ( is_memory_corrupt(reent_ptr) ){
				malloc_process_fault(reent_ptr); //this will exit the process
				__malloc_unlock(reent_ptr);
				errno = ENOMEM;
				return NULL;
			}

			//Try to get more memory
			if ( get_more_memory(reent_ptr) < 0 ){
				__malloc_unlock(reent_ptr);
				errno = ENOMEM;
				return NULL;
			}

			//Cleanup the memory
			cleanup_memory(reent_ptr);

		} else {

			//See if the memory will fit in this chunk
			if ( chunk->header.num_chunks > num_chunks ){
				next = chunk + (num_chunks);
				malloc_set_chunk_free(next, (chunk->header.num_chunks) - num_chunks);
			} else if ( chunk->header.num_chunks < num_chunks ){
				__malloc_unlock(reent_ptr);
				errno = ENOMEM;
				return NULL;
			}
			malloc_set_chunk_used(reent_ptr, chunk, num_chunks, size);
			alloc = chunk->memory;
		}
	} while(alloc == NULL);

	__malloc_unlock(reent_ptr);

	//mcu_debug_user_printf("a:%d 0x%X %d 0x%X 0x%X\n", _getpid(), alloc, size, reent_ptr, _GLOBAL_REENT);


	return alloc;
}

void malloc_set_chunk_used(struct _reent * reent, malloc_chunk_t * chunk, u16 num_chunks, u32 actual_size){
	chunk->header.num_chunks = num_chunks;
	chunk->header.actual_size = actual_size;
	//chunk->signature = ~(num_chunks ^ actual_size);
	if( (reent == _REENT) && ( task_isthread_asserted( task_get_current() )) ){
		chunk->header.task_id = task_get_current();
	} else {
		chunk->header.task_id = 0;
	}
	cortexm_assign_zero_sum32(chunk, CORTEXM_ZERO_SUM32_COUNT(malloc_chunk_header_t));
}

void malloc_set_chunk_free(malloc_chunk_t * chunk, u16 free_chunks){
	chunk->header.task_id = task_get_current();
	chunk->header.actual_size = 0;
	chunk->header.num_chunks = free_chunks;
	cortexm_assign_zero_sum32(chunk, CORTEXM_ZERO_SUM32_COUNT(malloc_chunk_header_t));
	//chunk->signature = ~(chunk->header.num_chunks ^ 0);
}

void set_last_chunk(malloc_chunk_t * chunk){
	chunk->header.task_id = task_get_current();
	chunk->header.num_chunks = 0;
	chunk->header.actual_size = 0;
	cortexm_assign_zero_sum32(chunk, CORTEXM_ZERO_SUM32_COUNT(malloc_chunk_header_t));
	//chunk->signature = ~(0 ^ 0);
}


int malloc_chunk_is_free(malloc_chunk_t * chunk){
	if( cortexm_verify_zero_sum32(chunk, CORTEXM_ZERO_SUM32_COUNT(malloc_chunk_header_t)) == 0){
		//This chunk is corrupt
		mcu_debug_user_printf("me:%d %d %ld 0x%08lX\n",
				chunk->header.task_id,
				chunk->header.num_chunks,
				chunk->header.actual_size,
				chunk->header.checksum);
		malloc_process_fault(chunk);
		return -1;
	}

	if ( chunk->header.actual_size == 0 ){ //Is the chunk free
		return 1;
	}

	//Chunk is in use
	return 0;
}

void malloc_process_fault(void * loc){
	SOS_TRACE_MESSAGE("Heap Fault");
	mcu_debug_user_printf("\nHeap Fault\n");

	if( task_get_pid(task_get_current()) > 0 ){
		//free the heap and reset the stack
		_exit(1);
	} else {
		mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_FATAL, (void*)"malloc");
	}

}


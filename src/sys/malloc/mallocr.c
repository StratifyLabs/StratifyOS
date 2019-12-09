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


#include <sys/unistd.h>
#include <unistd.h>
#include "mcu/types.h"
#include "sys/malloc/malloc_local.h"
#include "cortexm/cortexm.h"
#include "sos/sos.h"

#include "config.h"
#include "mcu/mcu.h"
#include "mcu/debug.h"
#include "mcu/core.h"
#include "trace.h"

static void set_last_chunk(malloc_chunk_t * chunk);
static void cleanup_memory(struct _reent * reent_ptr, int release_extra_memory);
static int get_more_memory(struct _reent * reent_ptr, u32 size, int is_new_heap);
static malloc_chunk_t * find_free_chunk(struct _reent * reent_ptr, u32 num_chunks);
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

malloc_chunk_t * find_free_chunk(struct _reent * reent_ptr, u32 num_chunks){
	int loop_count = 0;
	int is_free;
	malloc_chunk_t * chunk = (malloc_chunk_t *) &(reent_ptr->procmem_base->base);

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

void cleanup_memory(struct _reent * reent_ptr, int release_extra_memory){
	malloc_chunk_t * current;
	malloc_chunk_t * next;
	int next_free;
	int current_free;
	malloc_chunk_t * last_chunk_if_free = 0;
	current = (malloc_chunk_t*)&(reent_ptr->procmem_base->base);
	next = current + (current->header.num_chunks);
	//if num_chunks is zero -- that is the last chunk
	while( next->header.num_chunks != 0 ){
		current_free = malloc_chunk_is_free(current);
		next_free = malloc_chunk_is_free(next);

		if( next_free ){
			last_chunk_if_free = next;
		} else {
			last_chunk_if_free = 0;
		}

		if ( next_free == -1 ){
			return;
		}

		if ( (current_free == 1) && (next_free == 1) ){ //both blocks are free
			//combine the free chunks as one larger free chunk
			malloc_set_chunk_free(current, current->header.num_chunks + next->header.num_chunks);
		} else {
			current = next;
		}
		next = next + next->header.num_chunks;
	}

	//next->header.num_chunks is 0 -- next and current are the same
	if( release_extra_memory && (last_chunk_if_free != 0) ){
		//do negative _sbrk to give memory back to stack
		ptrdiff_t size = -1*(last_chunk_if_free->header.num_chunks * MALLOC_CHUNK_SIZE);
		_sbrk_r(reent_ptr, size);
		set_last_chunk(last_chunk_if_free);
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

	if( addr == NULL ){
		//take no action if addr is null
		return;
	}

	if ( reent_ptr->procmem_base == NULL ){
		return;
	}


	if ( addr == (void*)1 ){
		//special case to tell system to give memory back to the stack
		__malloc_lock(reent_ptr);
		cleanup_memory(reent_ptr, 1);
		__malloc_unlock(reent_ptr);
		return;
	}

	chunk = malloc_chunk_from_addr(addr);

	//sanity check the chunk
	base = reent_ptr->procmem_base;

	b = &(reent_ptr->procmem_base->base);
	//sanity check for chunk that is not in proc mem (low side)
	if( addr < b ){
		sos_trace_stack(32);
		mcu_debug_log_warning(
					MCU_DEBUG_MALLOC,
					"Free addr below heap %p < %p (id:%d)",
					addr,
					b,
					task_get_current()
					);
		//mcu_debug_trace_stack();
		return;
	}

	if( (u32)addr > (u32)b + reent_ptr->procmem_base->size ){
		mcu_debug_log_warning(
					MCU_DEBUG_MALLOC,
					"Free addr above heap %p > %p (id:%d)",
					addr,
					b,
					task_get_current()
					);
		return;
	}

	__malloc_lock(reent_ptr);
	//check for corrupt memory
	if( is_memory_corrupt(reent_ptr) < 0 ){
		mcu_debug_log_error(MCU_DEBUG_MALLOC, "Free Memory Corrupt 0x%lX", (u32)reent_ptr);
		SOS_TRACE_CRITICAL("Heap Fault");
		__malloc_unlock(reent_ptr);
		malloc_process_fault(reent_ptr); //this will exit the process
		return;
	}

	tmp = (unsigned int)chunk - (unsigned int)(&(base->base));
	if ( tmp % MALLOC_CHUNK_SIZE ){
		mcu_debug_log_warning(MCU_DEBUG_MALLOC, "Free addr not aligned");
		__malloc_unlock(reent_ptr);
		return;
	}

	is_free = malloc_chunk_is_free(chunk);

	if ( is_free != 0 ){  //Is the chunk in use (able to be freed)
		//This is not a valid memory allocation location
		mcu_debug_log_warning(MCU_DEBUG_MALLOC, "f:%d 0x%X is already free", getpid(), addr);
		__malloc_unlock(reent_ptr);
		return;
	}

	//mcu_debug_log_info(MCU_DEBUG_MALLOC, "f:%d 0x%X", getpid(), addr);
	malloc_set_chunk_free(chunk, chunk->header.num_chunks);
	cleanup_memory(reent_ptr, 0);

	mcu_debug_log_info(MCU_DEBUG_MALLOC, "f:%d %p %p %p", getpid(), addr, reent_ptr, _GLOBAL_REENT);


	__malloc_unlock(reent_ptr);
}


int get_more_memory(struct _reent * reent_ptr, u32 size, int is_new_heap){
	malloc_chunk_t * chunk;
	void * new_heap = 0;
	int extra_bytes = 0;

	if( is_new_heap ){
		extra_bytes = MALLOC_SBRK_JUMP_SIZE;
	}

	//jump as size but round up to a multiple of MALLOC_SBRK_JUMP_SIZE
	int jump_size = ((size + MALLOC_SBRK_JUMP_SIZE - 1) / MALLOC_SBRK_JUMP_SIZE) * MALLOC_SBRK_JUMP_SIZE;
	new_heap = _sbrk_r(reent_ptr, jump_size + extra_bytes);
	if ( new_heap == NULL ){
		//this means _sbrk_r was unable to allocate the requested memory due to a potential collision with the stack
		return -1;
	} else {
		if( is_new_heap ){
			chunk = new_heap;
		} else {
			/*
			 * After the first call, there is always an extra MALLOC_SBRK_JUMP_SIZE bytes on the heap
			 * that needs to be adjusted for. This extra MALLOC_SBRK_JUMP_SIZE leaves room to always
			 * have room for the last chunk header. That is, without the extra MALLOC_SBRK_JUMP_SIZE,
			 * the heap could run into the stack guard protected memory and crash the program.
			 *
			 */
			chunk = new_heap - MALLOC_SBRK_JUMP_SIZE;
		}
		malloc_set_chunk_free(chunk, jump_size / MALLOC_CHUNK_SIZE);
		set_last_chunk(chunk + chunk->header.num_chunks); //mark the last block (heap should have extra room for this)
	}
	return 0;
}

void * _malloc_r(struct _reent * reent_ptr, size_t size){
	void * alloc;
	u16 num_chunks;
	malloc_chunk_t * chunk;
	malloc_chunk_t * next;
	alloc = NULL;

	mcu_debug_log_info(MCU_DEBUG_MALLOC, "%s():%d->", __FUNCTION__, __LINE__);

	if ( reent_ptr == NULL ){
		errno = EINVAL;
		mcu_debug_log_info(MCU_DEBUG_MALLOC, "EINVAL %s():%d<-", __FUNCTION__, __LINE__);
		return NULL;
	}

	num_chunks = malloc_calc_num_chunks(size);

	__malloc_lock(reent_ptr);

	if ( reent_ptr->procmem_base->size == 0 ){
		mcu_debug_log_info(MCU_DEBUG_MALLOC, "Get more memory");
		if ( get_more_memory(reent_ptr, size, 1) < 0 ){
			__malloc_unlock(reent_ptr);
			errno = ENOMEM;
			mcu_debug_log_info(MCU_DEBUG_MALLOC, "ENOMEM %s():%d<-", __FUNCTION__, __LINE__);
			return NULL;
		}
	}

	//Find a free chunk that fits size -- add memory using get_more_memory() as necessary
	do {

		chunk = find_free_chunk(reent_ptr, num_chunks);
		if ( chunk == NULL ){

			//See if the memory is corrupt
			if ( is_memory_corrupt(reent_ptr) ){
				mcu_debug_log_error(MCU_DEBUG_MALLOC, "Memory Corrupt %p", reent_ptr);
				SOS_TRACE_CRITICAL("Heap Fault");
				__malloc_unlock(reent_ptr); //unlock in case it is shared memory
				malloc_process_fault(reent_ptr); //this will exit the process
				errno = ENOMEM;
				mcu_debug_log_info(MCU_DEBUG_MALLOC, "ENOMEM %s():%d<-", __FUNCTION__, __LINE__);
				return NULL;
			}

			//Try to get more memory
			if ( get_more_memory(reent_ptr, size, 0) < 0 ){
				cleanup_memory(reent_ptr, 0); //give memory back to stack
				__malloc_unlock(reent_ptr);
				errno = ENOMEM;
				mcu_debug_log_info(MCU_DEBUG_MALLOC, "ENOMEM %s():%d<-", __FUNCTION__, __LINE__);
				return NULL;
			}

			//Cleanup the memory
			cleanup_memory(reent_ptr, 0);

		} else {

			//See if the memory will fit in this chunk
			if ( chunk->header.num_chunks > num_chunks ){
				next = chunk + (num_chunks);
				malloc_set_chunk_free(next, (chunk->header.num_chunks) - num_chunks);
			} else if ( chunk->header.num_chunks < num_chunks ){
				__malloc_unlock(reent_ptr);
				errno = ENOMEM;
				mcu_debug_log_info(MCU_DEBUG_MALLOC, "ENOMEM %s():%d<-", __FUNCTION__, __LINE__);
				return NULL;
			}
			malloc_set_chunk_used(reent_ptr, chunk, num_chunks, size);
			alloc = chunk->memory;
		}
	} while(alloc == NULL);

	__malloc_unlock(reent_ptr);

	mcu_debug_log_info(MCU_DEBUG_MALLOC, "a:%d,%d %p %d (%d) %p %p<-", getpid(), task_get_current(), alloc, size, num_chunks*MALLOC_CHUNK_SIZE, reent_ptr, _GLOBAL_REENT);


	return alloc;
}

void malloc_set_chunk_used(struct _reent * reent, malloc_chunk_t * chunk, u16 num_chunks, u32 actual_size){
	chunk->header.num_chunks = num_chunks;
	chunk->header.actual_size = actual_size;
	if( (reent == _REENT) && ( task_thread_asserted( task_get_current() )) ){
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
}

void set_last_chunk(malloc_chunk_t * chunk){
	chunk->header.task_id = task_get_current();
	chunk->header.num_chunks = 0;
	chunk->header.actual_size = 0;
	cortexm_assign_zero_sum32(chunk, CORTEXM_ZERO_SUM32_COUNT(malloc_chunk_header_t));
}

//Returns 1 if free and zero if in use -- -1 for a corrupt chunk
int malloc_chunk_is_free(malloc_chunk_t * chunk){
	if( cortexm_verify_zero_sum32(chunk, CORTEXM_ZERO_SUM32_COUNT(malloc_chunk_header_t)) == 0){
		//This chunk is corrupt
		mcu_debug_log_error(MCU_DEBUG_MALLOC, "Corrupt Chunk 0x%lX", (u32)chunk);
		SOS_TRACE_CRITICAL("Heap Corrupt");
		malloc_process_fault(((void*)chunk) + 1);
		return -1;
	}

	if ( chunk->header.actual_size == 0 ){ //Is the chunk free
		return 1;
	}

	//Chunk is in use
	return 0;
}

void malloc_process_fault(void * loc){
	mcu_debug_log_error(
				MCU_DEBUG_SYS,
				"%Heap: 0x%lX (id:%d)",
				(u32)loc,
				task_get_current()
				);

	if( task_get_pid(task_get_current()) > 0 ){
		//free the heap and reset the stack
		_exit(1);
	} else {
		mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_FATAL, (void*)"malloc");
	}

}



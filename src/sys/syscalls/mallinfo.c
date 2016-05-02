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



#include "malloc_local.h"

struct mallinfo mallinfo(void){
	return _mallinfo_r(_REENT);
}

struct mallinfo _mallinfo_r(struct _reent * reent_ptr){
	int total_chunks;
	int total_used_memory;
	int total_free_chunks;
	int chunk_free;
	struct mallinfo mi;
	malloc_chunk_t * chunk;
	proc_mem_t * procmem_base;

	mi.arena = 0;
	mi.ordblks = 0;
	mi.fordblks = 0;
	mi.uordblks = 0;
	if ( reent_ptr->procmem_base == NULL ){
		return mi;
	}

	procmem_base = (proc_mem_t *)reent_ptr->procmem_base;
	chunk = (malloc_chunk_t *)&(procmem_base->base);

	total_chunks = 0;
	total_used_memory = 0;
	total_free_chunks = 0;

	while( chunk->num_chunks != 0 ){
		chunk_free = malloc_chunk_is_free(chunk);
		if ( chunk_free == -1 ){
			return mi;
		}
		total_chunks += chunk->num_chunks;
		if ( chunk_free == 1 ){
			total_free_chunks += chunk->num_chunks;
		} else {
			total_used_memory += chunk->actual_size;
		}
		chunk += chunk->num_chunks;
	}

	mi.arena = (total_chunks) * MALLOC_CHUNK_SIZE + (sizeof(malloc_chunk_t) - MALLOC_DATA_SIZE);
	mi.ordblks = total_free_chunks;
	mi.fordblks = total_free_chunks * MALLOC_CHUNK_SIZE;
	mi.uordblks = total_used_memory;
	return mi;

}

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

#include <sys/malloc/malloc_local.h>

void * _realloc_r(struct _reent * reent_ptr, void * addr, size_t size){
	u16 num_chunks;
	u16 free_chunks;
	malloc_chunk_t * chunk;
	malloc_chunk_t * next;
	void * alloc;

	if ( reent_ptr == NULL ){
		errno = EINVAL;
		return NULL;
	}

	if ( size == 0 ){
		if ( addr != NULL ){
			free(addr);
		}
		return NULL;
	}


	if ( addr == NULL ){
		return _malloc_r(reent_ptr, size);
	}

	__malloc_lock(reent_ptr);


	//Check to see if there is memory allocated already
	if ( reent_ptr->procmem_base->size == 0 ){

		if ( addr != NULL ){ //since base has not been initialized, this cannot be valid
			__malloc_unlock(reent_ptr);
			errno = ENOMEM;
			return NULL;
		}

	} else {

		num_chunks = malloc_calc_num_chunks(size);
		//Check to see if current memory can be resized
		chunk = malloc_chunk_from_addr(addr);
		if ( malloc_chunk_is_free(chunk) != 0 ){ //chunk is either corrupt or free
			errno = EINVAL;
			__malloc_unlock(reent_ptr);
			return NULL;
		}

		if ( num_chunks <= chunk->header.num_chunks ){ //check to see if new chunk count is less than or equal to current count
			free_chunks = chunk->header.num_chunks - num_chunks;
			malloc_set_chunk_used(reent_ptr, chunk, num_chunks, size);
			next = chunk + num_chunks;
			if( free_chunks != 0 ){
				malloc_set_chunk_free(next, free_chunks);
			}
			__malloc_unlock(reent_ptr);
			return addr;
		}

		next = chunk + chunk->header.num_chunks;

		if ( malloc_chunk_is_free(next) == 1 ){ //The next chunk is free
			free_chunks = next->header.num_chunks + chunk->header.num_chunks;
			if ( free_chunks < num_chunks ){
				malloc_set_chunk_used(reent_ptr, chunk, num_chunks, size);
				next = chunk + chunk->header.num_chunks;
				malloc_set_chunk_free(next, num_chunks - free_chunks);
				__malloc_unlock(reent_ptr);
				return addr;
			} else if ( free_chunks == num_chunks ){
				malloc_set_chunk_used(reent_ptr, chunk, num_chunks, size);
				__malloc_unlock(reent_ptr);
				return addr;
			}
		}
	}

	__malloc_unlock(reent_ptr);

	alloc = _malloc_r(reent_ptr, size);

	if ( alloc != NULL ){
		_free_r(reent_ptr, addr);
	}

	return alloc;
}

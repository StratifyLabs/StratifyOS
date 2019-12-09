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

#ifndef MALLOC_LOCAL_H_
#define MALLOC_LOCAL_H_


#include "config.h"
#include <_ansi.h>
#include <reent.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include "mcu/mcu.h"
#include "cortexm/task.h"


typedef struct MCU_PACK {
	u16 task_id;
	u16 num_chunks;
	u32 actual_size;
	u32 checksum;
} malloc_chunk_header_t;

#define MALLOC_DATA_SIZE (MALLOC_CHUNK_SIZE - (sizeof(malloc_chunk_header_t)))

typedef struct {
	malloc_chunk_header_t header;
	char memory[MALLOC_DATA_SIZE];
} malloc_chunk_t;

void malloc_set_chunk_used(struct _reent * reent, malloc_chunk_t * chunk, u16 num_chunks, u32 actual_size);
void malloc_set_chunk_free(malloc_chunk_t * chunk, u16 num_chunks);
int malloc_chunk_is_free(malloc_chunk_t * chunk);
u16 malloc_calc_num_chunks(u32 size);
malloc_chunk_t * malloc_chunk_from_addr(void * addr);

void malloc_free_task_r(struct _reent * reent_ptr, int task_id);

void __malloc_lock(struct _reent *ptr);
void __malloc_unlock(struct _reent *ptr);

#endif /* MALLOC_LOCAL_H_ */

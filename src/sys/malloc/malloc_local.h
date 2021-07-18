// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef MALLOC_LOCAL_H_
#define MALLOC_LOCAL_H_

#include "config.h"
#include <_ansi.h>
#include <errno.h>
#include <malloc.h>
#include <reent.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cortexm/task.h"

typedef struct MCU_PACK {
  u16 task_id;
  u16 num_chunks;
  u32 actual_size;
  u32 checksum;
} malloc_chunk_header_t;

#define MALLOC_DATA_SIZE (CONFIG_MALLOC_CHUNK_SIZE - (sizeof(malloc_chunk_header_t)))

typedef struct {
  malloc_chunk_header_t header;
  char memory[MALLOC_DATA_SIZE];
} malloc_chunk_t;

void malloc_set_chunk_used(
  struct _reent *reent,
  malloc_chunk_t *chunk,
  u16 num_chunks,
  u32 actual_size);
void malloc_set_chunk_free(malloc_chunk_t *chunk, u16 num_chunks);
int malloc_chunk_is_free(malloc_chunk_t *chunk);
u16 malloc_calc_num_chunks(u32 size);
malloc_chunk_t *malloc_chunk_from_addr(void *addr);

int malloc_get_more_memory(struct _reent *reent_ptr, u32 size, int is_new_heap);


void malloc_free_task_r(struct _reent *reent_ptr, int task_id);

void __malloc_lock(struct _reent *ptr);
void __malloc_unlock(struct _reent *ptr);

#endif /* MALLOC_LOCAL_H_ */

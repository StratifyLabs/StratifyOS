// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "sys/malloc/malloc_local.h"

struct mallinfo mallinfo() {
  return _mallinfo_r(_REENT);
}

struct mallinfo _mallinfo_r(struct _reent *reent_ptr) {
  int total_chunks;
  int total_used_memory;
  int total_free_chunks;
  struct mallinfo mi;
  malloc_chunk_t *chunk;
  proc_mem_t *procmem_base;

  mi.arena = 0;
  mi.ordblks = 0;
  mi.fordblks = 0;
  mi.uordblks = 0;
  if (reent_ptr->procmem_base == NULL) {
    return mi;
  }

  procmem_base = (proc_mem_t *)reent_ptr->procmem_base;
  chunk = (malloc_chunk_t *)&(procmem_base->base);

  total_chunks = 0;
  total_used_memory = 0;
  total_free_chunks = 0;

  while (chunk->header.num_chunks != 0) {
    int chunk_free = malloc_chunk_is_free(chunk);
    if (chunk_free == -1) {
      return mi;
    }
    total_chunks += chunk->header.num_chunks;
    if (chunk_free == 1) {
      total_free_chunks += chunk->header.num_chunks;
    } else {
      total_used_memory += chunk->header.actual_size;
    }
    chunk += chunk->header.num_chunks;
  }

  // this calculation may be wrong -- needs to account for the number o header entries
  // Issue #143
  mi.arena = (total_chunks)*MALLOC_CHUNK_SIZE;
  mi.ordblks = total_free_chunks;
  mi.fordblks = total_free_chunks * MALLOC_CHUNK_SIZE;
  mi.uordblks = total_used_memory;

  return mi;
}

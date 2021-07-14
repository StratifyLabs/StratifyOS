// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <sys/malloc/malloc_local.h>

void *_realloc_r(struct _reent *reent_ptr, void *addr, size_t size) {
  malloc_chunk_t *chunk;
  malloc_chunk_t *next;
  void *alloc;

  if (reent_ptr == NULL) {
    errno = EINVAL;
    return NULL;
  }

  if (size == 0) {
    if (addr != NULL) {
      free(addr);
    }
    return NULL;
  }

  if (addr == NULL) {
    return _malloc_r(reent_ptr, size);
  }

  __malloc_lock(reent_ptr);

  // Check to see if there is memory allocated already
  if (reent_ptr->procmem_base->size == 0) {
    __malloc_unlock(reent_ptr);
    errno = ENOMEM;
    return NULL;
  } else {

    u16 num_chunks_requested = malloc_calc_num_chunks(size);
    // Check to see if current memory can be resized
    chunk = malloc_chunk_from_addr(addr);

    // chunk is either corrupt or free
    if (malloc_chunk_is_free(chunk) != 0) {
      errno = EINVAL;
      __malloc_unlock(reent_ptr);
      return NULL;
    }

    // check to see if new chunk count is less than or equal to current count

    if (num_chunks_requested == chunk->header.num_chunks) {
      malloc_set_chunk_used(reent_ptr, chunk, num_chunks_requested, size);
      __malloc_unlock(reent_ptr);
      return addr;
    }

    if (num_chunks_requested < chunk->header.num_chunks) {
      const u16 free_chunks_next = chunk->header.num_chunks - num_chunks_requested;
      malloc_set_chunk_used(reent_ptr, chunk, num_chunks_requested, size);
      next = chunk + num_chunks_requested;
      malloc_set_chunk_free(next, free_chunks_next);
      __malloc_unlock(reent_ptr);
      return addr;
    }

    next = chunk + chunk->header.num_chunks;

    if (next->header.num_chunks > 0 && malloc_chunk_is_free(next) == 1) { // The next
                                                                          // chunk is free
      const u16 free_chunks_with_next =
        next->header.num_chunks + chunk->header.num_chunks;
      if (num_chunks_requested < free_chunks_with_next) {
        malloc_set_chunk_used(reent_ptr, chunk, num_chunks_requested, size);
        next = chunk + chunk->header.num_chunks;
        malloc_set_chunk_free(next, free_chunks_with_next - num_chunks_requested);
        __malloc_unlock(reent_ptr);
        return addr;
      } else if (free_chunks_with_next == num_chunks_requested) {
        malloc_set_chunk_used(reent_ptr, chunk, num_chunks_requested, size);
        __malloc_unlock(reent_ptr);
        return addr;
      }
    }
  }

  __malloc_unlock(reent_ptr);

  alloc = _malloc_r(reent_ptr, size);

  if (alloc != NULL) {
    chunk = malloc_chunk_from_addr(addr);
    const size_t copy_size =
      chunk->header.actual_size < size ? chunk->header.actual_size : size;
    memcpy(alloc, addr, copy_size);
    _free_r(reent_ptr, addr);
  }

  return alloc;
}

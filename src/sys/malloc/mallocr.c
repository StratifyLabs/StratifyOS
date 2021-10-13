// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "cortexm/cortexm.h"
#include "sos/sos.h"
#include "sys/malloc/malloc_local.h"
#include <sdk/types.h>
#include <sys/unistd.h>
#include <unistd.h>

#include "config.h"
#include "sos/debug.h"
#include "trace.h"

#define ENABLE_DEEP_TRACE 0

static void set_last_chunk(malloc_chunk_t *chunk);
static void cleanup_memory(struct _reent *reent_ptr, int release_extra_memory);
static malloc_chunk_t *find_free_chunk(struct _reent *reent_ptr, u32 num_chunks);
static int is_memory_corrupt(struct _reent *reent_ptr);

void malloc_process_fault(void *loc);

u16 malloc_calc_num_chunks(u32 size) {
  int num_chunks;
  if (size > MALLOC_DATA_SIZE) {
    num_chunks = ((size - MALLOC_DATA_SIZE) + CONFIG_MALLOC_CHUNK_SIZE - 1)
                   / CONFIG_MALLOC_CHUNK_SIZE
                 + 1;
  } else {
    num_chunks = 1;
  }
  return num_chunks;
}

malloc_chunk_t *find_free_chunk(struct _reent *reent_ptr, u32 num_chunks) {
  int loop_count = 0;
  malloc_chunk_t *chunk = (malloc_chunk_t *)&(reent_ptr->procmem_base->base);

#if ENABLE_DEEP_TRACE
  sos_debug_log_info(SOS_DEBUG_MALLOC, "-----------find free");
#endif

  while (chunk->header.num_chunks != 0) {
#if ENABLE_DEEP_TRACE
    sos_debug_log_info(
      SOS_DEBUG_MALLOC, "checking chunk %p %d %ld", chunk, chunk->header.num_chunks,
      chunk->header.actual_size);
#endif
    const int is_free = malloc_chunk_is_free(chunk);

    if (is_free == -1) {
      return NULL;
    }

    if ((is_free == 1) && (chunk->header.num_chunks >= num_chunks)) {
      return chunk;
    }
    loop_count++;
    chunk = chunk + chunk->header.num_chunks;
  }

  // No block found to fit size
  return NULL;
}

int is_memory_corrupt(struct _reent *reent_ptr) {
  malloc_chunk_t *chunk = (malloc_chunk_t *)&(reent_ptr->procmem_base->base);

  while (chunk->header.num_chunks != 0) {
    int is_free = malloc_chunk_is_free(chunk);
    if (is_free == -1) {
      return -1;
    }
    chunk += chunk->header.num_chunks;
  }
  return 0;
}

void cleanup_memory(struct _reent *reent_ptr, int release_extra_memory) {
  malloc_chunk_t *current;
  malloc_chunk_t *next;
  malloc_chunk_t *last_chunk_if_free = 0;
  current = (malloc_chunk_t *)&(reent_ptr->procmem_base->base);
  next = current + (current->header.num_chunks);
  // if num_chunks is zero -- that is the last chunk
  while (next->header.num_chunks != 0) {
    int current_free = malloc_chunk_is_free(current);
    int next_free = malloc_chunk_is_free(next);

    if (next_free) {
      last_chunk_if_free = next;
    } else {
      last_chunk_if_free = 0;
    }

    if (next_free == -1) {
      return;
    }

    if ((current_free == 1) && (next_free == 1)) { // both blocks are free
      // combine the free chunks as one larger free chunk
      malloc_set_chunk_free(
        current, current->header.num_chunks + next->header.num_chunks);
    } else {
      current = next;
    }
    next = next + next->header.num_chunks;
  }

  // next->header.num_chunks is 0 -- next and current are the same
  if (release_extra_memory && (last_chunk_if_free != 0)) {
    // do negative _sbrk to give memory back to stack
    ptrdiff_t size =
      -1 * (last_chunk_if_free->header.num_chunks * CONFIG_MALLOC_CHUNK_SIZE);
    _sbrk_r(reent_ptr, size);
    set_last_chunk(last_chunk_if_free);
  }
}

malloc_chunk_t *malloc_chunk_from_addr(void *addr) {
  malloc_chunk_t *chunk;
  chunk = (malloc_chunk_t *)((char *)addr - (sizeof(malloc_chunk_t) - MALLOC_DATA_SIZE));
  return chunk;
}

void malloc_free_task_r(struct _reent *reent_ptr, int task_id) {
  malloc_chunk_t *chunk;
  malloc_chunk_t *next;
  if (reent_ptr->procmem_base == NULL) {
    return;
  }

  chunk = (malloc_chunk_t *)&(reent_ptr->procmem_base->base);

  while (chunk->header.num_chunks != 0) {
    next = chunk + chunk->header.num_chunks;
    if (chunk->header.task_id == task_id) {
      _free_r(reent_ptr, chunk->memory);
    }
    chunk = next;
  }
}

void _free_r(struct _reent *reent_ptr, void *addr) {
  SOS_DEBUG_ENTER_TIMER_SCOPE(_free_r);
  int tmp;

  malloc_chunk_t *chunk;
  int is_free;
  proc_mem_t *base;
  void *b;

  if (addr == NULL) {
    // take no action if addr is null
    return;
  }

  if (reent_ptr->procmem_base->size == 0) {
    return;
  }

  if (addr == (void *)1) {
    // special case to tell system to give memory back to the stack
    __malloc_lock(reent_ptr);
    cleanup_memory(reent_ptr, 1);
    __malloc_unlock(reent_ptr);
    return;
  }

  chunk = malloc_chunk_from_addr(addr);

  // sanity check the chunk
  base = reent_ptr->procmem_base;

  b = &(reent_ptr->procmem_base->base);
  // sanity check for chunk that is not in proc mem (low side)
  if (addr < b) {
    sos_trace_stack((u32)-1);
    sos_debug_log_warning(
      SOS_DEBUG_MALLOC, "Free addr below heap %p < %p (id:%d)", addr, b,
      task_get_current());
    // sos_debug_trace_stack();
    return;
  }

  if ((u32)addr > (u32)b + reent_ptr->procmem_base->size) {
    sos_debug_log_warning(
      SOS_DEBUG_MALLOC, "Free addr above heap %p > %p (id:%d)", addr, b,
      task_get_current());
    return;
  }

  __malloc_lock(reent_ptr);
  // check for corrupt memory
  if (is_memory_corrupt(reent_ptr) < 0) {
    sos_debug_log_error(SOS_DEBUG_MALLOC, "Free Memory Corrupt 0x%lX", (u32)reent_ptr);
    SOS_TRACE_CRITICAL("Heap Fault");
    __malloc_unlock(reent_ptr);
    malloc_process_fault(reent_ptr); // this will exit the process
    return;
  }

  tmp = (unsigned int)chunk - (unsigned int)(&(base->base));
  if (tmp % CONFIG_MALLOC_CHUNK_SIZE) {
    sos_debug_log_warning(SOS_DEBUG_MALLOC, "Free addr not aligned");
    __malloc_unlock(reent_ptr);
    return;
  }

  is_free = malloc_chunk_is_free(chunk);

  if (is_free != 0) { // Is the chunk in use (able to be freed)
    // This is not a valid memory allocation location
    sos_debug_log_warning(SOS_DEBUG_MALLOC, "f:%d 0x%X is already free", getpid(), addr);
    __malloc_unlock(reent_ptr);
    return;
  }

  // sos_debug_log_info(SOS_DEBUG_MALLOC, "f:%d 0x%X", getpid(), addr);
  malloc_set_chunk_free(chunk, chunk->header.num_chunks);
  cleanup_memory(reent_ptr, 0);

  __malloc_unlock(reent_ptr);
  SOS_DEBUG_EXIT_TIMER_SCOPE(SOS_DEBUG_MALLOC, _free_r);

  sos_debug_log_datum(SOS_DEBUG_MALLOC, "heap%d:free,%d", getpid(), addr);
}

int malloc_get_more_memory(struct _reent *reent_ptr, u32 size, int is_new_heap) {
  void *new_heap = 0;
  int extra_bytes = 0;

  if (is_new_heap) {
    extra_bytes = CONFIG_MALLOC_SBRK_JUMP_SIZE;
  }

  // jump as size but round up to a multiple of CONFIG_MALLOC_SBRK_JUMP_SIZE
  int jump_size =
    ((size + CONFIG_MALLOC_SBRK_JUMP_SIZE - 1) / CONFIG_MALLOC_SBRK_JUMP_SIZE)
    * CONFIG_MALLOC_SBRK_JUMP_SIZE;
  new_heap = _sbrk_r(reent_ptr, jump_size + extra_bytes);

  if (new_heap == NULL) {
    sos_debug_log_error(
      SOS_DEBUG_MALLOC, "sbrk has no more memory (tried to jump %ld)",
      jump_size + extra_bytes);
    // this means _sbrk_r was unable to allocate the requested memory due to a potential
    // collision with the stack
    return -1;
  } else {
    malloc_chunk_t *chunk;
    if (is_new_heap) {
      chunk = new_heap;
    } else {
      /*
       * After the first call, there is always an extra CONFIG_MALLOC_SBRK_JUMP_SIZE bytes
       * on the heap that needs to be adjusted for. This extra
       * CONFIG_MALLOC_SBRK_JUMP_SIZE leaves room to always have room for the last chunk
       * header. That is, without the extra CONFIG_MALLOC_SBRK_JUMP_SIZE, the heap could
       * run into the stack guard protected memory and crash the program.
       *
       */
      chunk = new_heap - CONFIG_MALLOC_SBRK_JUMP_SIZE;
    }
    malloc_set_chunk_free(chunk, jump_size / CONFIG_MALLOC_CHUNK_SIZE);
    // mark the last block (heap should have extra room for this)
    set_last_chunk(chunk + chunk->header.num_chunks);
#if ENABLE_DEEP_TRACE
    sos_debug_log_info(
      SOS_DEBUG_MALLOC, "set last chunk at %p", chunk + chunk->header.num_chunks);
#endif
  }
  return 0;
}

int malloc_is_memory_corrupt(struct _reent *reent_ptr) {
  if (reent_ptr == NULL) {
    reent_ptr = _REENT;
  }

  malloc_chunk_t *chunk = (malloc_chunk_t *)&(reent_ptr->procmem_base->base);

  while (chunk->header.num_chunks != 0) {
    int is_free = malloc_chunk_is_free(chunk);
    if (is_free == -1) {
      return 1;
    }
    chunk = chunk + chunk->header.num_chunks;
  }

  // not corrupt
  return 0;
}

void *_malloc_r(struct _reent *reent_ptr, size_t size) {
  void *alloc;
  u16 num_chunks;
  malloc_chunk_t *chunk;
  alloc = NULL;

  SOS_DEBUG_ENTER_TIMER_SCOPE(_malloc_r);

  if (reent_ptr == NULL) {
    errno = EINVAL;
    sos_debug_log_info(SOS_DEBUG_MALLOC, "EINVAL %s():%d<-", __FUNCTION__, __LINE__);
    sos_handle_event(SOS_EVENT_MALLOC_FAILED, "reent_ptr");
    return NULL;
  }

  __malloc_lock(reent_ptr);
  num_chunks = malloc_calc_num_chunks(size);

  if (reent_ptr->procmem_base->size == 0) {
    if (malloc_get_more_memory(reent_ptr, size, 1) < 0) {
      __malloc_unlock(reent_ptr);
      errno = ENOMEM;
      sos_debug_log_info(SOS_DEBUG_MALLOC, "ENOMEM %s():%d<-", __FUNCTION__, __LINE__);
      sos_handle_event(SOS_EVENT_MALLOC_FAILED, "ENOMEM1");
      return NULL;
    }
  }

  // Find a free chunk that fits size -- add memory using get_more_memory() as necessary
  do {

    chunk = find_free_chunk(reent_ptr, num_chunks);
    if (chunk == NULL) {

      // See if the memory is corrupt
      if (is_memory_corrupt(reent_ptr)) {
        sos_debug_log_error(SOS_DEBUG_MALLOC, "Memory Corrupt %p", reent_ptr);
        SOS_TRACE_CRITICAL("Heap Fault");
        __malloc_unlock(reent_ptr);      // unlock in case it is shared memory
        malloc_process_fault(reent_ptr); // this will exit the process
        errno = ENOMEM;
        sos_debug_log_info(SOS_DEBUG_MALLOC, "ENOMEM %s():%d<-", __FUNCTION__, __LINE__);
        sos_handle_event(SOS_EVENT_MALLOC_FAILED, "ENOMEM2");
        return NULL;
      }

      // Try to get more memory
      if (malloc_get_more_memory(reent_ptr, size, 0) < 0) {
        cleanup_memory(reent_ptr, 0); // give memory back to stack
        __malloc_unlock(reent_ptr);
        errno = ENOMEM;
        sos_debug_log_info(SOS_DEBUG_MALLOC, "ENOMEM %s():%d<-", __FUNCTION__, __LINE__);
        sos_handle_event(SOS_EVENT_MALLOC_FAILED, "ENOMEM3");
        return NULL;
      }

      // Cleanup the memory
      cleanup_memory(reent_ptr, 0);

    } else {

      // See if the memory will fit in this chunk
      int diff_chunks = chunk->header.num_chunks - num_chunks;
      if (diff_chunks) {
        malloc_set_chunk_free(chunk + num_chunks, diff_chunks);
      } else if (chunk->header.num_chunks < num_chunks) {
        __malloc_unlock(reent_ptr);
        errno = ENOMEM;
        sos_debug_log_info(SOS_DEBUG_MALLOC, "ENOMEM %s():%d<-", __FUNCTION__, __LINE__);
        sos_handle_event(SOS_EVENT_MALLOC_FAILED, NULL);
        return NULL;
      }
      malloc_set_chunk_used(reent_ptr, chunk, num_chunks, size);
      alloc = chunk->memory;
    }
  } while (alloc == NULL);

  __malloc_unlock(reent_ptr);

  SOS_DEBUG_EXIT_TIMER_SCOPE(SOS_DEBUG_MALLOC, _malloc_r);

  sos_debug_log_datum(SOS_DEBUG_MALLOC, "heap%d:alloc,%d,%d", getpid(), alloc, size);

  return alloc;
}

void malloc_set_chunk_used(
  struct _reent *reent,
  malloc_chunk_t *chunk,
  u16 num_chunks,
  u32 actual_size) {
  chunk->header.num_chunks = num_chunks;
  chunk->header.actual_size = actual_size;
  if ((reent == _REENT) && (task_thread_asserted(task_get_current()))) {
    chunk->header.task_id = task_get_current();
  } else {
    chunk->header.task_id = 0;
  }
  cortexm_assign_zero_sum32(chunk, CORTEXM_ZERO_SUM32_COUNT(malloc_chunk_header_t));
}

void malloc_set_chunk_free(malloc_chunk_t *chunk, u16 free_chunks) {
  chunk->header.task_id = task_get_current();
  chunk->header.actual_size = 0;
  chunk->header.num_chunks = free_chunks;
  cortexm_assign_zero_sum32(chunk, CORTEXM_ZERO_SUM32_COUNT(malloc_chunk_header_t));
}

void set_last_chunk(malloc_chunk_t *chunk) {
  chunk->header.task_id = task_get_current();
  chunk->header.num_chunks = 0;
  chunk->header.actual_size = 0;
  cortexm_assign_zero_sum32(chunk, CORTEXM_ZERO_SUM32_COUNT(malloc_chunk_header_t));
}

// Returns 1 if free and zero if in use -- -1 for a corrupt chunk
int malloc_chunk_is_free(malloc_chunk_t *chunk) {
  if (
    cortexm_verify_zero_sum32(chunk, CORTEXM_ZERO_SUM32_COUNT(malloc_chunk_header_t))
    == 0) {
    // This chunk is corrupt
    sos_debug_log_error(SOS_DEBUG_MALLOC, "Corrupt Chunk 0x%lX", (u32)chunk);
    SOS_TRACE_CRITICAL("Heap Corrupt");
    malloc_process_fault(((u8 *)chunk) + 1);
    return -1;
  }

  if (chunk->header.actual_size == 0) { // Is the chunk free
    return 1;
  }

  // Chunk is in use
  return 0;
}

void malloc_process_fault(void *loc) {
  sos_debug_log_error(
    SOS_DEBUG_SYS, "Heap: 0x%lX (id:%d,pid:%d)", (u32)loc, task_get_current(),
    task_get_pid(task_get_current()));

  sos_trace_stack((u32)-1);
  if (task_get_pid(task_get_current()) > 0) {
    // free the heap and reset the stack
    _exit(1);
  } else {
    sos_handle_event(SOS_EVENT_FATAL, (void *)"malloc");
  }
}

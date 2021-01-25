// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <malloc.h>
#include <pthread.h>
#include <sys/lock.h>

void __malloc_lock(struct _reent *ptr) {
  pthread_mutex_lock(&(ptr->procmem_base->__malloc_lock_object));
}

void __malloc_unlock(struct _reent *ptr) {
  pthread_mutex_unlock(&(ptr->procmem_base->__malloc_lock_object));
}

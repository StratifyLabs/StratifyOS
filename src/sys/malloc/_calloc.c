// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md



#include <reent.h>
#include <malloc.h>
#include <sys/types.h>
#include <string.h>

void * _calloc_r(struct _reent * reent_ptr, size_t s1, size_t s2){
	int size;
	void * alloc;
	size = s1*s2;
	alloc = _malloc_r(reent_ptr, size);
	if ( alloc != NULL ){
		memset(alloc, 0, size);
	}
	return alloc;
}

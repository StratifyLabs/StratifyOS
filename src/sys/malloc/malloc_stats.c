// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "config.h"
#include <stdio.h>
#include <malloc.h>

void malloc_stats(){
	_malloc_stats_r(_REENT);
}

void _malloc_stats_r(struct _reent * reent_ptr){
	struct mallinfo mi;
	mi = _mallinfo_r(reent_ptr);
	iprintf("Total Malloc Memory %d bytes\n", mi.arena);
	iprintf("Total Free Chunks %d\n", mi.ordblks);
	iprintf("Total Free Memory %d bytes\n", mi.fordblks);
	iprintf("Total Used Memory %d bytes\n", mi.uordblks);
}


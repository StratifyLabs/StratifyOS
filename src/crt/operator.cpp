// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include <stdio.h>
#include <stdlib.h>

extern "C" void *operator new(size_t size) { return malloc(size); }

extern "C" void operator delete(void *p) { free(p); }

extern "C" void operator delete(void *p, unsigned int size) { free(p); }

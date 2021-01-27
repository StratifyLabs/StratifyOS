// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#ifndef CRT_COMMON_H_
#define CRT_COMMON_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>

#include "../sys/sysfs/appfs_local.h"


extern uint32_t _text;
extern uint32_t _text_size;
extern uint32_t _data;
extern uint32_t _data_size;
extern uint32_t _bss;
extern uint32_t _ebss;

extern uint32_t _app_ram_size;

void crt_common(char *path_arg, int *ret, const char *name)
  __attribute__((__short_call__));
void crt_exit(int exit_code);

void crt_load_data(void * global_reent, int code_size, int data_size);
char ** const crt_import_argv(const char * path_arg, int * argc);

#endif /* CRT_COMMON_H_ */

/* Copyright 2011-2016 Tyler Gilbert; 
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

#ifndef CRT_COMMON_H_
#define CRT_COMMON_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>

#include "../sys/sysfs/appfs.h"


extern uint32_t _text;
extern uint32_t _text_size;
extern uint32_t _data;
extern uint32_t _data_size;
extern uint32_t _bss;
extern uint32_t _ebss;

extern uint32_t _app_ram_size;

int crt_common(char * path_arg, int * ret, const char * name);
void crt_exit(int exit_code);

void crt_load_data(void * global_reent, int code_size, int data_size);
char ** const crt_import_argv(const char * path_arg, int * argc);

#endif /* CRT_COMMON_H_ */

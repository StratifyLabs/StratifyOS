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

#include "crt_common.h"

extern u32 _signature;
extern u32 _app_ram_size;

void crt(char * path_arg);

void * __dso_handle;

extern struct _reent global_impure_data __attribute__((section(".global_reent")));
struct _reent global_impure_data = _REENT_INIT(global_impure_data);

const appfs_file_t startup_data __attribute__ ((section(".startup"))) = {
		.hdr.name = "",
		.hdr.mode = 0777,
		.exec.startup = crt,
		.exec.code_start = &_text,
		.exec.code_size = (u32)&_text_size,
		.exec.ram_start = &_data,
		.exec.ram_size = (int)&_app_ram_size,
		.exec.data_size = (int)&_data_size,
		.exec.options = APPFS_EXEC_OPTIONS_FLASH,
		.exec.signature = (u32)&_signature
};


void crt(char * path_arg){
	int ret;

	//Copy the heap to RAM -- since the heap copy is in no man's land this needs to be privileged
	crt_load_data(&global_impure_data, startup_data.exec.code_size, startup_data.exec.data_size);

	if ( crt_common(path_arg, &ret, startup_data.hdr.name) != 0 ){
		ret = -1;
	}

	exit(ret);
}


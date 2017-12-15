/* Copyright 2011-2017 Tyler Gilbert;
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


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "sffs_block.h"
#include <sys/sffs/sffs_diag.h>
#include "sffs_dir.h"
#include "sffs_file.h"
#include "sffs_local.h"
#include "sffs_serialno.h"
#include <sys/sffs/sffs_tp.h>

#include "sffs.h"
#include "tests.h"

volatile int finished;
volatile int failed;

#define FS_ERROR_RET 1
#define TP_RET 2
#define SUCCESS_RET 0

//just for testing
extern void show_mem(int addr, int nbyte);
extern int save_filesystem(const char * path);
extern int load_filesystem(const char * path);


static void fail_routine(){
	failed = 1;
	save_filesystem("myfilesystem.fs");
	exit(TP_RET);
}


void diags(){
	const void * cfg;
	sffs_diag_t diag;
	cfg = NULL;
	//sffs_diag_show_eraseable(cfg);
	sffs_diag_get(cfg, &diag);
	sffs_diag_show(&diag);

	if ( finished == 1 ){
		printf("Filesystem saved:  no errors\n");
	} else if ( failed == 1 ){
		printf("Filesystem saved:  test point failure\n");
	} else {
		printf("Failed to complete\n");
	}
}


#define BUFFER_SIZE 16
#define NUM_TESTS 5

open_file_t open_file;
const sffs_config_t ccfg = {
		.open_file = &open_file,
		.dev_cfg = NULL,
		.devfs = (void*)1,
		.name = "disk"
};

int main() {
	sffs_diag_t diag;
	const void * cfg = NULL;
	pid_t child;
	int ret;

	cfg = &ccfg;

	do {
		child = fork();
		if ( child == 0 ){

			sffs_dev_open(cfg);
			finished = 0;
			failed = 0;

			atexit(diags);

			sffs_tp_setfailroutine(fail_routine);

			if ( load_filesystem("myfilesystem.fs") < 0 ){
				if ( sffs_mkfs(cfg) < 0 ){
					printf("failed to mkfs\n");
					exit(FS_ERROR_RET);
				}
			}

			if ( sffs_init(cfg) < 0 ){
				exit(FS_ERROR_RET);
			}


			if ( sffs_diag_scan(cfg) < 0 ){
				printf("Errors with filesystem\n");
				exit(FS_ERROR_RET);
			}


			if ( 1 ){
				if ( test_run(true, false) < 0 ){
					exit(FS_ERROR_RET);
				}
			}

			sffs_diag_get(cfg, &diag);
			sffs_diag_show(&diag);

			if ( sffs_diag_scan(cfg) < 0 ){
				printf("Errors with filesystem\n");
				exit(FS_ERROR_RET);
			}

			if ( save_filesystem("myfilesystem.fs") < 0 ){
				printf("failed to save filesystem\n");
			}

			finished = 1;

			sffs_tp_createreport("failreport.txt");

			exit(SUCCESS_RET);
		}

		wait(&ret);

		ret = WEXITSTATUS(ret);

		//exit(0);

	} while( (ret == SUCCESS_RET) || (ret == TP_RET) );

	return 0;
}

void * test_open(const char * path, int flags, int mode){
	void * handle;
	if ( sffs_open(NULL, &handle, path, flags, mode) < 0 ){
		return NULL;
	}
	return handle;
}

int test_close(void * handle){
	int ret;
	ret = sffs_close(NULL, &handle);
	return ret;
}

int test_read(void * handle, int loc, void * buf, int nbyte){
	return sffs_read(NULL, handle, 0, loc, buf, nbyte);
}

int test_write(void * handle, int loc, const void * buf, int nbyte){
	return sffs_write(NULL, handle, 0, loc, buf, nbyte);
}

void * test_opendir(const char * path){
	void * handle;
	if ( sffs_opendir(NULL, &handle, path) < 0 ){
		return NULL;
	}
	return handle;
}

int test_readdir_r(void * handle, int loc, struct dirent * entry){
	return sffs_readdir_r(NULL, handle, loc, entry);
}

int test_closedir(void * handle){
	return sffs_closedir(NULL, handle);
}
int test_fstat(void * handle, struct stat * stat){
	return sffs_fstat(NULL, handle, stat);
}

int test_remove(const char * path){
	return sffs_remove(NULL, path);
}

int test_unlink(const char * path){
	return sffs_unlink(NULL, path);
}

int test_stat(const char * path, struct stat * stat){
	return sffs_stat(NULL, path, stat);
}

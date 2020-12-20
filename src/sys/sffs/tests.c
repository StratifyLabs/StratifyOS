// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/sffs/sffs_diag.h>

#include "sffs.h"
#include "tests.h"

#define NUM_DIR_TESTS 5
#define NUM_RW_TESTS 26
#define NUM_LONG_RW_TESTS 5

#define BUFFER_SIZE 16
#define LONG_BUFFER_SIZE 1024



int test_run(bool file_test, bool dir_test){

	if( file_test == true ){
		if ( test_file() < 0 ){
			printf("File test failed\n");
			return -1;
		}
	}

	if( dir_test == true ){
		if ( test_file() < 0 ){
			printf("Dir test failed\n");
			return -1;
		}
	}

	return 0;

}


int test_dir(){
	struct dirent entry;
	void * handle;
	int i;
	char buffer[64];


	if ( (handle = test_opendir("")) == NULL ){
		printf("failed to opendir()\n");
		return -1;
	}

	i = 0;
	while( test_readdir_r(handle, i, &entry) == 0 ){
		sprintf(buffer, "test%d", i);
		printf("dir: %d %s\n", (int)entry.d_ino, entry.d_name);
		if ( strcmp(buffer, entry.d_name) != 0 ){
			printf("Unexpected directory entry %s\n", entry.d_name);
		}
		i++;
	}

	if ( test_readdir_r(handle, i, &entry) == 0 ){
		printf("Failed to remove all directories\n");
	}


	if ( test_closedir(handle) ){
		return -1;
	}

	return 0;
}



int test_listdir(const char * path){
	void * handle;
	struct dirent entry;
	int i;
	if ( (handle = test_opendir(path)) == NULL ){
		printf("failed to opendir()\n");
		return -1;
	}

	i = 0;
	while( test_readdir_r(handle, i, &entry) == 0 ){
		printf("dir: %d %s\n", (int)entry.d_ino, entry.d_name);
		i++;
	}

	test_closedir(&handle);
	return 0;

}



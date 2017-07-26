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

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "tests.h"

#define NUM_DIR_TESTS 5
#define NUM_RW_TESTS 26
#define NUM_LONG_RW_TESTS 5

#define BUFFER_SIZE 16
#define LONG_BUFFER_SIZE 1024


static int stress_test1();
static int stress_test2();
static int rw_test(const char * file, int buffer_size, int num_tests, int open_flags, int open_mode);
static int wr_test(const char * file, int buffer_size, int num_tests, int open_flags, int open_mode);
static int test_rw_trunc(const char * file);
static int test_rw_long_trunc(const char * file);
static int test_rw_short_trunc(const char * file);
static int test_wr_trunc(const char * file);
static int test_wr_long_trunc(const char * file);
static int test_wr_short_trunc(const char * file);

static int test_rw_exist(const char * file);
static int test_rw_long_exist(const char * file);
static int test_rw_short_exist(const char * file);
static int test_wr_exist(const char * file);
static int test_wr_long_exist(const char * file);
static int test_wr_short_exist(const char * file);


static int open_test();
static int run_remove_test(const char * path, bool exists);
static void * run_open_test(const char * path, int flags, int mode, int expected_errno, const char * condition);
static int run_close_test(void * f, int expected_errno, const char * condition);

int test_file(){
	int i;
	char buffer[32];


	if ( open_test() < 0 ){
		printf("Open test failed\n");
		return -1;
	}

	if ( stress_test1() < 0 ){
		printf("stress test 1 failed\n");
		return -1;
	}

	if ( stress_test2() < 0 ){
		printf("stress test 2 failed\n");
		return -1;
	}

	for(i=0; i < NUM_RW_TESTS; i++){
		sprintf(buffer, "file%d.txt", i);
		if ( test_rw_trunc(buffer) < 0 ){
			return -1;
		}
	}

	for(i=0; i < NUM_RW_TESTS; i++){
		sprintf(buffer, "file_long%d.txt", i);
		if ( test_rw_long_trunc(buffer) < 0 ){
			return -1;
		}
	}

	for(i=0; i < NUM_RW_TESTS; i++){
		sprintf(buffer, "file_short%d.txt", i);
		if ( test_rw_short_trunc(buffer) < 0 ){
			return -1;
		}
	}

	return 0;

	for(i=0; i < NUM_RW_TESTS; i++){
		sprintf(buffer, "file%d.txt", i);
		if ( test_rw_exist(buffer) < 0 ){
			return -1;
		}
	}

	for(i=0; i < NUM_RW_TESTS; i++){
		sprintf(buffer, "file_long%d.txt", i);
		if ( test_rw_long_exist(buffer) < 0 ){
			return -1;
		}
	}

	for(i=0; i < NUM_RW_TESTS; i++){
		sprintf(buffer, "file_short%d.txt", i);
		if ( test_rw_short_exist(buffer) < 0 ){
			return -1;
		}
	}

	for(i=0; i < NUM_RW_TESTS; i++){
		sprintf(buffer, "wfile%d.txt", i);
		if ( test_wr_trunc(buffer) < 0 ){
			return -1;
		}
	}

	for(i=0; i < NUM_RW_TESTS; i++){
		sprintf(buffer, "wfile_long%d.txt", i);
		if ( test_wr_long_trunc(buffer) < 0 ){
			return -1;
		}
	}

	for(i=0; i < NUM_RW_TESTS; i++){
		sprintf(buffer, "wfile_short%d.txt", i);
		if ( test_wr_short_trunc(buffer) < 0 ){
			return -1;
		}
	}

	for(i=0; i < NUM_RW_TESTS; i++){
		sprintf(buffer, "wfile%d.txt", i);
		if ( test_wr_exist(buffer) < 0 ){
			return -1;
		}
	}

	for(i=0; i < NUM_RW_TESTS; i++){
		sprintf(buffer, "wfile_long%d.txt", i);
		if ( test_wr_long_exist(buffer) < 0 ){
			return -1;
		}
	}

	for(i=0; i < NUM_RW_TESTS; i++){
		sprintf(buffer, "wfile_short%d.txt", i);
		if ( test_wr_short_exist(buffer) < 0 ){
			return -1;
		}
	}

	return 0;
}

int open_test(){
	void * f;
	char filename[256];

	strcpy(filename, "testfile0.txt");

	//test all types of open combinations
	if ( run_remove_test(filename, false) < 0 ){
		return -1;
	}

	f = run_open_test(filename, O_CREAT | O_RDWR, 0777, 0, "create new");
	if ( f == NULL ){
		return -1;
	}

	if ( run_close_test(f, 0, "") < 0 ){
		return -1;
	}

	f = run_open_test(filename, O_CREAT | O_RDWR | O_EXCL, 0777, EEXIST, "EEXIST");
	if ( f != NULL ){
		return -1;
	}

	f = run_open_test("0123456789", O_RDWR, 0777, ENOENT, "ENOENT");
	if ( f != NULL ){
		return -1;
	}

	f = run_open_test("", O_RDWR, 0777, ENOENT, "ENOENT (empty string)");
	if ( f != NULL ){
		return -1;
	}

	f = run_open_test(filename, O_RDWR, 0777, 0, "read/write existing");
	if ( f == NULL ){
		return -1;
	}

	if ( run_close_test(f, 0, "") < 0 ){
		return -1;
	}

	if ( run_remove_test(filename, true) < 0 ){
		return -1;
	}

	return 0;
}

void * run_open_test(const char * path, int flags, int mode, int expected_errno, const char * condition){
	void * f;
	printf("testing open()...");
	printf("%s", condition);
	printf("...");
	fflush(stdout);

	f = test_open(path, flags, mode);
	if ( f == NULL ){
		if ( expected_errno == 0 ){
			fflush(stdout);
			perror("unexpected error");
			return NULL;
		} else if ( errno == expected_errno ){
			printf("passed\n");
			return NULL;
		} else {
			printf("should have failed with %d not %d\n", expected_errno, errno);
			return NULL;
		}
	} else {
		if ( expected_errno == 0 ){
			printf("passed\n");
			return f;
		} else {
			printf("should have failed with %d not %d\n", expected_errno, errno);
			return f;
		}
	}

	return NULL;
}

int run_close_test(void * f, int expected_errno, const char * condition){
	int err;
	printf("testing close()...");
	printf("%s", condition);
	printf("...");
	err = test_close(f);
	if ( err < 0 ){
		if ( expected_errno == 0 ){
			fflush(stdout);
			perror("unexpected error");
			return -1;
		} else if ( errno == expected_errno ){
			printf("passed\n");
			return -1;
		}
	} else {
		if ( expected_errno == 0 ){
			printf("passed\n");
			return 0;
		} else {
			printf("should have failed with %d\n", expected_errno);
			return 0;
		}
	}


	return -1;
}

int run_remove_test(const char * path, bool exists){
	printf("testing remove()...");

	if ( test_remove(path) < 0 ){
		if ( (errno == ENOENT) && (exists == false) ){
			printf("does not exist...passed\n");
		} else {
			fflush(stdout);
			perror("unexpected error");
			return -1;
		}
	} else {
		printf("passed\n");
	}

	return 0;
}


int test_rw_trunc(const char * file){
	return rw_test(file, BUFFER_SIZE, NUM_RW_TESTS, O_CREAT | O_TRUNC | O_RDWR, 0777);
}

int test_rw_long_trunc(const char * file){
	return rw_test(file, LONG_BUFFER_SIZE, NUM_LONG_RW_TESTS, O_CREAT | O_TRUNC | O_RDWR, 0777);
}

int test_rw_short_trunc(const char * file){
	return rw_test(file, 7, NUM_RW_TESTS, O_CREAT | O_TRUNC | O_RDWR, 0777);
}

int test_rw_exist(const char * file){
	return rw_test(file, BUFFER_SIZE, NUM_RW_TESTS, O_RDWR, 0777);
}

int test_rw_long_exist(const char * file){
	return rw_test(file, LONG_BUFFER_SIZE, NUM_LONG_RW_TESTS, O_RDWR, 0777);
}

int test_rw_short_exist(const char * file){
	return rw_test(file, 7, NUM_RW_TESTS, O_RDWR, 0777);
}

int test_wr_trunc(const char * file){
	return wr_test(file, BUFFER_SIZE, NUM_RW_TESTS, O_CREAT | O_TRUNC, 0777);
}

int test_wr_long_trunc(const char * file){
	return wr_test(file, LONG_BUFFER_SIZE, NUM_LONG_RW_TESTS, O_CREAT | O_TRUNC, 0777);
}

int test_wr_short_trunc(const char * file){
	return wr_test(file, 7, NUM_RW_TESTS, O_CREAT | O_TRUNC, 0777);
}

int test_wr_exist(const char * file){
	return wr_test(file, BUFFER_SIZE, NUM_RW_TESTS, 0, 0777);
}

int test_wr_long_exist(const char * file){
	return wr_test(file, LONG_BUFFER_SIZE, NUM_LONG_RW_TESTS, 0, 0777);
}

int test_wr_short_exist(const char * file){
	return wr_test(file, 7, NUM_RW_TESTS, 0, 0777);
}

int wr_test(const char * file, int buffer_size, int num_tests, int open_flags, int open_mode){
	void * file1;
	char buffer[buffer_size];
	int i,j,k;

	file1 = run_open_test(file, open_flags | O_WRONLY, open_mode, 0, "");
	if ( file1 == NULL ){
		return -1;
	}


	printf("testing write...");
	fflush(stdout);
	for(k=0; k < num_tests; k++){
		printf("%d...", k+1);
		fflush(stdout);
		for(i=0; i < num_tests; i++){
			memset(buffer, (char)('a' + i), buffer_size);
			buffer[buffer_size-1] = 0;

			if ( test_write(file1, 0 + i*buffer_size, buffer, buffer_size) != buffer_size ){
				printf("-------------write failed\n");
				return -1;
			}
		}
	}
	printf("passed\n");

	if ( run_close_test(file1, 0, "") < 0 ){
		return -1;
	}

	file1 = run_open_test(file, O_RDONLY, open_mode, 0, "");
	if ( file1 == NULL ){
		return -1;
	}

	printf("testing read...");
	fflush(stdout);
	for(k=0; k < num_tests; k++){
		printf("%d...", k+1);
		fflush(stdout);

		for(i=0; i < num_tests; i++){
			memset(buffer, 0, buffer_size);
			if ( test_read(file1, 0 + i*buffer_size, buffer, buffer_size) < 0 ){
				printf("Read failed\n");
				return -1;
			}

			for(j=0; j < buffer_size-1; j++){
				if ( buffer[j] != (char)('a' + i) ){
					printf("Read/write mismatch (%d) %d %d\n", j, buffer[j], (char)('a' + i));
					return -1;
				}
			}
			buffer[buffer_size-1] = 0;
		}
	}
	printf("passed\n");

	if ( run_close_test(file1, 0, "") < 0 ){
		return -1;
	}

	return 0;
}

int rw_test(const char * file, int buffer_size, int num_tests, int open_flags, int open_mode){
	void * file1;
	char buffer[buffer_size];
	int i,j,k;

	file1 = run_open_test(file, open_flags, open_mode, 0, "");
	if ( file1 == NULL ){
		return -1;
	}


	printf("testing read/write...");
	fflush(stdout);
	for(k=0; k < num_tests; k++){
		printf("%d...", k+1);
		fflush(stdout);
		for(i=0; i < num_tests; i++){
			memset(buffer, (char)('a' + i), buffer_size);
			buffer[buffer_size-1] = 0;

			if ( test_write(file1, 0 + i*buffer_size, buffer, buffer_size) != buffer_size ){
				printf("-------------write failed\n");
				return -1;
			}
		}


		for(i=0; i < num_tests; i++){
			memset(buffer, 0, buffer_size);
			if ( test_read(file1, 0 + i*buffer_size, buffer, buffer_size) < 0 ){
				printf("Read failed\n");
				return -1;
			}

			for(j=0; j < buffer_size-1; j++){

				if ( buffer[j] != (char)('a' + i) ){
					printf("Read/write mismatch %d %d\n", buffer[j], (char)('a' + i));
					return -1;
				}
			}
			buffer[buffer_size-1] = 0;
		}

		for(i=0; i < num_tests; i++){

			memset(buffer, (char)('A' + i), buffer_size);
			buffer[buffer_size-1] = 0;

			if ( test_write(file1, 0 + i*buffer_size, buffer, buffer_size) != buffer_size ){
				printf("write failed\n");
				return -1;
			}
		}


		for(i=0; i < num_tests; i++){
			memset(buffer, 0, buffer_size);
			if ( test_read(file1, 0 + i*buffer_size, buffer, buffer_size) < 0 ){
				printf("Read failed\n");
				return -1;
			}

			for(j=0; j < buffer_size-1; j++){

				if ( buffer[j] != (char)('A' + i) ){
					printf("2 Read/write mismatch %d %d\n", buffer[j], (char)('A' + i));
					return -1;
				}
			}
			buffer[buffer_size-1] = 0;
		}
	}
	printf("passed\n");

	if ( run_close_test(file1, 0, "") < 0 ){
		return -1;
	}


	return 0;
}

int stress_test1(){
	int i;
	int j;
	int ret;
	int len;
	struct stat st;
	char buffer[64];
	int loc;

	void * f;
	i = 0;

	test_remove("log.txt");

	printf("append test %d...", i);
	fflush(stdout);
	for(i = 0; i < 1000; i++){
		if ( i % 50 == 0 ){
			printf("%d...", i);
			fflush(stdout);
		}

		f = test_open("log.txt", O_APPEND | O_RDWR | O_CREAT, 0666);
		if ( f == NULL ){
			perror("Failed to open file");
			return -1;
		}

		test_fstat(f, &st);
		loc = st.st_size;

		strcpy(buffer, "1234567890123456789012345678901234567890123456789\n");
		len = strlen(buffer);
		ret = test_write(f, loc, buffer, len);
		if ( ret != strlen(buffer) ){
			printf("Ret is %d == %d\n", ret, len);
		}
		if ( ret != strlen(buffer) ){
			perror("write failed");
			test_close(f);
			break;
		}

		fstat(fileno(f), &st);
		test_close(f);

		fflush(stdout);
		f = test_open("log.txt", O_RDONLY, 0);
		if ( f == NULL ){
			perror("Failed to open file");
			return -1;
		}

		loc = 0;

		j = 0;
		errno = 0;
		while(1){

			if ( test_read(f, loc, buffer, len) != len ){
				break;
			}

			loc += len;
			if ( strcmp(buffer, "1234567890123456789012345678901234567890123456789\n") != 0 ){
				printf("Bad comparison at line %d %d\n", j, (int)st.st_size);
				exit(1);
			}
			j++;
		}
		test_fstat(f, &st);
		test_close(f);
	}
	printf("passed\n");

	return 0;
}

int stress_test2(){
	int i;
	struct stat st;
	char buffer[64];
	int len;
	int loc;
	void * f;

	printf("Starting stess test 2\n");
	printf("appending...");
	fflush(stdout);
	f = test_open("log.txt", O_APPEND | O_RDWR | O_CREAT, 0666);
	if ( f == NULL ){
		perror("Failed to open file");
		return -1;
	}

	test_fstat(f, &st);
	loc = st.st_size;

	strcpy(buffer, "123456789\n");
	len = strlen(buffer);

	for(i = 0; i < 10000; i++){
		if ( i % 100 == 0 ){
			printf(".");
			fflush(stdout);
		}

		if ( test_write(f, loc, buffer, len) != len ){
			perror("write failed");
			break;
		}

		loc += len;
	}
	test_fstat(f, &st);
	printf("%d bytes\n", (int)st.st_size);
	test_close(f);
	return 0;
}


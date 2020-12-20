// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#ifndef TESTS_H_
#define TESTS_H_

#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>


int test_run(bool file_test, bool dir_test);

int test_dir();
int test_listdir(const char * path);
int test_file();

int test_rw(const char * file);
int test_rw_long(const char * file);
int test_rw_short(const char * file);




extern int test_close(void * handle);
extern void * test_opendir(const char * path);
extern int test_readdir_r(void * handle, int loc, struct dirent * entry);
extern int test_closedir(void * handle);
extern int test_fstat(void * handle, struct stat * stat);
extern void * test_open(const char * path, int flags, int mode);
extern int test_read(void * handle, int loc, void * buf, int nbyte);
extern int test_write(void * handle, int loc, const void * buf, int nbyte);
extern int test_close(void * handle);
extern int test_remove(const char * path);
extern int test_unlink(const char * path);
extern int test_stat(const char * path, struct stat * stat);


#endif /* TESTS_H_ */

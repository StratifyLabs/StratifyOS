// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md




#ifndef SFFS_DIR_H_
#define SFFS_DIR_H_

#include "../sffs/sffs_local.h"

typedef struct {
	serial_t serialno;
} sffs_dir_lookup_t;

/*
 *
 * Currently, there is only one directly. The root directly.
 *
 * The root directly has a list of serial numbers that are stored.
 * When a file is looked up, each item in the directory serial
 * number is traversed. Each entry contains the status
 * of the entry, a serial number, and a block. The block contains
 * the file header and the first segment of file data.
 *
 *
 *
 *
 */

#define SFFS_DIR_PARENT_EXISTS 1
#define SFFS_DIR_PATH_EXISTS 2

int sffs_dir_exists(const void * cfg, const char * path, sffs_dir_lookup_t * dest, int amode);
int sffs_dir_lookup(const void * cfg, const char * path, sffs_dir_lookup_t * dest, int amode);


#endif /* SFFS_DIR_H_ */

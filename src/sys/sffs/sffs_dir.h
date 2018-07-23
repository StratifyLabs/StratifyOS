/* Copyright 2011-2018 Tyler Gilbert; 
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

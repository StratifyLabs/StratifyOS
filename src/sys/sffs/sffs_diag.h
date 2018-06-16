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



#ifndef SFFS_DIAG_H_
#define SFFS_DIAG_H_


#include "sffs_local.h"

typedef struct {
	int free_blocks;
	int allocated_blocks;
	int used_blocks;
	int dirty_blocks;
	int eraseable_blocks;
	int total_blocks;
	int dir_list_blocks;
	int file_data_blocks;
	int serialno_list_blocks;
	int del_serialno_list_blocks;
	int file_list_blocks;
} sffs_diag_t;



typedef struct {
	int list_blocks;
	int dirty_blocks;
	int used_blocks;
	int allocated_blocks;
	int total_blocks;
} sffs_diag_file_t;


int sffs_diag_scan(const void * cfg);
int sffs_diag_get(const void * cfg, sffs_diag_t * dest);
void sffs_diag_show(sffs_diag_t * data);

int sffs_diag_getfile(const void * cfg, const char * path, sffs_diag_file_t * dest);
void sffs_diag_showfile(sffs_diag_file_t * data);

int sffs_diag_show_eraseable(const void * cfg);

int sffs_diag_scandir(const void * cfg, serial_t serialno);
int sffs_diag_scanfile(const void * cfg, serial_t serialno);



#endif /* SFFS_DIAG_H_ */

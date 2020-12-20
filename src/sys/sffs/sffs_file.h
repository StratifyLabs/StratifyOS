// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md




#ifndef SFFS_LITE_FILE_H_
#define SFFS_LITE_FILE_H_

#include <stdbool.h>

#include "sos/fs/sffs.h"
#include "../sffs/sffs_block.h"
#include "../sffs/sffs_dir.h"
#include "../sffs/sffs_local.h"


/*
 *
 *
 * The purpose of this module is to manage files.
 *
 * When files are modified, they will update the serial
 * number list to mark the status. When files are written,
 * they will update their file list to add new blocks
 * of data as needed.
 *
 * To read a file, the file will lookup the physical block
 * number according to the segment in the filelist. Once
 * it has the physical block address, it will read the data.
 *
 *
 *
 */

typedef struct {
	int segment;
	int offset;
} sffs_file_segment_t;


int sffs_file_new(const void * cfg, cl_handle_t * handle, const char * name, int mode, sffs_dir_lookup_t * entry, uint8_t type, int amode);
int sffs_file_open(const void * cfg, cl_handle_t * handle, serial_t serialno, int amode, bool trunc);
int sffs_file_close(const void * cfg, cl_handle_t * handle);
int sffs_file_write(const void * cfg, cl_handle_t * handle, int start_segment, int nsegments);

int sffs_file_remove(const void * cfg, serial_t serialno);

int sffs_file_startread(const void * cfg, cl_handle_t * handle);
int sffs_file_finishread(const void * cfg, cl_handle_t * handle);

int sffs_file_startwrite(const void * cfg, cl_handle_t * handle);
int sffs_file_finishwrite(const void * cfg, cl_handle_t * handle);

int sffs_file_read(const void * cfg, cl_handle_t * handle, int start_segment,  int nsegments);
void sffs_file_getsegment(const void * cfg, sffs_file_segment_t * segment, int loc);

int sffs_file_swapsegment(const void * cfg, cl_handle_t * handle, int new_segment);

int sffs_file_loadsegment(const void * cfg, cl_handle_t * handle, int segment);
int sffs_file_savesegment(const void * cfg, cl_handle_t * handle);

int sffs_file_clean(const void * cfg, serial_t serialno, block_t hdr_block, uint8_t status);




#endif /* SFFS_LITE_FILE_H_ */

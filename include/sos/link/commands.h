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

#ifndef SOS_LINK_COMMANDS_H_
#define SOS_LINK_COMMANDS_H_

typedef u8 link_cmd_t;


typedef struct MCU_PACK {
	link_cmd_t cmd;
	u32 path_size;
	u32 flags;
	u32 mode;
} link_open_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	u32 path_size_old;
	u32 path_size_new;
} link_symlink_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	int32_t fildes;
	int32_t request;
	u32 arg;
} link_ioctl_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	union {
		int32_t fildes;
		u32 addr;
	};
	u32 nbyte;
} link_read_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	union {
		int32_t fildes;
		u32 addr;
	};
	u32 nbyte;
} link_write_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	int32_t fildes;
} link_close_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	u32 path_size;
} link_unlink_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	int32_t fildes;
	int32_t offset;
	int32_t whence;
} link_lseek_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	u32 path_size;
} link_stat_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	int32_t fildes;
} link_fstat_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	u32 path_size;
	int32_t mode;
} link_mkdir_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	u32 path_size;
} link_rmdir_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	u32 path_size;
} link_opendir_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	int32_t dirp;
} link_readdir_t;

typedef struct MCU_PACK {
	int32_t err;
	int32_t err_number;
	int32_t d_ino;
	u32 d_name_size;
} link_readdir_reply_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	int32_t dirp;
} link_closedir_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	int32_t dirp;
} link_rewinddir_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	int32_t dirp;
} link_telldir_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	int32_t dirp;
	int32_t loc;
} link_seekdir_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	u32 size;
} link_malloc_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	u32 ptr;
} link_free_t;

#define LINK_PATH_ARG_MAX 256

typedef struct MCU_PACK {
	link_cmd_t cmd;
	u32 path_size;
} link_exec_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
} link_time_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	u32 pid;
	u32 tid;
} link_task_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	u32 old_size;
	u32 new_size;
} link_rename_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	u32 path_size;
	u32 uid;
	u32 gid;
} link_chown_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	u32 path_size;
	u32 mode;
} link_chmod_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	u32 path_size;
} link_mkfs_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	u32 pid;
} link_posix_trace_create_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	link_trace_id_t trace_id;
	u32 num_bytes;
} link_posix_trace_tryget_events_t;

typedef struct MCU_PACK {
	link_cmd_t cmd;
	link_trace_id_t trace_id;
} link_posix_trace_shutdown_t;

/*! \brief The USB Link Operation Data Structure (Interrupt Out)
 * \details This data structure defines the data unions
 */
typedef union {
		link_cmd_t cmd;
		link_open_t open;
		link_symlink_t symlink;
		link_ioctl_t ioctl;
		link_write_t write;
		link_read_t read;
		link_close_t close;
		link_unlink_t unlink;
		link_lseek_t lseek;
		link_stat_t stat;
		link_fstat_t fstat;
		link_mkdir_t mkdir;
		link_rmdir_t rmdir;
		link_opendir_t opendir;
		link_readdir_t readdir;
		link_closedir_t closedir;
		link_rewinddir_t rewinddir;
		link_telldir_t telldir;
		link_seekdir_t seekdir;
		link_malloc_t malloc;
		link_free_t free;
		link_exec_t exec;
		link_time_t time;
		link_task_t task;
		link_rename_t rename;
		link_chown_t chown;
		link_chmod_t chmod;
		link_mkfs_t mkfs;
		link_posix_trace_create_t posix_trace_create;
		link_posix_trace_tryget_events_t posix_trace_tryget_events;
		link_posix_trace_shutdown_t posix_trace_shutdown;
} link_op_t;

typedef struct MCU_PACK {
	int32_t err;
	int32_t err_number;
} link_reply_t;

//Commands
enum {
	LINK_CMD_NONE,
	LINK_CMD_READSERIALNO,
	LINK_CMD_IOCTL,
	LINK_CMD_READ,
	LINK_CMD_WRITE,
	LINK_CMD_OPEN,
	LINK_CMD_CLOSE,
	LINK_CMD_LINK,
	LINK_CMD_UNLINK,
	LINK_CMD_LSEEK,
	LINK_CMD_STAT,
	LINK_CMD_FSTAT,
	LINK_CMD_MKDIR,
	LINK_CMD_RMDIR,
	LINK_CMD_OPENDIR,
	LINK_CMD_READDIR,
	LINK_CMD_CLOSEDIR,
	LINK_CMD_RENAME,
	LINK_CMD_CHOWN,
	LINK_CMD_CHMOD,
	LINK_CMD_EXEC,
	LINK_CMD_MKFS,
	LINK_CMD_POSIX_TRACE_CREATE,
	LINK_CMD_POSIX_TRACE_TRY_GET_EVENTS,
	LINK_CMD_POSIX_TRACE_SHUTDOWN,
	LINK_CMD_TOTAL
};


#define LINK_BOOTLOADER_CMD_TOTAL LINK_CMD_WRITE
#define LINK_BOOTLOADER_FILDES (-125)



#endif /* SOS_LINK_COMMANDS_H_ */

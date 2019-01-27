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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef SOS_LINK_TYPES_H_
#define SOS_LINK_TYPES_H_

#include "mcu/types.h"

struct link_tm {
	s32	tm_sec;
	s32	tm_min;
	s32	tm_hour;
	s32	tm_mday;
	s32	tm_mon;
	s32	tm_year;
	s32	tm_wday;
	s32	tm_yday;
	s32	tm_isdst;
};

#define LINK_OPEN_MAX 32
#define LINK_NAME_MAX 24
#define LINK_PATH_MAX 64

#define LINK_TIMEOUT_ERROR (-4)

/*! \details There was an error on the physical layer trying to send/receive data
 * to/from the device.  This error can happen if the device is unexpectedly unplugged.
 */
#define LINK_PHY_ERROR (-3)


/*! \details The physical layer successfully sends and receives data, but
 * the device does not ack correctly.  This usually happens if a command
 * was partially completed before the host aborted.  The protocol needs to
 * be reset with link_resetprot().
 */
#define LINK_PROT_ERROR (-2)

/*! \details The physical and protocol layers successfully transferred
 * data to/from the device and the command was correctly received.  However,
 * there was an error when the device executed the command.  For example,
 * calling link_open() on a file that does not exist on the target will
 * return this error.
 */
#define LINK_DEV_ERROR (-1)

#define LINK_TRANSFER_ERR -701

#define LINK_SIGABRT 1
#define LINK_SIGALRM 2
#define LINK_SIGBUS 3
#define LINK_SIGCHLD 4
#define LINK_SIGCONT 5
#define LINK_SIGFPE 6
#define LINK_SIGHUP 7
#define LINK_SIGILL 8
#define LINK_SIGINT 9
#define LINK_SIGKILL 10
#define LINK_SIGPIPE 11
#define LINK_SIGQUIT 12
#define LINK_SIGSEGV 13
#define LINK_SIGSTOP 14
#define LINK_SIGTERM 15
#define LINK_SIGTSTP 16
#define LINK_SIGTTIN 17
#define LINK_SIGTTOU 18
#define LINK_SIGUSR1 19
#define LINK_SIGUSR2 20
#define LINK_SIGPOLL 21
#define LINK_SIGPROF 22
#define LINK_SIGSYS 23
#define LINK_SIGTRAP 24
#define LINK_SIGURG 25
#define LINK_SIGVTALRM 26
#define LINK_SIGXCPU 27
#define LINK_SIGXFSZ 28
#define LINK_SIGRTMIN 29
#define LINK_SIGRTMAX 31
#define LINK_RTSIG_MAX (SIGRTMAX - SIGRTMIN + 1)
#define LINK_NSIG	32      /* signal 0 implied */

#define LINK_SI_USER    1    /* Sent by a user. kill(), abort(), etc */
#define LINK_SI_QUEUE   2    /* Sent by sigqueue() */
#define LINK_SI_TIMER   3    /* Sent by expiration of a timer_settime() timer */
#define LINK_SI_ASYNCIO 4    /* Indicates completion of asycnhronous IO */
#define LINK_SI_MESGQ   5    /* Indicates arrival of a message at an empty queue */

#define	LINK_F_OK	0
#define	LINK_R_OK	4
#define	LINK_W_OK	2
#define	LINK_X_OK	1

#define LINK_DEV_MAX_STRING 16

#define	LINK_SEEK_SET 0
#define	LINK_SEEK_CUR 1
#define	LINK_SEEK_END 2

#define	LINK__IFMT		0170000	/* type of file */
#define	LINK__IFDIR	0040000	/* directory */
#define	LINK__IFCHR	0020000	/* character special */
#define	LINK__IFBLK	0060000	/* block special */
#define	LINK__IFREG	0100000	/* regular */
#define	LINK__IFLNK	0120000	/* symbolic link */
#define	LINK__IFSOCK	0140000	/* socket */
#define	LINK__IFIFO	0010000	/* fifo */

/*! \details S_IFMT equivalent.
 * \hideinitializer
 */
#define	LINK_S_IFMT		LINK__IFMT
/*! \details S_IFDIR equivalent.
 * \hideinitializer
 */
#define	LINK_S_IFDIR		LINK__IFDIR
/*! \details S_IFCHR equivalent.
 * \hideinitializer
 */
#define	LINK_S_IFCHR		LINK__IFCHR
/*! \details S_IFBLK equivalent.
 * \hideinitializer
 */
#define	LINK_S_IFBLK		LINK__IFBLK
/*! \details S_IFREG equivalent.
 * \hideinitializer
 */
#define	LINK_S_IFREG		LINK__IFREG
/*! \details S_IFLNK equivalent.
 * \hideinitializer
 */
#define	LINK_S_IFLNK		LINK__IFLNK
/*! \details S_IFSOCK equivalent.
 * \hideinitializer
 */
#define	LINK_S_IFSOCK	LINK__IFSOCK
/*! \details S_IFIFO equivalent.
 * \hideinitializer
 */
#define	LINK_S_IFIFO		LINK__IFIFO

/*! \details S_IRWXU equivalent.
 * \hideinitializer
 */
#define	LINK_S_IRWXU 	(LINK_S_IRUSR | LINK_S_IWUSR | LINK_S_IXUSR)
/*! \details S_IRUSR equivalent.
 * \hideinitializer
 */
#define		LINK_S_IRUSR 0000400	/* read permission, owner */
/*! \details S_IWUSR equivalent.
 * \hideinitializer
 */
#define		LINK_S_IWUSR 0000200	/* write permission, owner */
/*! \details S_IXUSR equivalent.
 * \hideinitializer
 */
#define		LINK_S_IXUSR 0000100/* execute/search permission, owner */
/*! \details S_IRWXG equivalent.
 * \hideinitializer
 */
#define	LINK_S_IRWXG		(LINK_S_IRGRP | LINK_S_IWGRP | LINK_S_IXGRP)
/*! \details S_IRGRP equivalent.
 * \hideinitializer
 */
#define		LINK_S_IRGRP 0000040	/* read permission, group */
/*! \details S_IWGRP equivalent.
 * \hideinitializer
 */
#define		LINK_S_IWGRP 0000020	/* write permission, grougroup */
/*! \details S_IXGRP equivalent.
 * \hideinitializer
 */
#define		LINK_S_IXGRP 0000010/* execute/search permission, group */
/*! \details S_IRWXO equivalent.
 * \hideinitializer
 */
#define	LINK_S_IRWXO		(LINK_S_IROTH | LINK_S_IWOTH | LINK_S_IXOTH)
/*! \details S_IROTH equivalent.
 * \hideinitializer
 */
#define		LINK_S_IROTH 0000004	/* read permission, other */
/*! \details S_IWOTH equivalent.
 * \hideinitializer
 */
#define		LINK_S_IWOTH 0000002	/* write permission, other */
/*! \details S_IXOTH equivalent.
 * \hideinitializer
 */
#define		LINK_S_IXOTH 0000001 /* execute/search permission, other */
/*! \details S_ISUID equivalent.
 * \hideinitializer
 */
#define	LINK_S_ISUID 0004000	/* set user id on execution */
/*! \details S_ISGID equivalent.
 * \hideinitializer
 */
#define	LINK_S_ISGID 0002000	/* set group id on execution */
/*! \details S_ISVTX equivalent.
 * \hideinitializer
 */
#define	LINK_S_ISVTX 0001000	/* save swapped text even after use */


/*! \details Beginning of trace overflow \hideinitializer */
#define LINK_POSIX_TRACE_OVERFLOW 0

/*! \hideinitializer \details End of trace overflow */
#define LINK_POSIX_TRACE_RESUME 1

/*! \hideinitializer \details Flush start event */
#define LINK_POSIX_TRACE_FLUSH_START 2

/*! \hideinitializer \details Flush stop event */
#define LINK_POSIX_TRACE_FLUSH_STOP 3

/*! \hideinitializer \details Trace started */
#define LINK_POSIX_TRACE_START 4

/*! \hideinitializer \details Trace stopped */
#define LINK_POSIX_TRACE_STOP 5

/*! \hideinitializer \details Trace filter changed */
#define LINK_POSIX_TRACE_FILTER 6

/*! \hideinitializer \details Trace filter changed */
#define LINK_POSIX_TRACE_ERROR 7

/*! \hideinitializer \details Unname user event */
#define LINK_POSIX_TRACE_UNNAMED_USER_EVENT 8

/*! \hideinitializer \details Message event -- data is a string */
#define LINK_POSIX_TRACE_MESSAGE 9

/*! \hideinitializer \details Warning event -- data is a string */
#define LINK_POSIX_TRACE_WARNING 10

/*! \hideinitializer \details Critical/Error event -- data is a string */
#define LINK_POSIX_TRACE_CRITICAL 11

/*! \hideinitializer \details Fatal event -- data is a string (associate with a crash) */
#define LINK_POSIX_TRACE_FATAL 12

typedef struct MCU_PACK {
	u32 pid;
	u32 tid;
	uint64_t timer;
	u32 mem_loc /*! location of data memory */;
	u32 mem_size /*! size of data memory */;
	u32 malloc_loc /*! malloc base + size */;
	u32 stack_ptr /*! location of stack pointer */;
	u8 prio;
	u8 prio_ceiling;
	u8 is_active;
	u8 is_thread;
	u8 is_enabled;
	char name[LINK_NAME_MAX];
} link_task_data_t;

struct link_dirent {
	u32 d_ino;
	char d_name[LINK_NAME_MAX];
} MCU_PACK;

struct link_stat {
	u32 st_dev;
	u32 st_ino;
	u32 st_mode;
	u32 st_uid;
	u32 st_gid;
	u32 st_rdev;
	u32 st_size;
	u32 st_mtime_;
	u32 st_ctime_;
	u32 st_blksize;
	u32 st_blocks;
} MCU_PACK;

#define LINK_NOTIFY_ID_DEVICE_WRITE 0x101
#define LINK_NOTIFY_ID_DEVICE_READ 0x100
#define LINK_NOTIFY_ID_FILE_WRITE 0x201
#define LINK_NOTIFY_ID_FILE_READ 0x200
#define LINK_NOTIFY_ID_POSIX_TRACE_EVENT 0x300


struct link_timespec {
	u32	tv_sec;
	u32 tv_nsec;
} MCU_PACK;


typedef struct MCU_PACK {
	struct link_timespec create_time;
	u32 log_size;
	u32 data_size;
	u32 stream_size;
	u16 stream_policy;
	u16 log_policy;
	char name[LINK_NAME_MAX];
} link_trace_attr_t;


typedef u32 link_trace_event_id_t;
typedef u32 link_trace_event_set_t;
typedef u32 link_mqd_t;

typedef struct MCU_PACK {
	u32 status;
	link_trace_event_set_t filter;
	link_trace_attr_t attr;
	link_mqd_t mq;
	int tid; //task ID of tracing thread
	int pid; //process ID of traced thread
	u32 checksum; //checksum to ensure trace is valid
} link_trace_id_handle_t;

#ifdef __link
typedef u32 link_trace_id_t;
#else
typedef link_trace_id_handle_t * link_trace_id_t;
#endif

#define LINK_POSIX_TRACE_DATA_SIZE 20


typedef struct MCU_PACK {
		u16 size;
		u16 id;
} link_trace_event_header_t;

typedef struct MCU_PACK {
	u16 posix_event_id;
	s16 posix_truncation_status;
	u16 posix_thread_id;
	u16 posix_pid;
	u32 posix_prog_address;
	u32 posix_timestamp_tv_sec;
	u32 posix_timestamp_tv_nsec;
	u8 data[LINK_POSIX_TRACE_DATA_SIZE];
} link_posix_trace_event_t;


typedef struct MCU_PACK {
	link_trace_event_header_t header;
	union {
		link_posix_trace_event_t posix_trace_event;
	};
	u32 sum32; //must be aligned on 4-byte boundary
} link_trace_event_t;




typedef u32 link_mode_t;

/*! \details Link read-only flag when opening a file/device.
 * \hideinitializer
 */
#define	LINK_O_RDONLY	0
/*! \details Link write-only flag when opening a file/device.
 * \hideinitializer
 */
#define	LINK_O_WRONLY	1
/*! \details Link read-write flag when opening a file/device.
 * \hideinitializer
 */
#define	LINK_O_RDWR		2
/*! \details Link append flag when opening a file/device.
 * \hideinitializer
 */
#define	LINK_O_APPEND	0x0008
/*! \details Link create flag when opening a file/device.
 * \hideinitializer
 */
#define	LINK_O_CREAT	0x0200
/*! \details Link trunc flag when opening a file/device.
 * \hideinitializer
 */
#define	LINK_O_TRUNC	0x0400
/*! \details Link exclude flag when opening a file/device.
 * \hideinitializer
 */
#define	LINK_O_EXCL		0x0800

/*! \details Link non-blocking flag when opening a file/device.
 * \hideinitializer
 */
#define	LINK_O_NONBLOCK	0x4000

#define LINK_O_ACCMODE 0x03



#endif /* SOS_LINK_TYPES_H_ */

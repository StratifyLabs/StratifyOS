/* Copyright 2011-2016 Tyler Gilbert; 
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

/*!
 * \addtogroup LINK_LAYER
 * @{
 * \details The Link Layer is a desktop API that allows desktop programs to hardware and data on a
 * target board running Stratify OS.
 *
 */

/*! \file
 * \brief Link Layer Header
 *
 */

#ifndef IFACE_LINK_H_
#define IFACE_LINK_H_


#include <sys/stat.h>
#include <time.h>

#define LINK_NAME_MAX 24
#define LINK_PATH_MAX 64


#include "../mcu/types.h"
#include "dev/adc.h"
#include "dev/bootloader.h"
#include "dev/core.h"
#include "dev/pio.h"
#include "dev/dac.h"
#include "dev/eint.h"
#include "dev/flash.h"
#include "dev/i2c.h"
#include "dev/pwm.h"
#include "dev/qei.h"
#include "dev/rtc.h"
#include "dev/spi.h"
#include "dev/tmr.h"
#include "dev/uart.h"
#include "dev/usb.h"
#include "dev/sys.h"

#include "link_transport.h"

#if defined( __cplusplus )
extern "C" {
#endif

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

#define APPFS_PAGE_SIZE 256
typedef struct {
	u32 loc;
	u32 nbyte;
	u8 buffer[APPFS_PAGE_SIZE];
} appfs_installattr_t;

typedef appfs_installattr_t appfs_createattr_t;

//Install an executable in RAM or Flash
#define I_APPFS_INSTALL _IOCTLW('a', 0, appfs_installattr_t)

//Create a data file in RAM or flash
#define I_APPFS_CREATE _IOCTLW('a', 1, appfs_createattr_t)
#define I_APPFS_FREE_RAM _IOCTL('a', 2)
#define I_APPFS_RECLAIM_RAM _IOCTL('a', 3)

#define APPFS_CREATE_SIGNATURE 0x12345678


typedef u8 link_cmd_t;

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


//Link Trace API

typedef struct MCU_PACK {
	struct timespec create_time;
	size_t log_size;
	size_t data_size;
	size_t stream_size;
	uint16_t stream_policy;
	uint16_t log_policy;
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

typedef struct MCU_PACK {
	link_trace_event_id_t posix_event_id;
	u32 posix_pid;
	u32 posix_prog_address;
	int32_t posix_truncation_status;
	u32 posix_timestamp_tv_sec;
	u32 posix_timestamp_tv_nsec;
	u32 posix_thread_id;
} link_posix_trace_event_info_t;

#ifdef __link
typedef u32 link_trace_id_t;
#else
typedef link_trace_id_handle_t * link_trace_id_t;
#endif

/*! \details This is set to the errno value
 * reported on the target device.
 */
extern int link_errno;

typedef u32 link_mode_t;

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
	mcu_target_ptr_t ptr;
} link_free_t;

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
};

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
};

#define LINK_INTERRUPT_ENDPOINT_SIZE 32
#define LINK_BULK_ENDPOINT_SIZE 64

#define LINK_ENDPOINT_DATA_SIZE (LINK_BULK_ENDPOINT_SIZE)

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


/*! \details This defines the data structure
 * of the reply received from the device.
 */
typedef struct MCU_PACK {
	int32_t err;
	int32_t err_number;
} link_reply_t;

#define LINK_VID 0x20A0
#define LINK_PID 0x413B

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


#define LINK_APPFS_EXEC_OPTIONS_FLASH (1<<0)
#define LINK_APPFS_EXEC_OPTIONS_STARTUP (1<<1) //if set executes on boot
#define LINK_APPFS_EXEC_OPTIONS_ROOT (1<<3) //run as root
#define LINK_APPFS_EXEC_OPTIONS_REPLACE (1<<4) //replace (default is to duplicate)
#define LINK_APPFS_EXEC_OPTIONS_ORPHAN (1<<5) //calling process wont' be parent
#define LINK_APPFS_EXEC_OPTIONS_UNIQUE (1<<6) //install with a unique name in the flash or RAM

typedef struct {
	u32 startup /*! The startup routine */;
	u32 code_start /*! The start of memory */;
	u32 code_size /*! The size of the code section */;
	u32 ram_start /*! The start of memory */;
	u32 ram_size /*! The size of  memory (total RAM for process) */;
	u32 data_size /*! Size of "data" section */;
	u32 options /*! A pointer to the re-entrancy structure */;
	u32 signature /*! must be valid to execute the file */;
} link_appfs_exec_t; //32 bytes

typedef struct {
	char name[LINK_NAME_MAX] /*! The name of the process or file (must be specially written to the binary) */;
	u32 mode;
} link_appfs_hdr_t;  //28 bytes

typedef struct {
	link_appfs_hdr_t hdr;
	link_appfs_exec_t exec;
} link_appfs_file_t;


/*! \addtogroup USB_HANDLE Device Handling
 * @{
 */

/*! \details This defines the opaque type
 * used to select different USB Link devices.
 */
typedef volatile void * link_dev_t;

/*! \details This initializes the link framework.  This
 * will be called automatically by link_new_dev() if
 * it is not called explicitly.
 * \return Zero on success.
 */
int link_init(void);

/*! \details This shuts down the link framework.
 */
void link_exit(void);

/*! \details This opens a USB Link device based on
 * the serial number.
 * \return The new device or NULL if it could not be opened
 */

link_transport_phy_t link_connect(const char * sn);

/*! \details This closes the specified USB Link device.
 *
 */
int link_disconnect(link_transport_phy_t handle /*! The device to close */);


/*! \details This generates a list of available Link
 * devices.
 * \return A pointer to the new string array.
 */
char * link_new_device_list(int max);

/*! \details This frees a list of USB Link devices
 * (No USB transactions are performed).
 */
void link_del_device_list(char * sn_list /*! The list to free */);

char * link_device_list_entry(char * list, int entry);

/*! \details This function checks to see if there was an error
 * on the last transaction.
 */
int link_get_err(void);

/*! \details This function sets the debugging level for the
 * library (0 for no debugging output).
 */
void link_set_debug(int debug_level);


enum {
	LINK_DEBUG_FATAL,
	LINK_DEBUG_CRITICAL,
	LINK_DEBUG_WARNING,
	LINK_DEBUG_MESSAGE
};

typedef struct {
	char msg[1024];
	int type;
	char file[256];
	char function[256];
	int line;
} link_debug_context_t;

void link_set_debug_callback(void (*write_callback)(link_debug_context_t*));

/*! @} */


//Access to files and devices
/*! \details This function opens a file or device on the
 * target.
 * \return The file handle used to read/write/ioctl the file or device; less
 * than zero on an error.
 */
int link_open(link_transport_phy_t handle, const char * path, int flags, ...);

/*! \details This function performs ioctl control on device associated
 * with the file descriptor.
 * \return Zero on success or an error code.
 */
int link_ioctl(link_transport_phy_t handle, int fildes, int request, ...);

/*! \details This function reads the device (or regular file) associated
 * with the file descriptor.
 *
 * \return The number of bytes read or an error code (less than zero).
 */
int link_read(link_transport_phy_t handle, int fildes, void * buf, int nbyte);

/*! \details This function writes the device (or regular file) associated
 * with the file descriptor.
 *
 * \return The number of bytes writte or an error code (less than zero).
 */
int link_write(link_transport_phy_t handle, int fildes, const void * buf, int nbyte);

/*! \details This function closes the device (or regular file) associated
 * with the file descriptor.
 *
 * \return Zero on success or an error code.
 */
int link_close(link_transport_phy_t handle, int fildes);

/*! \details This function deletes a file from the
 * device's filesystem.
 *
 * \return Zero on success or an HWPL error code.
 */
int link_unlink(link_transport_phy_t handle /*! Device handle */, const char * path /*! The full path to the file to delete */);

//For block devices and files
/*! \details For block devices, this function
 * moves the access pointer.
 *
 * \return Zero on success or an error code.
 */
int link_lseek(link_transport_phy_t handle, int fildes, off_t offset, int whence);

//For files only
int link_stat(link_transport_phy_t handle, const char * path, struct link_stat * buf);
int link_fstat(link_transport_phy_t handle, int fildes, struct link_stat * buf);

//Access to directories
/*! \details This function creates a new directory on
 * the device filesystem.
 * \return 0 on success or an HWPL error code.
 */
int link_mkdir(link_transport_phy_t handle, const char * path /*! The full path to the directory to create */,
		link_mode_t mode /*! Directory mode */);

/*! \details This function removes a directory from
 * the device filesystem.
 * \return 0 on success or an HWPL error code.
 */
int link_rmdir(link_transport_phy_t handle, const char * path /*! The full path to the directory to remove */);

/*! \details This function opens a directory on the device
 * filesystem.  link_readdir() may then be used to
 * read the entries in the directory.
 * \return 0 on success or an HWPL error code.
 */
int link_opendir(link_transport_phy_t handle, const char * dirname /*! The full path to the directory to open */);

/*! \details This function closes a directory that
 * was previously opened with link_opendir().
 *
 * \return 0 on success or an HWPL error code.
 */
int link_closedir(link_transport_phy_t handle, int dirp /*! The handle for the open directory */);

/*! \details This function reads an entry from
 * a the directory that was previously opened with link_opendir().
 * \return 0 on success or an HWPL error code.
 */
int link_readdir_r(link_transport_phy_t handle, int dirp /*! The directory handle */,
		struct link_dirent * entry /*! A pointer to the destination entry */,
		struct link_dirent ** result /*! Points to \a entry on success */);

/*! \details This function formats the filesystem.  All
 * data is erased from the filesystem leaving an
 * emtpy root directory.
 * \return 0 on success or an HWPL error code.
 */
int link_mkfs(link_transport_phy_t handle, const char * path);

/*! \details This function starts a new process on the device.
 *
 * \return Zero on success or an HWPL error code
 */
int link_exec(link_transport_phy_t handle, const char * file);

/*! \details This function creates a new symbolic link.
 *
 * \return Zero on success or an error code.
 */
int link_symlink(link_transport_phy_t handle, const char * old_path, const char * new_path);

/*! \details This function opens the stdio.
 * return Zero on success or an error code.
 */
int link_open_stdio(link_transport_phy_t handle);

/*! \details This function closes the stdio.
 * return Zero on success or an error code.
 */
int link_close_stdio(link_transport_phy_t handle);

/*! \details This function reads the standard output from the device.
 *
 */
int link_read_stdout(link_transport_phy_t handle, void * buf, int nbyte);

/*! \details This function writes the standard input to the device.
 *
 */
int link_write_stdin(link_transport_phy_t handle, const void * buf, int nbyte);

/*! \details This function sets the time on the target device according to the
 * specified struct tm.
 * \return Zero on success or an error code
 */
int link_settime(link_transport_phy_t handle, struct tm * t);

/*! \details This function gets the time on the target device according and
 * stores it in t.
 * \return Zero on success or an error code
 */
int link_gettime(link_transport_phy_t handle, struct tm * t);

/*! \details This function sends the specified signal to the specified pid.
 */
int link_kill_pid(link_transport_phy_t handle, int pid, int signo);




/*! \details This function reads the address of the security word.
 *
 */
int link_get_security_addr(u32 * addr);

/*! \details This renames a file on the filesystem.
 *
 */
int link_rename(link_transport_phy_t handle, const char * old_path, const char * new_path);

/*! \details This changes file ownership.
 *
 */
int link_chown(link_transport_phy_t handle, const char * path, int owner, int group);

/*! \details This changes a file mode.
 *
 */
int link_chmod(link_transport_phy_t handle, const char * path, int mode);

/*! \addtogroup BOOTLOADER Bootloader Functions
 * @{
 *
 * \details These functions are supported when the device is in bootloader mode as well as normal mode.
 * The functions link_readflash(), link_writeflash(), and link_erase_flash() are only available in bootloader
 * mode.
 */

/*! \details
 *
 */
int link_isbootloader(link_transport_phy_t handle);


int link_bootloader_attr(link_transport_phy_t handle, bootloader_attr_t * attr, u32 id);

/*! \details
 *
 */
int link_readserialno(link_transport_phy_t handle, char * serialno, int len);


/*! \details This function resets the device
 * \return 0 on success
 */
int link_reset(link_transport_phy_t handle);

/*! \details This function resets the device and starts the DFU bootloader.
 * \return 0 on success
 */
int link_resetbootloader(link_transport_phy_t handle);

/*! \details This function reads the flash memory from the device.
 *
 */
int link_readflash(link_transport_phy_t handle, int addr, void * buf, int nbyte);

/*! \details This function writes the flash memory of the device.
 *
 */
int link_writeflash(link_transport_phy_t handle, int addr, const void * buf, int nbyte);

/*! \details This erases everything in the flash except the bootloader.
 *
 */
int link_eraseflash(link_transport_phy_t handle);

/*! @} */

int link_posix_trace_create(link_transport_phy_t handle,
		u32 pid,
		link_trace_id_t * id);
int link_posix_trace_tryget_events(link_transport_phy_t handle,
		link_trace_id_t id,
		void * data,
		size_t num_bytes);
int link_posix_trace_shutdown(link_transport_phy_t handle,
		link_trace_id_t id);

const char * link_posix_trace_getname(int trace_number);

void link_set_driver(const link_transport_mdriver_t * driver);
const link_transport_mdriver_t * link_driver();

#if defined( __cplusplus )
}
#endif


#endif /* IFACE_LINK_H_ */

/*! @} */

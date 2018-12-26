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

/*! \mainpage
 *
 * <small>\ref VERSION</small>
 *
 * \section INTRO Introduction
 *
 * <b>Stratify OS</b> is an RTOS designed to drastically reduce development time.  It does this using these principles:
 * - Pre-installed Kernel
 * - Deep hardware integration
 * - Applications are built and installed separate from the kernel
 *
 * We ship hardware <b>pre-installed with Stratify OS</b> (much like many single board computers are pre-installed with Linux).  This means
 * you don't need to do any RTOS code integration.  Just start building your application.
 *
 * Having <b>deep hardware integration</b> means <b>Stratify OS</b> runs on fewer processors but provides drivers for almost
 * all the included hardware.  So no more writing or integrating UART/SPI/I2C drivers.  That is already-ready.
 *
 * Your first step after getting your Stratify enabled board is to <b>write your application</b>.  The application is built
 * and installed independent of the kernel.  Your application is binary compatible with all Stratify boards using the same
 * processor core.  This means you can build a web server and run it on any board with ethernet or wifi.
 *
 * \section START Start Coding
 *
 * This section is the kernel documentation.  You only need to read this is you want to tinker with the kernel.  The API used
 * for applications is <a href="../../StratifyAPI/html/"><b>StratifyAPI</b> documentation</a>. If you want finer control,
 * you can use the built-in \ref POSIX and \ref STDC functions.
 *
 */

/*! \file */

/*! \addtogroup STFY Stratify OS
 * @{
 *
 * \details <b>Stratify OS</b> includes two main APIs for users: the \ref STDC and the \ref POSIX API.
 *
 */

/*! \addtogroup LINK_LAYER Link Transport Layer (for remote access to system)
 * @{
 *
 */

/*! @} */

/*! \addtogroup IFACE_DEV Device Interface
 * @{
 *
 */

/*! @} */

/*!
 *
 * \addtogroup STDC C Standard Library
 * @{
 * \details <b>Stratify OS</b> comes pre-installed with the majority of the C Standard library including:
 * - <assert.h>
 * - <ctype.h>
 * - <errno.h>
 * - <float.h>
 * - <inttypes.h>
 * - <iso646.h>
 * - <limits.h>
 * - <locale.h>
 * - <math.h>
 * - <setjmp.h>
 * - <signal.h>
 * - <stdarg.h>
 * - <stdbool.h>
 * - <stddef.h>
 * - <stdint.h>
 * - <stdio.h>
 * - <stdlib.h>
 * - <string.h>
 * - <tgmath.h>
 * - <time.h>
 * - <wchar.h>
 * - <wctype.h>
 *
 * The following C standard library components are not available:
 * - <complex.h>
 * - <fenv.h>
 *
 * If you need to familiarize yourself with the details of the C Standard Library, try
 * the <a href="http://en.wikipedia.org/wiki/C_standard_library">Wikipedia page</a>.
 *
 *
 */

/*! @} */

/*! \addtogroup POSIX POSIX
 * @{
 *
 * \details The POSIX API contains functions that allow users to create new threads, access files as well as hardware, and
 * perform interprocess communication.
 */

/*! \addtogroup TIME Time
 * @{
 *
 * \details Unix style time functions are available in <b>Stratify OS</b> as documented herein.  The C standard library
 * time functions are also available (see \ref STDC).
 *
 */

/*! @} */

/*! \addtogroup UNISTD Unix Standard (unistd)
 * @{
 *
 */

/*! \addtogroup UNI_FS Filesystem
 * @{
 */

/*! \addtogroup UNI_PERMS Permissions
 * @{
 */

/*! @} */

/*! \addtogroup UNI_CRYPT Encryption
 * @{
 */

/*! @} */

/*! \addtogroup UNI_FILE_ACCESS Access
 * @{
 */

/*! @} */

/*! \addtogroup UNI_FILDES Descriptor Handling
 * @{
 */

/*! @} */

/*! @} */

/*! \addtogroup UNI_PROCESS Processes
 * @{
 */

/*! @} */

/*! \addtogroup UNI_SLEEP Sleep Funcions
 * @{
 */

/*! @} */



/*! @} */

/*! \addtogroup PTHREAD Posix Threads (pthread)
 * @{
 */

/*! @} */

/*! \addtogroup SIGNAL Signal Handling
 * @{
 */

/*! @} */

/*! \addtogroup GRP Groups
 * @{
 */

/*! @} */

/*! \addtogroup PWD Password
 * @{
 */

/*! @} */

/*! \addtogroup SEMAPHORE Semaphore
 * @{
 */

/*! @} */

/*! \addtogroup MQUEUE Message Queue (mqueue)
 * @{
 */

/*! @} */

/*! \addtogroup DIRENT Directory Entry (dirent)
 * @{
 */

/*! @} */

/*! \addtogroup ERRNO Error Numbers
 * @{
 *
 * \details
 *
 * Many functions write the \a errno value to indicate which error has occured.  The list below associates
 * \a errno values to their descriptions.  Alternatively, \a perror() can be used to show the description
 * of the error on the stderr descriptor (normally the same descriptor as the stdout).  For example:
 * \code
 * int fd;
 * fd = open("/noexist", O_RDONLY);
 * if ( fd < 0 ){
 * 	perror("Could not open /noexist");
 * }
 * \endcode
 *
 * The above code outputs "Could not open /noexist: No such file or directory\n" on the stderr while \a errno is
 * assigned a value of 2.
 *
 * - EPERM 1:  Not super-user (Not owner)
 * - ENOENT 2:  No such file or directory
 * - ESRCH 3:  No such process
 * - EINTR 4:  Interrupted system call
 * - EIO 5:  I/O error
 * - ENXIO 6:  No such device or address
 * - E2BIG 7:  Arg list too long
 * - ENOEXEC 8:  Exec format error
 * - EBADF 9:  Bad file number
 * - ECHILD 10:  No children
 * - EAGAIN 11:  Resource temporarily unavailable
 * - ENOMEM 12:  Not enough core
 * - EACCES 13:  Permission denied
 * - EFAULT 14:  Bad address
 * - ENOTBLK 15:  Block device required
 * - EBUSY 16:  Mount device busy
 * - EEXIST 17:  File exists
 * - EXDEV 18:  Cross-device link
 * - ENODEV 19:  No such device
 * - ENOTDIR 20:  Not a directory
 * - EISDIR 21:  Is a directory
 * - EINVAL 22:  Invalid argument
 * - ENFILE 23:  Too many open files in system
 * - EMFILE 24:  Too many open files
 * - ENOTTY 25:  Not a character device
 * - ETXTBSY 26:  Text file busy
 * - EFBIG 27:  File too large
 * - ENOSPC 28:  No space left on device
 * - ESPIPE 29:  Illegal seek
 * - EROFS 30:  Read only file system
 * - EMLINK 31:  Too many links
 * - EPIPE 32:  Broken pipe
 * - EDOM 33:  Math arg out of domain of func
 * - ERANGE 34:  Math result not representable
 * - ENOMSG 35:  No message of desired type
 * - EIDRM 36:  Identifier removed
 * - ECHRNG 37:  Channel number out of range
 * - EL2NSYNC 38:  Level 2 not synchronized
 * - EL3HLT 39:  Level 3 halted
 * - EL3RST 40:  Level 3 reset
 * - ELNRNG 41:  Link number out of range
 * - EUNATCH 42:  Protocol driver not attached
 * - ENOCSI 43:  No CSI structure available
 * - EL2HLT 44:  Level 2 halted
 * - EDEADLK 45:  Deadlock condition
 * - ENOLCK 46:  No record locks available
 * - EBADE 50:  Invalid exchange
 * - EBADR 51:  Invalid request descriptor
 * - EXFULL 52:  Exchange full
 * - ENOANO 53:  No anode
 * - EBADRQC 54:  Invalid request code
 * - EBADSLT 55:  Invalid slot
 * - EDEADLOCK 56:  File locking deadlock error
 * - EBFONT 57:  Bad font file fmt
 * - ENOSTR 60:  Device not a stream
 * - ENODATA 61:  No data (for no delay io)
 * - ETIME 62:  Timer expired
 * - ENOSR 63:  Out of streams resources
 * - ENONET 64:  Machine is not on the network
 * - ENOPKG 65:  Package not installed
 * - EREMOTE 66:  The object is remote
 * - ENOLINK 67:  The link has been severed
 * - EADV 68:  Advertise error
 * - ESRMNT 69:  Srmount error
 * - ECOMM 70:  Communication error on send
 * - EPROTO 71:  Protocol error
 * - EMULTIHOP 74:  Multihop attempted
 * - ELBIN 75:  Inode is remote (not really error)
 * - EDOTDOT 76:  Cross mount point (not really error)
 * - EBADMSG 77:  Trying to read unreadable message
 * - EFTYPE 79:  Inappropriate file type or format
 * - ENOTUNIQ 80:  Given log. name not unique
 * - EBADFD 81:  f.d. invalid for this operation
 * - EREMCHG 82:  Remote address changed
 * - ELIBACC 83:  Can't access a needed shared lib
 * - ELIBBAD 84:  Accessing a corrupted shared lib
 * - ELIBSCN 85:  .lib section in a.out corrupted
 * - ELIBMAX 86:  Attempting to link in too many libs
 * - ELIBEXEC 87:  Attempting to exec a shared library
 * - ENOSYS 88:  Function not implemented
 * - ENMFILE 89      :  No more files
 * - ENOTEMPTY 90:  Directory not empty
 * - ENAMETOOLONG 91:  File or path name too long
 * - ELOOP 92:  Too many symbolic links
 * - EOPNOTSUPP 95:  Operation not supported on transport endpoint
 * - EPFNOSUPPORT 96 :  Protocol family not supported
 * - ECONNRESET 104  :  Connection reset by peer
 * - ENOBUFS 105:  No buffer space available
 * - EAFNOSUPPORT 106 :  Address family not supported by protocol family
 * - EPROTOTYPE 107:  Protocol wrong type for socket
 * - ENOTSOCK 108:  Socket operation on non-socket
 * - ENOPROTOOPT 109:  Protocol not available
 * - ESHUTDOWN 110:  Can't send after socket shutdown
 * - ECONNREFUSED 111:  Connection refused
 * - EADDRINUSE 112:  Address already in use
 * - ECONNABORTED 113:  Connection aborted
 * - ENETUNREACH 114:  Network is unreachable
 * - ENETDOWN 115:  Network interface is not configured
 * - ETIMEDOUT 116:  Connection timed out
 * - EHOSTDOWN 117:  Host is down
 * - EHOSTUNREACH 118:  Host is unreachable
 * - EINPROGRESS 119:  Connection already in progress
 * - EALREADY 120:  Socket already connected
 * - EDESTADDRREQ 121:  Destination address required
 * - EMSGSIZE 122:  Message too long
 * - EPROTONOSUPPORT 123:  Unknown protocol
 * - ESOCKTNOSUPPORT 124:  Socket type not supported
 * - EADDRNOTAVAIL 125:  Address not available
 * - ENETRESET 126
 * - EISCONN 127:  Socket is already connected
 * - ENOTCONN 128:  Socket is not connected
 * - ETOOMANYREFS 129
 * - EPROCLIM 130
 * - EUSERS 131
 * - EDQUOT 132
 * - ESTALE 133
 * - ENOTSUP 134:  Not supported
 * - ENOMEDIUM 135   :  No medium (in tape drive)
 * - ENOSHARE 136    :  No such host or network path
 * - ECASECLASH 137  :  Filename exists with different case
 * - EILSEQ 138
 * - EOVERFLOW 139:  Value too large for defined data type
 * - ECANCELED 140:  Operation canceled
 * - ENOTRECOVERABLE 141:  State not recoverable
 * - EOWNERDEAD 142:  Previous owner died
 * - ESTRPIPE 143:  Streams pipe error
 * - EWOULDBLOCK EAGAIN:  Operation would block
 */

/*! @} */

/*! @} */

#ifndef SOS_H_
#define SOS_H_

#define SOS_VERSION_2_1_0 (0x00020100)
#define SOS_VERSION_2_2_0 (0x00020200)
#define SOS_VERSION (SOS_VERSION_2_2_0)

//Standard libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "mcu/mcu.h"
#include "mcu/types.h"
#include "cortexm/task.h"
#include "cortexm/fault.h"

#if defined __link
#include <posix/trace.h>
#else
#include <sys/lock.h>
#include "trace.h"
#include "sys/socket.h"
#endif
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \details This function puts the controller
 * in hibernate mode.  All CPU and peripheral activity is stopped.
 * The RTC continues running and will wake up the controller
 * after \a seconds has elapsed.  When the controller wakes up, it resumes
 * execution in the same state as before \a hibernate() was called.
 *
 * Hibernate corresponds to \a CORE_SLEEP_DEEPSLEEP.  It can be woken up
 * before \a seconds elapsed if other interrupts are enabled such
 * as pin change interrupts. See the device specific documentation
 * for wakeup events for \a CORE_SLEEP_DEEPSLEEP.
 *
 * @param seconds Number of seconds to hibernate (zero to hibernate indefinitely)
 *
 */
int hibernate(int seconds);

/*! \details This function turns the controller off. The RTC
  * stays on and wakes up the controller after \a seconds has elapsed. When
  * the controller wakes up from power down, it resets.
  *
  * @param seconds Number of seconds to powerdown (zero to powedown indefinitely)
  *
  *
  */
void powerdown(int seconds);


/*! \details This function is weakly bound to code
 * that initializes each filesystem and starts any processes
 * that are designed to auto-run.  It can be completely replaced so
 * that users can customize the startup sequence.
 */
void * sos_default_thread(void * arg);

/*! \details Format the filesystem that is mounted at \a path.
 * \return Zero on success
 *
 */
int mkfs(const char * path);


/*! \details Mount the filesystem specified at \a path
 * @param path The path to the filesystem (specified at build time)
 * @return Zero on success
 */
int mount(const char * path);

/*! \details Unmount the filesystem specified at \a path
 * @param path The path to the filesystem (specified at build time)
 * @return Zero on success
 */
int unmount(const char * path);


/*! \details Launch an application from a data file system.
 *
 * The options are:
 * - APPFS_FLAG_IS_REPLACE
 * - APPFS_FLAG_IS_FLASH
 * - APPFS_FLAG_IS_STARTUP (only with APPFS_FLAG_IS_FLASH)
 * - APPFS_FLAG_IS_REPLACE
 * - APPFS_FLAG_IS_ORPHAN
 *
 * Here is an example of launching a new application:
 * \code
 * #include <caos.h>
 *
 * pid_t p;
 * int status;
 * char exec_path[PATH_MAX];
 *
 * p = launch("/home/HelloWorld", exec_path, 0, APPFS_FLAG_IS_FLASH, 0, 0, 0);
 *
 * wait(&status); //wait until hello world is done running
 * unlink(exec_path); //delete the installed image (or just leave it there to run again)
 *
 * \endcode
 *
 *
 * @param path Path to launch binary
 * @param exec_path The path to store the path where the binary is installed (e.g. /app/flash/0-HelloWorld)
 * @param args Pointer to the arguments for launch
 * @param options Install options mask (flash, startup, etc)
 * @param ram_size The amount of RAM that will be allocated to stack/heap (excludes code even if running from RAM), set to zero for default RAM size
 * @param update_progress Callback to show progress of install/launch
 * @param envp Null for this version
 * @return Zero on success
 */
int launch(const char * path,
			  char * exec_path,
			  const char * args,
			  int options,
			  int ram_size,
			  int (*update_progress)(int, int),
			  char *const envp[]
			  );


/*! \brief Install an application
 * \details This function installs an application in flash or RAM
 * @param path The source path
 * @param exec_path A destination buffer for the path to execute once installed
 * @param options Install options
 * @param ram_size The number of bytes to use for heap/stack
 * @param update_progress Callback to show progress of the install
 * @return Zero on success
 */
int install(const char * path,
				char * name,
				int options,
				int ram_size,
				int (*update_progress)(int, int));

void htoa(char * dest, int num);
char htoc(int nibble);


/*! \brief To be implemented by user. Default returns zero.
 *
 * @param request Request to execute
 * @param data Single argument to the request
 * @return Returns result of request operation
 */
int kernel_request(int request, void * data) MCU_WEAK;

/*! \brief To be implemented by the board support package.
 *
 * @param request The request API unique identifier
 * @return Returns a pointer to the API if it is installed, zero otherwise
 *
 * This function can be implemented to return libraries that
 * are stored in the kernel and shared between applications.
 *
 * For example, the following libraries are requests are supported in the StratifyAPI
 *
 * - MBED TLS for secure sockets
 * - Jansson Json library
 * - Stratify Graphics
 * - Stratify Object notation
 *
 *
 */
const void * kernel_request_api(u32 request) MCU_WEAK;

typedef struct {
	//bool logged;
	uint32_t tid;
	uint32_t pid;
	fault_t fault;
	//time_t time;
	//unsigned int useconds;
} scheduler_fault_t;

#define SOS_TRACE_MESSAGE(msg) sos_trace_event(LINK_POSIX_TRACE_MESSAGE, msg, strnlen(msg, LINK_POSIX_TRACE_DATA_SIZE))
#define SOS_TRACE_WARNING(msg) sos_trace_event(LINK_POSIX_TRACE_WARNING, msg, strnlen(msg, LINK_POSIX_TRACE_DATA_SIZE))
#define SOS_TRACE_CRITICAL(msg) sos_trace_event(LINK_POSIX_TRACE_CRITICAL, msg, strnlen(msg, LINK_POSIX_TRACE_DATA_SIZE))
#define SOS_TRACE_FATAL(msg) sos_trace_event(LINK_POSIX_TRACE_FATAL, msg, strnlen(msg, LINK_POSIX_TRACE_DATA_SIZE))

void sos_trace_event(link_trace_event_id_t event_id, const void * data_ptr, size_t data_len);
void sos_trace_event_addr_tid(link_trace_event_id_t event_id, const void * data_ptr, size_t data_len, u32 addr, int tid);
void sos_trace_root_trace_event(link_trace_event_id_t event_id, const void * data_ptr, size_t data_len);

#define SOS_SCHEDULER_TIMEVAL_SECONDS 2048
#define STFY_SCHEDULER_TIMEVAL_SECONDS SOS_SCHEDULER_TIMEVAL_SECONDS
#define SOS_USECOND_PERIOD (1000000UL * SOS_SCHEDULER_TIMEVAL_SECONDS)
#define STFY_USECOND_PERIOD SOS_USECOND_PERIOD
#define SOS_PROCESS_TIMER_COUNT 4

typedef struct {
	u32 o_flags;
	struct mcu_timeval value;
	struct mcu_timeval interval;
	struct sigevent sigevent;
} sos_process_timer_t;

typedef struct {
	pthread_attr_t attr /*! This holds the task's pthread attributes */;
	volatile void * block_object /*! The blocking object */;
	union {
		volatile int exit_status /*! The task's exit status */;
		void * (*init)(void*) /*! Task 0 init routine */;
	};
	pthread_mutex_t * signal_delay_mutex /*! The mutex to lock if the task cannot be interrupted */;
	volatile struct mcu_timeval wake /*! When to wake the task */;
	volatile u16 flags /*! This indicates whether the process is active or not */;
	trace_id_t trace_id /*! Trace ID is PID is being traced (0 otherwise) */;
	sos_process_timer_t timer[SOS_PROCESS_TIMER_COUNT];
} sched_task_t;

#if !defined __link

/*! \brief Stratify Board Configuration Structure
 * \details This structure holds the compiler-link time
 * configuration data.
 */
typedef struct MCU_PACK {
	u8 clk_usecond_tmr /*! Hardware timer used for usecond counter */;
	u8 task_total /*! Total number of supported tasks */;
	u16 start_stack_size /*! Stack size of the first thread (when in doubt use SOS_DEFAULT_START_STACK_SIZE) */;
	const char * stdin_dev /*! Device used for standard input */;
	const char * stdout_dev /*! Device used for standard output */;
	const char * stderr_dev /*! Device used for standard error */;
	const char * trace_dev /*! Device used for tracing */;
	const char * sys_name /*! System (or board) name */;
	const char * sys_version /*! System (or board) version (distinct from kernel version) */;
	const char * sys_id /*! System ID (globally unique cloud identifier for board) */;
	int sys_memory_size /*! Memory size reserved for the system */;
	int o_sys_flags /*! System flags */;
	void * (*start)(void*) /*! The start routine (when in doubt use sos_default_thread()) */;
	void * start_args /*! Arguments passed to the start routine (for  sos_default_thread() use a pointer to the link transport driver) */;
	const sos_socket_api_t * socket_api /*! Socket API (zero if sockets are not supported) */;
	void (*trace_event)(void * event) /*! This will trace an event (zero if not supported) */;
	void (*request)(void) /*! Send an interrupt request to the link master (0 if not supported) */;
	const char * git_hash /*! A pointer to the git hash string */;
} sos_board_config_t;

#define SOS_DEFAULT_START_STACK_SIZE 2048

//must be provided by board support package
extern volatile sched_task_t sos_sched_table[];
extern volatile task_t sos_task_table[];
extern const sos_board_config_t sos_board_config;


#define SOS_DECLARE_TASK_TABLE(task_count) \
	volatile sched_task_t sos_sched_table[task_count] MCU_SYS_MEM; \
	volatile task_t sos_task_table[task_count] MCU_SYS_MEM

#define SOS_USER_ROOT 0
#define SOS_USER 1

void sos_led_startup();
void sos_led_root_enable(void * args);
void sos_led_root_disable(void * args);
void sos_led_root_error(void * args);

#endif

#ifdef __cplusplus
}
#endif

#endif /*  SOS_H_ */

/*! @} */

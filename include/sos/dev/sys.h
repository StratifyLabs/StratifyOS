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

/*! \addtogroup SYS_DEV System Devices
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details This driver provides details about the currently installed operating system.
 *
 *
 */

/*! \file  */

#ifndef SOS_DEV_SYS_H_
#define SOS_DEV_SYS_H_

#include "mcu/types.h"
#include "sos/link/types.h"


#ifdef __cplusplus
extern "C" {
#endif



enum {
	SYS_FLAG_STDIO_FIFO = (1<<0),
	SYS_FLAG_STDIO_VCP = (1<<1),
	SYS_FLAG_DISABLE_WDT = (1<<2),
	SYS_FLAG_NOTIFY = (1<<3)
};

#define SYS_FLAGS_STDIO_FIFO SYS_FLAG_STDIO_FIFO
#define SYS_FLAGS_STDIO_VCP SYS_FLAG_STDIO_VCP
#define SYS_FLAGS_DISABLE_WDT SYS_FLAG_DISABLE_WDT
#define SYS_FLAGS_NOTIFY SYS_FLAG_NOTIFY

#define SYS_IOC_CHAR 's'

/*! \details This structure defines the system attributes.
 *
 */
typedef struct MCU_PACK {
	char kernel_version[8] /*!  The OS (kernel) Version */;
	char sys_version[8] /*!  The System (board) Version */;
	char arch[16] /*!  The target architecture (lpc17xx, lpc13xx, etc) */;
	u32 signature /*!  Ths OS library signature used to ensure proper build system is used for applications */;
	u32 security /*!  Security flags (see \ref sys_security_flags_t)*/;
	u32 cpu_freq /*!  The CPU clock frequency */;
	u32 sys_mem_size /*!  The number of bytes in RAM shared across OS and other processes */;
	char stdout_name[LINK_NAME_MAX] /*!  Default value for the standard output */;
	char stdin_name[LINK_NAME_MAX] /*!  Default value for the standard output */;
	char name[LINK_NAME_MAX] /*!  Device Name */;
	char id[LINK_PATH_MAX] /*!  Globally unique Cloud Kernel ID value */;
	mcu_sn_t serial /*!  Device Serial number */;
	u32 flags /*!  System flags */;
	u32 hardware_id /*! Hardware ID of the board */;
} sys_26_attr_t;

typedef struct MCU_PACK {
	char version[8] /*!  The OS (kernel) Version */;
	char sys_version[8] /*!  The System (board) Version */;
	char arch[8] /*!  The target architecture (lpc17xx, lpc13xx, etc) */;
	u32 signature /*!  Ths OS library signature used to ensure proper build system is used for applications */;
	u32 security /*!  Security flags (see \ref sys_security_flags_t)*/;
	u32 cpu_freq /*!  The CPU clock frequency */;
	u32 sys_mem_size /*!  The number of bytes in RAM shared across OS and other processes */;
	char stdout_name[LINK_NAME_MAX] /*!  Default value for the standard output */;
	char stdin_name[LINK_NAME_MAX] /*!  Default value for the standard output */;
	char name[LINK_NAME_MAX] /*!  Device Name */;
	mcu_sn_t serial /*!  Device Serial number */;
	u32 flags /*!  System flags */;
} sys_23_attr_t;

typedef sys_26_attr_t sys_attr_t;

typedef struct MCU_PACK {
	char id[LINK_PATH_MAX] /*!  Globally unique Cloud Kernel ID value */;
} sys_id_t;

/*! \brief Task Attributes
 * \details This structure contains the task attributes
 * used with I_SYS_GETTASK.
 */
typedef struct MCU_PACK {
	u32 pid /*! \brief PID for the task */;
	u32 tid /*! \brief Task ID */;
	u64 timer /*! \brief Timer used for tracking task CPU usage */;
	u32 mem_loc /*! \brief location of data memory */;
	u32 mem_size /*! \brief Size of data memory */;
	u32 malloc_loc /*! \brief malloc base + size */;
	u32 stack_ptr /*! \brief location of stack pointer */;
	u8 prio /*! \brief Task Priority */;
	u8 prio_ceiling /*! \brief Task priority ceiling */;
	u8 is_active /*! \brief Non-zero for an active task */;
	u8 is_thread /*! \brief Non-zero if not main process thread */;
	u8 is_enabled /*! \brief Non-zero if associated with running process */;
	char name[LINK_NAME_MAX] /*! \brief Name of the process associated with the task */;
} sys_taskattr_t;

/*! \brief Structure for I_SYS_KILL and I_SYS_PTHREADKILL
 * \details This structure is used with the I_SYS_KILL
 * and I_SYS_PHTHREADKILL requests.
 */
typedef struct MCU_PACK {
	u32 id /*! \brief The id (PID or pthread id) to send the signal to */;
	u32 si_signo /*! \brief The signal to send (SIGKILL or LINK_SIGKILL for example) */;
	s32 si_sigcode /*! \brief The signal code (SI_USER or LINK_SI_USER for example) */;
	s32 si_sigvalue /*! \brief The signal value (not used by default handlers) */;
} sys_killattr_t;

/*! \brief Structure for checking the status of
 * a named process.
 * \details This structure is used for checking the status of a named
 * process.  It is used with the I_SYS_GETPROCESS request.
 */
typedef struct MCU_PACK {
	char name[24] /*! \brief Written by caller to specify name */;
	s32 pid /*! \brief Process ID (written by driver; -1 is process is not running) */;
	s32 pthread_id /*! \brief Thread ID of primary thread in process (written by driver; -1 if process is not running) */;
} sys_process_t;


/*! \brief Data structure to unlock the security word features.
 * \details This data structure is used with I_SYS_UNLOCK.  A successful
 * request will unlock the security features (see \a security in \ref sys_attr_t) of the device giving access
 * to certain parts of the device.
 */
typedef struct MCU_PACK {
	u8 key[32] /*! \brief The password used to unlock the device */;
} sys_sudo_t;

/*! \brief See below.
 * \details This request applies the software write protect
 * to the entire device.
 *
 * Example:
 * \code
 * #include <dev/sys.h>
 * #include <uinstd.h>
 * #include <stdio.h>
 * ...
 * sys_attr_t attr;
 * ioctl(fildes, I_SYS_ATTR, &attr);
 * printf("Version is %s\n", attr.version);
 * \endcode
 *
 * \hideinitializer
 *
 */
#define I_SYS_GETINFO _IOCTLR(SYS_IOC_CHAR, 0, sys_attr_t)
#define I_SYS_26_GETINFO _IOCTLR(SYS_IOC_CHAR, 0, sys_26_attr_t)
#define I_SYS_23_GETINFO _IOCTLR(SYS_IOC_CHAR, 0, sys_23_attr_t)
#define I_SYS_ATTR I_SYS_GETINFO

/*! \brief See below for details.
 * \details This request gets the information about the specified task.
 * \code
 * sys_task_t task;
 * task.tid = 1;
 * ioctl(fd, I_SYS_GETTASK, &task);
 * \endcode
 */
#define I_SYS_GETTASK _IOCTLRW(SYS_IOC_CHAR, 1, sys_taskattr_t)

/*! \brief See below for details
 * \details This request sends a signal to all the tasks in the
 * specified process group.
 */
#define I_SYS_KILL _IOCTLW(SYS_IOC_CHAR, 2, sys_killattr_t)

/*! \brief See below for details
 * \details This request sends a signal to a single task (thread)
 */
#define I_SYS_PTHREADKILL _IOCTLW(SYS_IOC_CHAR, 3, sys_killattr_t)

/*! \brief See below for details
 * \details Read the system ID's globally unique cloud identifier
 */
#define I_SYS_GETID _IOCTLR(SYS_IOC_CHAR, 4, sys_id_t)


/*! \brief See below for details
 * \details This requests checks to see if the named process is currently
 * running.
 *
 * \code
 * sys_process_t p;
 * strcpy(p.name, "hello");
 * ioctl(fd, I_SYS_ISRUNNING, &p);
 * if( p.pid < 0 ){
 * 	//process is not running
 * }
 * \endcode
 */
#define I_SYS_GETPROCESS _IOCTLRW(SYS_IOC_CHAR, 4, sys_process_t)

/*! \brief See below for details
 * \details This request temporarily changes the effective user ID to root.
 * As root certain system functions are available that would not available to
 * the user.  For example, as root, an application can set an interrupt callback
 * that is executed in privileged mode.
 *
 * \code
 * sys_sudo_t passwd;
 * strcpy(passwd.key, "the password");
 * if( ioctl(fd, I_SYS_SUDO, &passwd) < 0 ){
 * 	//failed to accept password
 * }
 * \endcode
 */
#define I_SYS_SUDO _IOCTLW(SYS_IOC_CHAR, 5, sys_sudo_t)

/*! \brief See below for details.
 * \details This copies the sos_board_config_t data that is set by the
 * board support package.
 * \code
 * sos_board_config_t config;
 * ioctl(core_fd, I_SYS_GETBOARDCONFIG, &config);
 * \endcode
 *
 */
#define I_SYS_GETBOARDCONFIG _IOCTLR(SYS_IOC_CHAR, 6, sos_board_config_t)

#define I_SYS_TOTAL 6



#ifdef __cplusplus
}
#endif

#endif /* SOS_DEV_SYS_H_ */

/*! @} */

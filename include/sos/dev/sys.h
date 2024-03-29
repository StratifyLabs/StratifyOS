// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

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

/*! \file 
 */


#ifndef SOS_DEV_SYS_H_
#define SOS_DEV_SYS_H_

#include <sdk/types.h>

#include "sos/fs/types.h"
#include "sos/link/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// 3.3.0 adds path_max and arg_max to sys_info_t
#define SYS_VERSION (0x030300)
#define SYS_IOC_CHAR 's'

/*! \details SYS flags used with
 * o_flags in sos_config_t.
 *
 *
 *
 */
enum sys_board_config_flags {
  SYS_FLAG_IS_STDIO_FIFO /*! Indicates STDIO are independent FIFOs (board config flag) */
  = (1 << 0),
  SYS_FLAG_IS_STDIO_VCP /*! Deprecated (board config flag) */ = (1 << 1),
  SYS_FLAG_IS_WDT_DISABLED /*! Disables the WDT (board config flag) */ = (1 << 2),
  SYS_FLAG_IS_REQUEST /*! Deprecated (board config flag) */ = (1 << 3),
  SYS_FLAG_IS_TRACE /*! Deprecated (board config flag) */ = (1 << 4),
  SYS_FLAG_IS_STDIO_CFIFO /*! STDIO is a with channels 0:stdout 1:stdin 2: stderr (board
                             config flag) */
  = (1 << 5),
  SYS_FLAG_IS_STDIO_CFIFO_SHARE_OUTERR /*! Used with SYS_FLAG_IS_STDIO_CFIFO to indicate
                                          stderr and stdout are the same channel (0)
                                          (board config flag) */
  = (1 << 6),
  SYS_FLAG_IS_ACTIVE_ON_IDLE /*! Don't stop the CPU when the system is idle (board config
                                flag) */
  = (1 << 7),
  SYS_FLAG_IS_KEYED /*! Binary has a 256-bit secret inserted in the binary (before HASH if
                       present).*/
  = (1 << 8),
  SYS_FLAG_IS_HASHED /*! Binary has a 256-bit SHA256 hash appended to the end (after
                        secret key if present) */
  = (1 << 9),
  SYS_FLAG_IS_FIRST_THREAD_AUTHENTICATED /*! First thread is started as a root enabled
                                            thread */
  = (1 << 10)
};

#define SYS_FLAG_IS_FIRST_THREAD_ROOT SYS_FLAG_IS_FIRST_THREAD_AUTHENTICATED

enum sys_memory_flags {
  SYS_FLAG_SET_MEMORY_REGION = (1 << 0),
  SYS_FLAG_IS_READ_ALLOWED = (1 << 1),
  SYS_FLAG_IS_WRITE_ALLOWED = (1 << 2),
  SYS_FLAG_IS_FLASH = (1 << 3),
  SYS_FLAG_IS_EXTERNAL = (1 << 4)
};

typedef struct MCU_PACK {
  char kernel_version[8] /*!  The OS (kernel) Version */;
  char sys_version[8] /*!  The System (board) Version */;
  char arch[16];
  u32 signature;
  u32 security /*!  Security flags */;
  u32 cpu_freq /*!  The CPU clock frequency */;
  u32 sys_mem_size /*!  and other processes */;
  char stdout_name[DEVFS_NAME_MAX + 1] /*!  Default value for the standard output */;
  char stdin_name[DEVFS_NAME_MAX + 1] /*!  Default value for the standard output */;
  char name[DEVFS_NAME_MAX + 1] /*!  Device Name */;
  char id[APPFS_ID_MAX + 1] /*!  Globally unique Cloud Kernel ID value */;
  u32 resd_id[10];
  mcu_sn_t serial /*!  Device Serial number */;
  u32 o_flags /*!  System flags */;
  u32 hardware_id /*! Hardware ID of the board */;
  char trace_name[DEVFS_NAME_MAX + 1] /*! Name of device used for tracing */;
  char bsp_git_hash[16] /*! Git Hash for the board support package */;
  char sos_git_hash[16] /*! Git Hash for the SOS build */;
  char mcu_git_hash[16] /*! Git Hash for the linked MCU library */;
  u32 o_mcu_board_config_flags /*! Flags set by mcu board configuration*/;
  char team_id[APPFS_ID_MAX + 1] /*!  Globally unique Cloud Team ID value */;
  u16 path_max;
  u16 arg_max;
  u32 resd[12];
} sys_info_t;

/*! \details This structure defines the system attributes.
 *
 */
typedef struct MCU_PACK {
  char kernel_version[8] /*!  The OS (kernel) Version */;
  char sys_version[8] /*!  The System (board) Version */;
  char arch[16] /*!  The target architecture (lpc17xx, lpc13xx, etc) */;
  u32 signature /*!  Ths OS library signature */;
  u32 security /*!  Security flags */;
  u32 cpu_freq /*!  The CPU clock frequency */;
  u32 sys_mem_size /*! bytes in RAM shared across OS and other processes */;
  char stdout_name[LINK_NAME_MAX] /*!  Default value for the standard output */;
  char stdin_name[LINK_NAME_MAX] /*!  Default value for the standard output */;
  char name[LINK_NAME_MAX] /*!  Device Name */;
  char id[LINK_PATH_MAX] /*!  Globally unique Cloud Kernel ID value */;
  mcu_sn_t serial /*!  Device Serial number */;
  u32 o_flags /*!  System flags */;
  u32 hardware_id /*! Hardware ID of the board */;
} sys_26_info_t;

typedef struct MCU_PACK {
  char version[8] /*!  The OS (kernel) Version */;
  char sys_version[8] /*!  The System (board) Version */;
  char arch[8] /*!  The target architecture (lpc17xx, lpc13xx, etc) */;
  /*!  Ths OS library signature used to ensure proper build system is used for
   * applications */
  u32 signature;
  u32 security /*!  Security flags */;
  u32 cpu_freq /*!  The CPU clock frequency */;
  u32
    sys_mem_size /*!  The number of bytes in RAM shared across OS and other processes */;
  char stdout_name[LINK_NAME_MAX] /*!  Default value for the standard output */;
  char stdin_name[LINK_NAME_MAX] /*!  Default value for the standard output */;
  char name[LINK_NAME_MAX] /*!  Device Name */;
  mcu_sn_t serial /*!  Device Serial number */;
  u32 o_flags /*!  System flags */;
} sys_23_info_t;

typedef struct MCU_PACK {
  char id[LINK_PATH_MAX] /*! Globally unique Cloud Kernel ID value */;
} sys_id_t;

typedef struct MCU_PACK {
  u32 o_flags /*! Flags used with I_SYS_SETATTR */;
  u32 address;
  u32 size;
  u16 region;
  u16 resd16;
  u32 resd[5];
} sys_attr_t;

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
  u32 id /*! \brief The id (PID for I_SYS_KILL or pthread id for I_SYS_PTHREADKILL) to
            send the signal to */
    ;
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
  char name[LINK_NAME_MAX] /*! \brief Written by caller to specify name */;
  s32 pid /*! \brief Process ID (written by driver; -1 is process is not running) */;
  s32 pthread_id /*! \brief Thread ID of primary thread in process (written by driver; -1
                    if process is not running) */
    ;
} sys_process_t;

typedef struct MCU_PACK {
  u8 data[32];
} sys_secret_key_t;

#define I_SYS_GETVERSION _IOCTL(SYS_IOC_CHAR, I_MCU_GETVERSION)
#define I_SYS_GETINFO _IOCTLR(SYS_IOC_CHAR, I_MCU_GETINFO, sys_info_t)
#define I_SYS_26_GETINFO _IOCTLR(SYS_IOC_CHAR, I_MCU_GETINFO, sys_26_info_t)
#define I_SYS_23_GETINFO _IOCTLR(SYS_IOC_CHAR, I_MCU_GETINFO, sys_23_info_t)

#define I_SYS_SETATTR _IOCTLW(SYS_IOC_CHAR, I_MCU_SETATTR, sys_attr_t)
#define I_SYS_SETACTION _IOCTLW(SYS_IOC_CHAR, I_MCU_SETACTION, mcu_action_t)

/*! \brief See below for details.
 * \details This request gets the information about the specified task.
 * \code
 * sys_task_t task;
 * task.tid = 1;
 * ioctl(fd, I_SYS_GETTASK, &task);
 * \endcode
 */
#define I_SYS_GETTASK _IOCTLRW(SYS_IOC_CHAR, I_MCU_TOTAL, sys_taskattr_t)

/*! \brief See below for details
 * \details This request sends a signal to all the tasks in the
 * specified process group.
 */
#define I_SYS_KILL _IOCTLW(SYS_IOC_CHAR, I_MCU_TOTAL + 1, sys_killattr_t)

/*! \brief See below for details
 * \details This request sends a signal to a single task (thread)
 */
#define I_SYS_PTHREADKILL _IOCTLW(SYS_IOC_CHAR, I_MCU_TOTAL + 2, sys_killattr_t)

/*! \brief See below for details
 * \details Read the system ID's globally unique cloud identifier
 */
#define I_SYS_GETID _IOCTLR(SYS_IOC_CHAR, I_MCU_TOTAL + 3, sys_id_t)

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
#define I_SYS_GETPROCESS _IOCTLRW(SYS_IOC_CHAR, I_MCU_TOTAL + 4, sys_process_t)

#define I_SYS_SUDO _IOCTLW(SYS_IOC_CHAR, I_MCU_TOTAL + 5, sys_auth_t)

/*! \brief See below for details.
 * \details This copies the sos_config_t data that is set by the
 * board support package.
 * \code
 * sos_config_t config;
 * ioctl(core_fd, I_SYS_GETBOARDCONFIG, &config);
 * \endcode
 *
 */
#define I_SYS_GETBOARDCONFIG _IOCTLR(SYS_IOC_CHAR, I_MCU_TOTAL + 6, sos_config_t)

/*! \brief See below for details.
 * \details This gets the MCU
 * board configuration data. If
 * the caller is not privileged,
 * the secret key information
 * will be zero'd.
 *
 */
#define I_SYS_GETMCUBOARDCONFIG _IOCTLR(SYS_IOC_CHAR, I_MCU_TOTAL + 7, mcu_board_config_t)

/*! \brief See below for details.
 * \details Returns 1 if the caller
 * is authenticated access. Returns zero otherwise.
 *
 */
#define I_SYS_ISAUTHENTICATED _IOCTL(SYS_IOC_CHAR, I_MCU_TOTAL + 8)

#define I_SYS_GETSECRETKEY _IOCTLR(SYS_IOC_CHAR, I_MCU_TOTAL + 9, sys_secret_key_t)

/*! \brief See below for details.
 * \details If the caller is authenticated,
 * returns 0 and deauthenticates. If the
 * caller is not authenticated, returns
 *  less than zero with errno set to EPERM.
 *
 */
#define I_SYS_DEAUTHENTICATE _IOCTL(SYS_IOC_CHAR, I_MCU_TOTAL + 10)

#define I_SYS_TOTAL 11

#ifdef __cplusplus
}
#endif

#endif /* SOS_DEV_SYS_H_ */

/*! @} */

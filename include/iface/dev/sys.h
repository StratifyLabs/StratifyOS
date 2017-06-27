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

#ifndef IFACE_DEV_SYS_H_
#define IFACE_DEV_SYS_H_

#include <stdint.h>
#include "../link_types.h"
#include "ioctl.h"

#include "mcu/types.h"


#ifdef __cplusplus
extern "C" {
#endif

/*! \brief This defines the configuration values for
 * the system device (/dev/sys) device in the device table.
 * \hideinitializer
 */
#define SYS_DEVICE { \
		.name = "sys", \
		DEVICE_MODE(0666, 0, 0, S_IFCHR), \
		DEVICE_DRIVER(sys), \
		.cfg.periph.port = 0 \
}

#define SYS_23_DEVICE { \
		.name = "sys", \
		DEVICE_MODE(0666, 0, 0, S_IFCHR), \
		DEVICE_DRIVER(sys_23), \
		.cfg.periph.port = 0 \
}

/*! \brief This defines the configuration values for
 * the null device (/dev/null) device in the device table.
 *
 *
 * \hideinitializer
 *
 */
#define NULL_DEVICE { \
		.name = "null", \
		DEVICE_MODE(0666, 0, 0, S_IFCHR), \
		DEVICE_DRIVER(null), \
		.cfg.periph.port = 0 \
}

/*! \brief This defines the configuration values for
 * the null device (/dev/null) device in the device table.
 *
 *
 * \hideinitializer
 *
 */
#define ZERO_DEVICE { \
		.name = "zero", \
		DEVICE_MODE(0666, 0, 0, S_IFCHR), \
		DEVICE_DRIVER(zero), \
		.cfg.periph.port = 0 \
}

/*! \brief This defines the configuration values for
 * the full device (/dev/full) device in the device table.
 *
 *
 * \hideinitializer
 *
 */
#define FULL_DEVICE { \
		.name = "full", \
		DEVICE_MODE(0666, 0, 0, S_IFCHR), \
		DEVICE_DRIVER(full), \
		.cfg.periph.port = 0 \
}


/*! \brief See below.
 *
 * \details This defines the configuration values for
 * an LED device that can drive up to 4 LEDs.
 *
 * \param dev_name The name of the led device
 * \param active_level LED_ACTIVE_LOW or LED_ACTIVE_HIGH
 * \param pio_port0 The port for channel 0
 * \param pio_pin0 The pin for channel 0
 * \param pio_port1 The port for channel 1 (use LED_DEVICE_UNUSED if not used)
 * \param pio_pin1 The pin for channel 1 (use LED_DEVICE_UNUSED if not used)
 * \param pio_port2 The port for channel 2 (use LED_DEVICE_UNUSED if not used)
 * \param pio_pin2 The pin for channel 2 (use LED_DEVICE_UNUSED if not used)
 * \param pio_port3 The port for channel 3 (use LED_DEVICE_UNUSED if not used)
 * \param pio_pin3 The pin for channel 3 (use LED_DEVICE_UNUSED if not used)
 *
 * \hideinitializer
 *
 */
#define LED_DEVICE(dev_name, active_level, pio_port0, pio_pin0, pio_port1, pio_pin1, pio_port2, pio_pin2, pio_port3, pio_pin3) { \
		.name = dev_name, \
		DEVICE_MODE(0666, 0, 0, S_IFCHR), \
		DEVICE_DRIVER(led), \
		.cfg.periph.port = 0, \
		.cfg.pin_assign = active_level, \
		.cfg.pcfg.pio[0].port = pio_port0, \
		.cfg.pcfg.pio[0].pin = pio_pin0, \
		.cfg.pcfg.pio[1].port = pio_port1, \
		.cfg.pcfg.pio[1].pin = pio_pin1, \
		.cfg.pcfg.pio[2].port = pio_port2, \
		.cfg.pcfg.pio[2].pin = pio_pin2, \
		.cfg.pcfg.pio[3].port = pio_port3, \
		.cfg.pcfg.pio[3].pin = pio_pin3, \
}

#define LED_DEVICE_UNUSED (-1)

#define RANDOM_DEVICE { \
		.name = "random", \
		DEVICE_MODE(0666, 0, 0, S_IFCHR), \
		DEVICE_DRIVER(random), \
		.cfg.periph.port = 0 \
}

#define URANDOM_DEVICE { \
		.name = "urandom", \
		DEVICE_MODE(0666, 0, 0, S_IFCHR), \
		DEVICE_DRIVER(urandom), \
		.cfg.periph.port = 0 \
}


enum {
	SYS_FLAGS_STDIO_FIFO = (1<<0),
	SYS_FLAGS_STDIO_VCP = (1<<1),
	SYS_FLAGS_DISABLE_WDT = (1<<2),
	SYS_FLAGS_NOTIFY = (1<<3)
};

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
	sn_t serial /*!  Device Serial number */;
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
	sn_t serial /*!  Device Serial number */;
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
	i32 si_sigcode /*! \brief The signal code (SI_USER or LINK_SI_USER for example) */;
	i32 si_sigvalue /*! \brief The signal value (not used by default handlers) */;
} sys_killattr_t;

/*! \brief Structure for checking the status of
 * a named process.
 * \details This structure is used for checking the status of a named
 * process.  It is used with the I_SYS_GETPROCESS request.
 */
typedef struct MCU_PACK {
	char name[24] /*! \brief Written by caller to specify name */;
	i32 pid /*! \brief Process ID (written by driver; -1 is process is not running) */;
	i32 pthread_id /*! \brief Thread ID of primary thread in process (written by driver; -1 if process is not running) */;
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
#define I_SYS_GETATTR _IOCTLR(SYS_IOC_CHAR, 0, sys_attr_t)
#define I_SYS_26_GETATTR _IOCTLR(SYS_IOC_CHAR, 0, sys_26_attr_t)
#define I_SYS_23_GETATTR _IOCTLR(SYS_IOC_CHAR, 0, sys_23_attr_t)
#define I_SYS_ATTR I_SYS_GETATTR

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
 * \details This copies the stratify_board_config_t data that is set by the
 * board support package.
 * \code
 * stratify_board_config_t config;
 * ioctl(core_fd, I_SYS_GETBOARDCONFIG, &config);
 * \endcode
 *
 */
#define I_SYS_GETBOARDCONFIG _IOCTLR(SYS_IOC_CHAR, 6, stratify_board_config_t)

#define I_SYS_TOTAL 6

#define LED_IOC_CHAR 'l'
#define LED_ACTIVE_LOW 0
#define LED_ACTIVE_HIGH 1


/*! \details This is used to turn the
 * specified LED on and off.
 */
typedef struct MCU_PACK {
	u8 channel /*! \brief the channel (from 0 to 4) */;
	u8 on /*! \brief Non-zero to turn the LED on (zero to turn it off) */;
} led_req_t;


/*! \brief See below.
 * \details This sets the state of the specified LED.
 *
 * Example:
 * \code
 * #include <unistd.h>
 * #include <dev/sys.h>
 * ...
 * led_req_t req;
 * req.channel = 0;
 * req.on = 1;
 * ioctl(led_fd, I_LED_SET, &req); //turns LED on channel 1 on
 * \endcode
 */
#define I_LED_SET _IOCTLW(LED_IOC_CHAR, 0, led_req_t)

/*! \details This defines the configuration values for
 * a SD SPI device in the device table.
 *
 *
 * \param device_name The name of the device (e.g "disk0")
 * \param port_number The SPI peripheral port number
 * \param pin_assign_value The GPIO configuration to use with the SPI bus
 * \param cs_port_value The GPIO port used for the chip select
 * \param cs_pin_value The GPIO pin used for the chip select
 * \param bitrate_value The maximum allowable bitrate for the SST25VF
 * \param cfg_ptr A pointer to the configuration structure (const sst25vf_cfg_t *)
 * \param state_ptr A pointer to the state structure (sst25vf_state_t *)
 * \param mode_value The access mode (usually 0666)
 * \param uid_value The User ID
 * \param gid_value The Group ID
 *
 * \hideinitializer
 *
 */
#define SDSPI_DEVICE(device_name, port_number, pin_assign_value, cs_port_value, cs_pin_value, bitrate_value, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFBLK), \
		DEVICE_DRIVER(sdspi), \
		.cfg.periph.port = port_number, \
		.cfg.pin_assign = pin_assign_value, \
		.cfg.pcfg.spi.mode = SPI_ATTR_MODE0, \
		.cfg.pcfg.spi.cs.port = cs_port_value, \
		.cfg.pcfg.spi.cs.pin = cs_pin_value, \
		.cfg.pcfg.spi.width = 8, \
		.cfg.pcfg.spi.format = SPI_ATTR_FORMAT_SPI, \
		.cfg.bitrate = bitrate_value, \
		.cfg.state = state_ptr, \
		.cfg.dcfg = cfg_ptr \
}

#define SDSSP_DEVICE(device_name, port_number, pin_assign_value, cs_port_value, cs_pin_value, bitrate_value, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFBLK), \
		DEVICE_DRIVER(sdssp), \
		.cfg.periph.port = port_number, \
		.cfg.pin_assign = pin_assign_value, \
		.cfg.pcfg.spi.mode = SPI_ATTR_MODE0, \
		.cfg.pcfg.spi.cs.port = cs_port_value, \
		.cfg.pcfg.spi.cs.pin = cs_pin_value, \
		.cfg.pcfg.spi.width = 8, \
		.cfg.pcfg.spi.format = SPI_ATTR_FORMAT_SPI, \
		.cfg.bitrate = bitrate_value, \
		.cfg.state = state_ptr, \
		.cfg.dcfg = cfg_ptr \
}



typedef struct MCU_PACK {
	uint16_t r2;
} sdspi_status_t;


#define I_SDSPI_STATUS _IOCTLW('S', 2, sdspi_status_t)

#ifdef __cplusplus
}
#endif

#endif /* IFACE_DEV_SYS_H_ */

/*! @} */

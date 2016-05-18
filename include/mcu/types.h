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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */


#ifndef _MCU_TYPES_H_
#define _MCU_TYPES_H_

#include <stdint.h>
#include <stdbool.h>

#include "iface/dev/ioctl.h"

/*! \details Unsigned 8-bit value */
typedef uint8_t u8;
/*! \details Signed 8-bit value */
typedef int8_t s8;
typedef int8_t i8;
/*! \details Unsigned 16-bit value */
typedef uint16_t u16;
/*! \details Signed 16-bit value */
typedef int16_t s16;
typedef int16_t i16;
/*! \details Unsigned 32-bit value */
typedef uint32_t u32;
/*! \details Signed 32-bit value */
typedef int32_t s32;
typedef int32_t i32;
/*! \details Unsigned 64-bit value */
typedef uint64_t u64;
/*! \details Signed 64-bit value */
typedef int64_t s64;
typedef int64_t i64;



#define MCU_TEST_BIT(x,y) (x & (1<<y))
#define MCU_SET_BIT(x,y) (x |= (1<<y))
#define MCU_SET_MASK(x,y) (x |= y)
#define MCU_CLR_BIT(x,y) (x &= ~(1<<y))
#define MCU_CLR_MASK(x,y) (x &= ~y)

#ifdef __cplusplus
extern "C" {
#endif

/*! \details This macro defines how to declare a weakly-bound
 * alias.
 */
#define MCU_ALIAS(f) __attribute__((weak, alias (#f)))

#define MCU_WEAK __attribute__((weak))

/*! \details This macro is used to specify that a structure
 * should be packed rather than word aligned.
 */
#if defined __MINGW32__
#define MCU_PACK __attribute__((packed, gcc_struct))
#else
#define MCU_PACK __attribute__((packed))
#endif


#define MCU_NAKED __attribute__((naked))


/*! \details This macro aligns memory to a boundary
 *
 * @param x Memory alignment
 */
#define MCU_ALIGN(x) __attribute__((aligned (x)))

/*! \details This macro is used to specify functions
 * which are always inline
 */
#define MCU_ALWAYS_INLINE __attribute__((always_inline))


/*! \details This macro specifies to never inline a function.
 *
 */
#define MCU_NEVER_INLINE __attribute__((noinline))


typedef void * mcu_event_t;

/*! \details This is a HWPL callback pointer.  It is
 * a member of the \ref mcu_action_t structure.
 *
 * See \ref I_SETACTION for an example to use a mcu callback.
 *
 *
 */
typedef int (*mcu_callback_t)(void*, mcu_event_t);

typedef struct {
	mcu_callback_t callback;
	void * context;
} mcu_event_handler_t;

/*! \details This attribute can be applied to RAM so
 * that it is stored in system memory that is universally
 * readable but can only be written in privileged mode.
 *
 * Example:
 * \code
 * static char buffer[512] MCU_SYS_MEM;
 * \endcode
 */
#define MCU_SYS_MEM __attribute__((section(".sysmem")))

#ifdef __link
#define MCU_PRIV_CODE
#else
/*! \details This attribute is applied to functions that should
 * only be executed in privileged mode.  The code in this
 * section can be unreadable by applications.
 *
 */
#define MCU_PRIV_CODE __attribute__((section(".priv_code")))
#endif

#ifdef __link
#define MCU_PRIV_EXEC_CODE
#else
/*! \details This attribute is applied to functions that can
 * be directly executed using mcu_core_privcall()
 *
 */
#define MCU_PRIV_EXEC_CODE __attribute__((section(".priv_exec_code")))
#endif

/*! \details This structure defines an action
 * to take when an asynchronous event occurs (such as
 * a pin change interrupt).
 *
 * \sa I_SETACTION
 */
typedef struct {
	u8 channel /*! \brief The channel (a GPIO pin or timer channel) */;
	s8 prio /*! \brief The interrupt priority */;
	int event /*! \brief The peripheral specific event */;
	mcu_callback_t callback /*! \brief The function to call.  First argument is the context, second argument is peripheral specific */;
	void * context /*! \brief The context to execute the callback in */;
} mcu_action_t;

/*! \details This lists the bits used in the
 * sigcode.
 */
typedef enum {
	MCU_SIGCODE_OP_COMPLETE /*! Operation complete bit */ = (1<<0),
	MCU_SIGCODE_NEW_DATA /*! New data bit */ = (1<<1),
	MCU_SIGCODE_EVENT /*! Event bit */ = (1<<2)
} mcu_sigcode_bitmask_t;

/*! \details This defines a global ioctl() request.  It
 * specifies to read the peripheral's attribute data structure.  The data
 * is written to the data pointed to by the third argument to ioctl().
 * Example:
 * \code
 * adc_attr_t attr;
 * ioctl(adc_fd, I_GETATTR, &attr);
 * \endcode
 *
 * \hideinitializer
 */
#define I_GLOBAL_ATTR 0
#define I_GLOBAL_GETATTR I_GLOBAL_ATTR

/*! \details This defines a global ioctl() request.  It
 * specifies to write the peripheral's attribute data structure.
 * The attribute structure is passed as a pointer in the third argument
 * to ioctl().
 *
 * Example:
 * \code
 * adc_attr_t attr;
 * attr.pin_assign = 0;
 * attr.freq = 200000;
 * attr.enabled_channels = (1<<0);
 * ioctl(adc_fd, I_SETATTR, &attr);
 * \endcode
 *
 * \hideinitializer
 */
#define I_GLOBAL_SETATTR 1

/*! \details This defines a global ioctl() request.  It
 * sets an action to take when the operation completes.
 *
 * The third argument to ioctl() is always a pointer to an \ref mcu_action_t
 * data structure.
 *
 *
 * \hideinitializer
 */
#define I_GLOBAL_SETACTION (2 | _IOCTL_ROOT)
#define I_GLOBAL_CANCEL I_GLOBAL_SETACTION

#define I_GLOBAL_TOTAL 3

/*! \details This specifies the value for pin_assign
 * when opening a port on a peripheral.  Most MCU's
 * allow a port to be mapped to different GPIO
 * locations.  The standard locations are supported
 * by using values 0, 1, 2, etc (see the device specific
 * documentation for details).  If none
 * of the standard locations works for your
 * application, you can configure the GPIO
 * manually, then send this value to the
 * driver to indicate the GPIO was already configured.
 */
#define MCU_PINASSIGN_USER 50
#define MCU_GPIO_CFG_USER MCU_PINASSIGN_USER

/*! \brief Serial number */
typedef struct {
	u32 sn[4];
} sn_t;


#ifdef __cplusplus
}
#endif


#endif /* _MCU_TYPES_H_ */

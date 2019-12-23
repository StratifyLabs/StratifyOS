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


#ifndef _MCU_TYPES_H_
#define _MCU_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#if !defined __link
#include <sys/stat.h>
#define F32X "%lX"
#define F3208X "%08lX"
#define F32D "%ld"
#define F32U "%lu"
#define FINTD "%d"
#define FINTU "%u"
#else
#define F32X "%X"
#define F3208X "%08X"
#define F32D "%d"
#define F32U "%u"
#define FINTD "%d"
#define FINTU "%u"
#endif


#include "sos/ioctl.h"

typedef uint8_t u8;
typedef int8_t s8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;

#ifdef __cplusplus
extern "C" {
#endif


#define MCU_ALIAS(f) __attribute__((weak, alias (#f)))
#define MCU_WEAK __attribute__((weak))
#define MCU_UNUSED __attribute__((unused))
#define MCU_UNUSED_ARGUMENT(arg) ((void)arg)
#define MCU_NO_RETURN __attribute((noreturn))
#define MCU_STRINGIFY2(x) #x
#define MCU_STRINGIFY(x) MCU_STRINGIFY2(x)

#if defined MCU_SOS_GIT_HASH
#define SOS_GIT_HASH MCU_STRINGIFY(MCU_SOS_GIT_HASH)
#else
#define SOS_GIT_HASH "0000000"
#endif

#if defined __MINGW32__
#define MCU_PACK __attribute__((packed, gcc_struct))
#else
#define MCU_PACK __attribute__((packed))
#endif

#define MCU_NAKED __attribute__((naked))
#define MCU_ALIGN(x) __attribute__((aligned (x)))
#define MCU_ALWAYS_INLINE __attribute__((always_inline))
#define MCU_NEVER_INLINE __attribute__((noinline))

#define MCU_ARRAY_COUNT(x) (sizeof(x)/sizeof(x[0]))

typedef struct MCU_PACK {
	u32 o_events /*! Event or events that happened */;
	void * data /*! A pointer to the device specific event data */;
} mcu_event_t;


typedef enum {
	MCU_EVENT_FLAG_NONE = 0,
	MCU_EVENT_FLAG_DATA_READY /*! Data have been received and is ready to read */ = (1<<0),
	MCU_EVENT_FLAG_WRITE_COMPLETE /*! A write operation has completed */ = (1<<1),
	MCU_EVENT_FLAG_CANCELED /*! An operation was canceled */ = (1<<2),
	MCU_EVENT_FLAG_RISING /*! Specifies a rising edge */ = (1<<3),
	MCU_EVENT_FLAG_FALLING /*! Specifies a falling edge */ = (1<<4),
	MCU_EVENT_FLAG_SET_PRIORITY /*! If set, I_X_SETACTION requests will adjust the interrupt priority */ = (1<<5),
	MCU_EVENT_FLAG_ERROR /*! An error occured during */ = (1<<6),
	MCU_EVENT_FLAG_ADDRESSED /*! The device has been addressed (I2C for example) */ = (1<<7),
	MCU_EVENT_FLAG_OVERFLOW /*! An overflow condition has occurred */ = (1<<8),
	MCU_EVENT_FLAG_UNDERRUN /*! An underrun condition has occurred */ = (1<<9),
	MCU_EVENT_FLAG_HIGH /*! High event */ = (1<<10),
	MCU_EVENT_FLAG_LOW /*! Low event (used with external interrupts) */ = (1<<11),
	MCU_EVENT_FLAG_SETUP /*! USB Setup event */ = (1<<12),
	MCU_EVENT_FLAG_INDEX /*! Index event for QEI and similar (Setup alias) */ = MCU_EVENT_FLAG_SETUP,
	MCU_EVENT_FLAG_STALL /*! Stall event */ = (1<<13),
	MCU_EVENT_FLAG_STOP /*! Stop event (Stall alias) */ = MCU_EVENT_FLAG_STALL,
	MCU_EVENT_FLAG_DIRECTION_CHANGED /*! Direction change for QEI an similar (Stall alias) */ = MCU_EVENT_FLAG_STALL,
	MCU_EVENT_FLAG_RESET /*! Reset event */ = (1<<14),
	MCU_EVENT_FLAG_POWER /*! Power event */ = (1<<15),
	MCU_EVENT_FLAG_SUSPEND /*! Suspend event */ = (1<<16),
	MCU_EVENT_FLAG_RESUME /*! Resume event */ = (1<<17),
	MCU_EVENT_FLAG_DEBUG /*! Debug event */ = (1<<18),
	MCU_EVENT_FLAG_WAKEUP /*! Wakeup event */ = (1<<19),
	MCU_EVENT_FLAG_SOF /*! Start of frame event */ = (1<<20),
	MCU_EVENT_FLAG_MATCH /*! Match event */ = (1<<21),
	MCU_EVENT_FLAG_ALARM /*! Alarm event (match alias) */ = MCU_EVENT_FLAG_MATCH,
	MCU_EVENT_FLAG_COUNT /*! Count event */ = (1<<22),
	MCU_EVENT_FLAG_HALF_TRANSFER /*! The transfer is halfway complete (used with MCU_EVENT_FLAG_WRITE_COMPLETE and MCU_EVENT_FLAG_DATA_READY with circular DMA) */ = (1<<23),
	MCU_EVENT_FLAG_STREAMING /*! The transfer is streaming data and will continue until stopped */ = (1<<24)
} mcu_event_flag_t;

typedef enum {
	MCU_ERROR_NONE /*! 0: No Error */,
	MCU_ERROR_INVALID_PIN_ASSINGMENT /*! 1: Invalid Pin Assignment */,
	MCU_ERROR_INVALID_FREQUENCY /*! 2: Invalid Frequency */,
	MCU_ERROR_INVALID_CHANNEL_LOCATION /*! 3: Invalid Channel Location */,
	MCU_ERROR_INVALID_CHANNEL_VALUE /*! 4: Invalid Channel Value */,
	MCU_ERROR_I2C_ACK_ERROR /*! 6: I2C Ack Error */
} mcu_error_t;

typedef enum {
	MCU_CHANNEL_FLAG_IS_INPUT = 0x80
} mcu_channel_flag_t;

typedef int (*mcu_callback_t)(void*, const mcu_event_t *);

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
#define MCU_SYS_MEM __attribute__((section(".sysmem"))) MCU_ALIGN(4)

#ifdef __link
#define MCU_ROOT_CODE
#define MCU_PRIV_CODE
#else
#define MCU_ROOT_CODE __attribute__((section(".priv_code")))
#define MCU_PRIV_CODE MCU_ROOT_CODE
#endif

#ifdef __link
#define MCU_ROOT_EXEC_CODE
#define MCU_PRIV_EXEC_CODE
#else
#define MCU_ROOT_EXEC_CODE __attribute__((section(".priv_exec_code")))
#define MCU_PRIV_EXEC_CODE MCU_ROOT_EXEC_CODE
#endif

/*! \details This structure defines an action
 * to take when an asynchronous event occurs (such as
 * a pin change interrupt).
 *
 */
typedef struct {
	u8 channel /*! The channel (a GPIO pin or timer channel) */;
	s8 prio /*! The interrupt priority */;
	u32 o_events /*! The peripheral specific event */;
	mcu_event_handler_t handler /*! Event handler */;
} mcu_action_t;

/*! \brief MCU Pin
 *
 */
typedef struct MCU_PACK {
	u8 port /*! Port */;
	u8 pin /*! Pin number */;
} mcu_pin_t;

static inline int mcu_is_port_valid(u8 port){
	return (port != 0xff);
}

static inline mcu_pin_t mcu_invalid_pin(){
	mcu_pin_t pin;
	pin.port = 0xff;
	pin.pin = 0xff;
	return pin;
}

static inline mcu_pin_t mcu_pin(u8 port, u8 num){
	mcu_pin_t pin;
	pin.port = port;
	pin.pin = num;
	return pin;
}

#define MCU_PIN_ASSIGNMENT_COUNT(type) (sizeof(type)/sizeof(mcu_pin_t))

#define MCU_CONFIG_PIN_ASSIGNMENT(type, handle) (handle->config ? &(((type*)(handle->config))->attr.pin_assignment) : 0)


typedef struct MCU_PACK {
	u32 loc;
	u32 value;
} mcu_channel_t;

static inline mcu_channel_t mcu_channel(u32 loc, u32 value){
	mcu_channel_t channel;
	channel.loc = loc;
	channel.value = value;
	return channel;
}

#define I_MCU_GETVERSION 0
#define I_MCU_GETINFO 1
#define I_MCU_SETATTR 2
#define I_MCU_SETACTION (3 | _IOCTL_ROOT)
#define I_MCU_TOTAL 4

#define MCU_TEST_BIT(x,y) (x & (1<<y))
#define MCU_SET_BIT(x,y) (x |= (1<<y))
#define MCU_SET_MASK(x,y) (x |= y)
#define MCU_CLR_BIT(x,y) (x &= ~(1<<y))
#define MCU_CLR_MASK(x,y) (x &= ~y)

typedef struct {
	u32 sn[4];
} mcu_sn_t;

struct mcu_timeval {
	u32 tv_sec; //SCHEDULER_TIMEVAL_SECONDS seconds each
	u32 tv_usec; //up to 1000000 * SCHEDULER_TIMEVAL_SECONDS
};

#define MCU_RAM_PAGE_SIZE 1024

#define MCU_API_REQUEST_CODE(a,b,c,d) (a<<24|b<<16|c<<8|d)
#define MCU_REQUEST_CODE(a,b,c,d) (a<<24|b<<16|c<<8|d)

#define MCU_PI_FLOAT (3.14159265358979323846f)


#ifdef __cplusplus
}
#endif


#endif /* _MCU_TYPES_H_ */

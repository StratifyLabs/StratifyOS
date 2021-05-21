// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup SYS_FIFO FIFO Buffer
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details This device is a FIFO RAM buffer.  A write to the buffer adds
 * data to the FIFO.  A read from the buffer reads from the FIFO.  If the FIFO
 * fills up, additional writes will block until data arrives.  Similarly, if the FIFO
 * is empty, a read will block until data arrives.
 *
 */

/*! \file  */

#ifndef SOS_DEV_CFIFO_H_
#define SOS_DEV_CFIFO_H_

#include "fifo.h"
#include <sdk/types.h>

#define CFIFO_VERSION (0x030000)
#define CFIFO_IOC_CHAR 'M'

enum {
  CFIFO_FLAG_NONE = 0,
};

typedef struct MCU_PACK {
  u32 o_flags /*! Fifo flags */;
  u16 count /*! Total number of channels in the fifo */;
  u16 size /*! The size of each channel */;
  u32 o_ready /*! Bitmask of channels with at least one byte */;
  u32 resd[8];
} cfifo_info_t;

/*! \brief FIFO Attributes
 * \details This structure defines the attributes of a FIFO.
 */
typedef struct MCU_PACK {
  u32 o_flags /*! Fifo flags */;
  u32 resd[8];
} cfifo_attr_t;

typedef struct MCU_PACK {
  u32 channel;
} cfifo_fiforequest_t;

typedef struct MCU_PACK {
  u32 channel;
  fifo_attr_t attr;
} cfifo_fifoattr_t;

typedef struct MCU_PACK {
  u32 channel;
  fifo_info_t info;
} cfifo_fifoinfo_t;

#define I_CFIFO_GETVERSION _IOCTL(CFIFO_IOC_CHAR, I_MCU_GETVERSION)
#define I_CFIFO_GETINFO _IOCTLR(CFIFO_IOC_CHAR, 0, cfifo_info_t)
#define I_CFIFO_SETATTR _IOCTLW(CFIFO_IOC_CHAR, 1, cfifo_attr_t)

#define I_CFIFO_GETOWNER _IOCTLRW(CFIFO_IOC_CHAR, 2, mcu_channel_t)
#define I_CFIFO_SETOWNER _IOCTLW(CFIFO_IOC_CHAR, 3, mcu_channel_t)

#define I_CFIFO_FIFOINIT _IOCTLW(CFIFO_IOC_CHAR, 4, cfifo_fiforequest_t)
#define I_CFIFO_FIFOFLUSH _IOCTLW(CFIFO_IOC_CHAR, 5, cfifo_fiforequest_t)
#define I_CFIFO_FIFOEXIT _IOCTLW(CFIFO_IOC_CHAR, 6, cfifo_fiforequest_t)
#define I_CFIFO_FIFOSETATTR _IOCTLW(CFIFO_IOC_CHAR, 7, cfifo_fifoattr_t)
#define I_CFIFO_FIFOGETINFO _IOCTLRW(CFIFO_IOC_CHAR, 8, cfifo_fifoinfo_t)

#endif /* SOS_DEV_CFIFO_H_ */

/*! @} */

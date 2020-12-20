// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup SYS_DEVFIFO Device FIFO
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details This device driver is used to add a FIFO read buffer to a device.  When writing the FIFO,
 * the device is written directly.  Any data that arrives asynchronously through the device is stored
 * in the FIFO buffer.  When reading the FIFO, the FIFO buffer is read.
 *
 *
 *
 *
 */

/*! \file  */

#ifndef SOS_DEV_DEVFIFO_H_
#define SOS_DEV_DEVFIFO_H_

#include <sdk/types.h>

#define DEVFIFO_VERSION (0x030000)
#define DEVFIFO_IOC_CHAR 'f'

/*! \details This structure defines the attributes used
 * in a device fifo.
 */
typedef struct MCU_PACK {
	uint32_t size /*! \brief The total number of bytes in the FIFO */;
	uint32_t used /*! \brief The number of bytes ready to be read from the FIFO */;
	uint32_t overflow /*! \brief Non-zero if FIFO has dropped data because data is arriving
	faster than it is being read; this value is cleared when it is read using I_DEVFIFO_GETINFO */;
} devfifo_info_t;

typedef devfifo_info_t devfifo_attr_t;

#define I_DEVFIFO_GETVERSION _IOCTL(DEVFIFO_IOC_IDENT_CHAR, I_MCU_GETVERSION)

/*! \details This request gets the size of the FIFO in bytes.
 *
 * Example:
 * \code
 * devfifo_attr_t attr;
 * ioctl(fifo_fd, I_DEVFIFO_GETINFO, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_DEVFIFO_GETINFO _IOCTLR(DEVFIFO_IOC_CHAR, I_MCU_GETINFO, devfifo_info_t)

#define I_DEVFIFO_TOTAL 2

/*! \details This is used for the configuration of the device.
 *
 */
typedef struct {
	const void * dev /*! \brief The underlying device */;
	char * buffer /*! \brief The buffer for the fifo */;
	int size /*! \brief The size of the fifo */;
	int req_getbyte /*! \brief The request used to get a byte from the device */;
	int req_setaction /*! \brief The request to set the action */;
	int event /*! \brief The event to trigger on */;
} devfifo_config_t;


#endif /* SOS_DEV_DEVFIFO_H_ */


/*! @} */

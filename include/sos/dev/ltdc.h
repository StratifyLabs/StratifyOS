// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup LTDC_DEV LCD/TFT Display Controller
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details
 */

/*! \file 
 */

#ifndef SOS_DEV_LTDC_H_
#define SOS_DEV_LTDC_H_


#include <stdint.h>

#include <sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LTDC_VERSION (0x030000)
#define LTDC_IOC_CHAR 'L'

enum {
	LTDC_FLAG_SET_MASTER = (1<<0)
};


/*! \brief LTDC information
 * \details This contains information about
 *	the driver.
 */
typedef struct MCU_PACK {
	u32 o_flags /*! LTDC_FLAG's that are supported by the driver implementation */;
	u32 o_events /*! MCU_EVENT_FLAG's that are supported by the implementation */;
	u32 resd[8];
} ltdc_info_t;


typedef struct MCU_PACK {
	mcu_pin_t red[8];
	mcu_pin_t green[8];
	mcu_pin_t blue[8];
	mcu_pin_t hsync;
	mcu_pin_t vsync;
	mcu_pin_t clock;
	mcu_pin_t data_enable;
} ltdc_pin_assignment_t;

typedef struct MCU_PACK {
	u32 o_flags;
	u32 freq;
	ltdc_pin_assignment_t pin_assignment;
	u32 resd[8];
} ltdc_attr_t;

#define I_LTDC_GETVERSION _IOCTL(LTDC_IOC_CHAR, I_MCU_GETVERSION)


/*! \details This request gets the attributes of the device.
 *
 * Example:
 * \code
 * display_info_t info;
 * ioctl(fildes, I_DISPLAY_GETINFO, &info);
 * \endcode
 * \hideinitializer
 */
#define I_LTDC_GETINFO _IOCTLR(DISPLAY_IOC_CHAR, 0, display_info_t)

#define I_LTDC_TOTAL 2

/*! @} */

#ifdef __cplusplus
}
#endif


#endif /* SOS_DEV_LTDC_H_ */

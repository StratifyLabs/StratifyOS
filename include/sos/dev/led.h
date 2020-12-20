// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef SOS_DEV_LED_H_
#define SOS_DEV_LED_H_

#include <sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LED_VERSION (0x030000)
#define LED_IOC_IDENT_CHAR 'L'

enum {
	LED_FLAG_NONE = (1<<0),
	LED_FLAG_IS_HIGH_IMPEDANCE /*! LED driver disables output when LED is off */ = (1<<1),
	LED_FLAG_IS_DUTY_CYCLE /*! When set, use \a duty_cycle and \a period to set brightness or flashing */ = (1<<2),
	LED_FLAG_ENABLE /*! Turn the LED on */ = (1<<3),
	LED_FLAG_DISABLE /*! Turn the LED off (use with LED_FLAG_IS_HIGH_IMPEDANCE) */ = (1<<4),
};

/*! \brief LED status data structure
 * \details This data structure is written to the
 * LED to apply the settings.
 *
 *
 */
typedef struct MCU_PACK {
	u8 brightness;
	u8 red;
	u8 green;
	u8 blue;
} led_status_t;

/*! \brief I2S IO Attributes
 *  \details This structure defines how the control structure
 * for configuring the I2S port.
 */
typedef struct MCU_PACK {
	u32 o_flags  /*!  Flags that are supported by the driver */;
	u32 o_events  /*! Events that are supported by the driver */;
	u32 resd[8];
} led_info_t;

#define LED_PIN_ASSIGNMENT_COUNT 1

typedef struct MCU_PACK {
	u32 o_flags  /*! Flag bitmask */;
	u32 duty_cycle /*! LED duty cycle (if supported) */;
	u32 period /*! LED blinking period in us (if supported) */;
	u32 resd[8];
} led_attr_t;

#define I_LED_GETVERSION _IOCTL(LED_IOC_IDENT_CHAR, I_MCU_GETVERSION)


/*! \brief This request gets the LED info.
 * \hideinitializer
 */
#define I_LED_GETINFO _IOCTLR(LED_IOC_IDENT_CHAR, I_MCU_GETINFO, led_info_t)

/*! \brief This request sets the LED attributes.
 * \hideinitializer
 */
#define I_LED_SETATTR _IOCTLW(LED_IOC_IDENT_CHAR, I_MCU_SETATTR, led_attr_t)

/*! \brief This request sets the I2S action.
 * \hideinitializer
 */
#define I_LED_SETACTION _IOCTLW(LED_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)


#define I_LED_TOTAL 0


#ifdef __cplusplus
}
#endif



#endif /* SOS_DEV_LED_H_ */

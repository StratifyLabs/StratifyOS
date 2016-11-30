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

/*! \addtogroup DEVICE Device Configuration
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details This interface describes data structures for writing device drivers.
 *
 */

#ifndef _DEVICE_H_
#define _DEVICE_H_


#include "mcu/types.h"
#include "iface/dev/pio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEVICE_GET_PORT(x) (x->periph.port)

/*! \brief Data structure for on-chip MCU peripherals.
 * \details This data structure is used for all on-chip MCU peripherals.
 *
 */
typedef struct MCU_PACK {
	u8 port /*! \brief The port number for the peripheral */;
} device_periph_t;


/*! \brief Structure defining a GPIO port/pin combination
 * \details This structure defines a GPIO port/pin
 * combination used in configuring devices.
 */
typedef struct MCU_PACK {
	i8 port /*! \brief The GPIO port number (-1 means not used)*/;
	i8 pin /*! \brief The GPIO pin number */;
} device_gpio_t;

/*! \brief Data structure for a SPI device with a CS pin
 * \details This data structure defines
 * the data used to implement
 * a device driver which utilizes
 * the SPI port and a single GPIO pin
 * for the chip select.
 *
 */
typedef struct MCU_PACK {
	pio_t cs /*! \brief The chip select gpio port/pin */;
	u8 width /*! \brief The width of a SPI bus word */;
	u8 format /*! \brief The format the SPI bus uses */;
	u8 mode /*! \brief The SPI mode (0, 1, 2, or 3) */;
	u8 reserved0;
} device_spi_cfg_t;

/*! \brief Data structure for an I2C device (SCL and SDA lines)
 * \details This structure defines the data structure
 * for configuring a device driver that uses
 * the I2C bus.
 */
typedef struct MCU_PACK {
	u16 slave_addr /*! \brief The I2C Slave address */;
} device_i2c_cfg_t;

/*! \brief Data structure for a UART device (RX and TX lines)
 * \details This structure defines the data
 * used to configure a device driver which
 * uses the UART to communicate with the device.
 */
typedef struct MCU_PACK {
	u8 stop_bits /*! \brief The number of stop bits */;
	u8 parity /*! \brief The Parity (none, odd, or even) */;
	u8 width /*! \brief The width of a UART word */;
} device_uart_cfg_t;

/*! \brief The maximum number of ADC channels in \ref device_adc_cfg_t.
 *
 */
#define DEVICE_MAX_ADC_CHANNELS 8

/*! \brief Data structure for an analog input device (up to 16 channels)
 * \details This structure defines the data for
 * an analog input device with up to 16 inputs.
 *
 */
typedef struct MCU_PACK {
	u16 reference /*! \brief The reference voltage in millivolts */;
	i8 channels[DEVICE_MAX_ADC_CHANNELS] /*! \brief The channels to use (-1) if not used. */;
} device_adc_cfg_t;

/*! \brief The maximum number of PWM channels in \ref device_pwm_cfg_t.
 *
 */
#define DEVICE_MAX_PWM_CHANNELS 8

/*! \brief Data structure for a PWM device (up to 8 channels)
 * \details This structure defines the data for use
 * with a PWM driven device (such as an H-bridge).
 *
 */
typedef struct {
	u32 top /*! \brief The top value of the PWM */;
	u32 freq /*! \brief The PWM timer frequency; The period is the "top" member divided by "freq" */;
	i8 channels[DEVICE_MAX_PWM_CHANNELS] /*! \brief The channels to use (-1) if not used. */;
} device_pwm_cfg_t;


#define MCU_CFG_SIZE_MAX 32

/*! \brief Data structure for device configuration.
 * \details This defines the device configuration.
 *
 */
typedef struct MCU_PACK {
	device_periph_t periph /*! \brief This is the configuration used for all peripherals */;
	u8 pin_assign /*! \brief The GPIO pin configuration */;
	u32 bitrate /*! \brief Specifies the max bit rate in bps */;
	union {
		device_spi_cfg_t spi /*! \brief The configuration for devices that are connected via SPI bus */;
		device_pwm_cfg_t pwm /*! \brief The configuration for devices connected via PWM */;
		device_adc_cfg_t adc /*! \brief The configuration for devices connected to ADC pins */;
		device_uart_cfg_t uart /*! \brief The configuration for devices connected to UART pins */;
		device_i2c_cfg_t i2c /*! \brief The configuration for devices connected via I2C bus */;
		pio_t pio[4] /*! \brief Used for generic GPIO devices such as LEDs */;
	} pcfg;
	const void * dcfg /*! \brief Pointer to device specific configuration */;
	void * state /*! \brief Pointer to device specific state (RAM) */;
} device_cfg_t;


/*! \brief Data structure used for reads and writes of devices.
 * \details This is the data structure for data transfers.
 *
 */
typedef struct MCU_PACK {
	int tid /*! \brief The calling task ID */;
	int flags /*! \brief The flags for the open file descriptor */;
	int loc /*! \brief The location to read or write */;
	union {
		const void * cbuf /*! \brief Pointer to const void buffer */;
		void * buf /*! \brief Pointer to void buffer */;
	};
	int nbyte /*! \brief The number of bytes to transfer */;
	//mcu_event_handler_t handler /*! \brief MCU Event handler */;
	mcu_callback_t callback /*! \brief The function to call when the operation completes */;
	void * context /*! \brief The first argument to \a callback */;
} device_transfer_t;

typedef int (*device_driver_open_t)(const device_cfg_t*);
typedef int (*device_driver_ioctl_t)(const device_cfg_t*, int, void*);
typedef int (*device_driver_read_t)(const device_cfg_t*, device_transfer_t *);
typedef int (*device_driver_write_t)(const device_cfg_t*, device_transfer_t *);
typedef int (*device_driver_close_t)(const device_cfg_t*);

/*! \brief HWPL Driver Function Pointers
 * \details This structure defines the contents
 * of a HWPL device file.  The HWPL device files
 * are stored in local flash rather
 * than on the external flash so that
 * they can be loaded even if there is no
 * filesystem.
 */
typedef struct MCU_PACK {
	device_driver_open_t open /*! \brief A pointer to the periph_open() function */;
	device_driver_ioctl_t ioctl /*! \brief A pointer to the periph_ioctl() function */;
	device_driver_read_t read /*! \brief A pointer to the periph_read() function */;
	device_driver_write_t write /*! \brief A pointer to the periph_write() function */;
	device_driver_close_t close /*! \brief A pointer to the periph_close() function */;
} device_driver_t;

/*! \brief A device driver (can either be an MCU peripheral or external IC)
 * \details This data structure contains the driver name,
 * function pointers, port number, and type.
 */
typedef struct {
	char name[NAME_MAX] /*! \brief The name of the device */;
	u8 uid /*! \brief The user ID of the device */;
	u8 gid /*! \brief The group ID of the device */;
	u16 mode /*! \brief The file access values */;
	device_driver_t driver /*! \brief The driver functions */;
	device_cfg_t cfg /*! \brief The configuration for the device */;
} device_t;


#define DEVICE_MODE(mode_value, uid_value, gid_value, type) .mode = mode_value | type, \
		.uid = uid_value, \
		.gid = gid_value

#define DEVICE_DRIVER(driver_name) .driver.open = driver_name##_open, \
		.driver.close = driver_name##_close, \
		.driver.ioctl = driver_name##_ioctl, \
		.driver.read = driver_name##_read, \
		.driver.write = driver_name##_write

/*! \details This macro is used to initialize an MCU peripheral in the
 * devices table.
 *
 * Example:
 * \code
 * const device_t devices[] = {
 * 	DEVICE_PERIPH("adc0", adc, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR),
 * 	DEVICE_PERIPH("uart0", uart, 0, 0666, USER_ROOT, GROUP_ROOT, S_IFCHR)
 * }
 * \endcode
 */
#define DEVICE_PERIPH(device_name, periph_name, port_number, mode_value, uid_value, gid_value, device_type) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, device_type), \
		DEVICE_DRIVER(periph_name), \
		.cfg.periph.port = port_number \
}

#define DEVICE_TERMINATOR { \
		.driver.open = NULL \
}

#ifndef __link

static inline bool device_is_terminator(const device_t * dev);
bool device_is_terminator(const device_t * dev){
	if ( dev->driver.open == NULL ){
		return true;
	}
	return false;
}
#endif


/*! @} */

/*! @} */

#ifdef __cplusplus
}
#endif


#endif /* _DEVICE_H_ */

/*! @} */

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

/*! \addtogroup QEI Quadrature Encoder Interface (QEI)
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details The Quadrature Encoder Interface (QEI) provides to MCUs' hardware encoders
 * used to monitor the speed and direction of motors and other rotary devices.
 *
 * More information about accessing peripheral IO is in the \ref IFACE_DEV section.
 */

/*! \file
 * \brief Quadrature Encoder Interface Header File
 *
 */

#ifndef DEV_QEI_H_
#define DEV_QEI_H_

#include <stdint.h>
#include "ioctl.h"
#include "mcu/arch.h"

#ifdef __cplusplus
extern "C" {
#endif


#define QEI_IOC_IDENT_CHAR 'q'

#ifndef QEI_SAMPLE_T
#define QEI_SAMPLE_T
typedef uint32_t qei_sample_t;
#endif

/*! \details This defines the bits used in
 * the "mode" member of \ref qei_attr_t.
 */
typedef enum {
	QEI_MODE_INVERT_DIR /*! Invert the direction */ = (1<<0),
	QEI_MODE_SIGNAL_MODE /*! Use signal mode */ = (1<<1),
	QEI_MODE_DOUBLE_EDGE /*! Count both edges */ = (1<<2),
	QEI_MODE_INVERT_INDEX /*! Invert the index */ = (1<<3),
} qei_mode_t;

/*! \details This lists the bitmask used with
 * the \ref I_QEI_RESET request.
 */
typedef enum {
	QEI_RESET_POS /*! Reset the position */ = (1<<0),
	QEI_RESET_VELOCITY /*! Reset the velocity */ = (1<<1),
	QEI_RESET_INDEX /*! Reset the index count */ = (1<<2),
	QEI_RESET_POS_ONINDEX /*! Reset the position on the next index pulse */ = (1<<3)
} qei_reset_mask_t;


/*! \brief See below for details.
 * \details These are the events for \ref qei_action_t.
 */
typedef enum {
	QEI_ACTION_EVENT_INDEX /*! A pulse on the index line */ = (1<<0),
	QEI_ACTION_EVENT_DIRCHANGE /*! A change in direction */ = (1<<1)
} qei_action_event_mask_t;

/*! \brief QEI IO Attributes
 * \details This structure defines how the control structure
 * for opening or reconfiguring the QEI port.
 */
typedef struct MCU_PACK {
	uint8_t pin_assign /*! \brief The GPIO configuration to use (see \ref LPC17XXDEV) */;
	uint32_t max_pos /*! \brief The maximum position (QEI rolls over at this point) */;
	uint32_t velocity_comp /*! \brief The Velocity compare value */;
	uint32_t filter /*! \brief The filter coefficient (0 to disable) */;
	uint32_t mode /*! \brief The QEI mode (see \ref qei_mode_t) */;
	uint32_t vfreq /*! \brief The frequency at which to update the velocity in Hz */;
} qei_attr_t;

/*! \brief This defines a QEI action.
 */
typedef mcu_action_t qei_action_t;

/*! \brief This request gets the QEI attributes.
 * \hideinitializer
 */
#define I_QEI_GETATTR _IOCTLR(QEI_IOC_IDENT_CHAR, I_GLOBAL_GETATTR, qei_attr_t)
/*! \brief This request sets the QEI attributes.
 * \hideinitializer
 */
#define I_QEI_SETATTR _IOCTLW(QEI_IOC_IDENT_CHAR, I_GLOBAL_SETATTR, qei_attr_t)
/*! \brief This request sets the action to be taken when a QEI interrupt occurs.
 * \hideinitializer
 */
#define I_QEI_SETACTION _IOCTLW(QEI_IOC_IDENT_CHAR, I_GLOBAL_SETACTION, qei_action_t)

/*! \brief See details below.
 * \details This request reads the QEI position.
 *
 * Example:
 * \code
 * uint32_t value;
 * value = ioctl(qei_fd, I_QEI_READ);
 * \endcode
 * \hideinitializer
 */
#define I_QEI_GET _IOCTL(QEI_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 0)


/*! \brief See details below.
 * \details This request reads the QEI velocity.
 *
 * Example:
 * \code
 * int32_t velocity;
 * velocity = ioctl(qei_fd, I_QEI_GETVELOCITY);
 * \endcode
 *
 * \hideinitializer
 */
#define I_QEI_GETVELOCITY _IOCTL(QEI_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 1)
#define I_QEI_GET_VELOCITY I_QEI_GETVELOCITY

/*! \brief See details below.
 * \details This request reads the QEI index counter.
 *
 * Example:
 * \code
 * int32_t index_pulses;
 * index_pulses = ioctl(qei_fd, I_QEI_GETINDEX);
 * \endcode
 *
 * \hideinitializer
 */
#define I_QEI_GETINDEX _IOCTL(QEI_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 2)
#define I_QEI_GET_INDEX I_QEI_GETINDEX

/*! \brief See details below.
 * \details This request resets the QEI position.  The ctl
 * argument is one or more \ref qei_reset_mask_t values.
 *
 * Example (resets the position and velocity but not the index count):
 * \code
 * ioctl(qei_fd, I_QEI_RESETPOS, (QEI_RESET_POS|QEI_RESET_VELOCITY));
 * \endcode
 *
 * \hideinitializer
 */
#define I_QEI_RESET _IOCTL(QEI_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 3)


#define I_QEI_TOTAL 4

#ifdef __cplusplus
}
#endif


#endif /* DEV_QEI_H_ */

/*! @} */

// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

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

#ifndef SOS_DEV_QEI_H_
#define SOS_DEV_QEI_H_

#include <sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif


#define QEI_VERSION (0x030000)
#define QEI_IOC_IDENT_CHAR 'q'

#ifndef QEI_SAMPLE_T
#define QEI_SAMPLE_T
typedef u32 qei_sample_t;
#endif

/*! \details This defines the bits used in
 * the "mode" member of \ref qei_attr_t.
 */
typedef enum {
	QEI_FLAG_NONE = 0,
	QEI_FLAG_SET /*! Set the QEI attributes */ = (1<<0),
	QEI_FLAG_IS_INVERT_DIR /*! Invert the direction */ = (1<<1),
	QEI_FLAG_IS_SIGNAL_MODE /*! Use signal mode */ = (1<<2),
	QEI_FLAG_IS_DOUBLE_EDGE /*! Count both edges */ = (1<<3),
	QEI_FLAG_IS_INVERT_INDEX /*! Invert the index */ = (1<<4),
	QEI_FLAG_RESET /*! Reset the QEI */ = (1<<5),
	QEI_FLAG_IS_RESET_POS /*! Reset the position */ = (1<<6),
	QEI_FLAG_IS_RESET_VELOCITY /*! Reset the velocity */ = (1<<7),
	QEI_FLAG_IS_RESET_INDEX /*! Reset the index count */ = (1<<8),
	QEI_FLAG_IS_RESET_POS_ONINDEX /*! Reset the position on the next index pulse */ = (1<<9),
} qei_flag_t;

typedef struct {
	u32 o_flags;
	u32 o_events;
	u32 resd[8];
} qei_info_t;

typedef struct MCU_PACK {
	mcu_pin_t a;
	mcu_pin_t b;
	mcu_pin_t idx;
} qei_pin_assignment_t;

/*! \brief QEI IO Attributes
 * \details This structure defines how the control structure
 * for opening or reconfiguring the QEI port.
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Flag bitmask */;
	qei_pin_assignment_t pin_assignment /*! Pin assignment */;
	u32 freq;
	u32 top /*! Maximum value */;
	u32 velocity_freq;
	u32 max_position /*! The maximum position (QEI rolls over at this point) */;
	u32 velocity_comp /*! The Velocity compare value */;
	u32 filter /*! The filter coefficient (0 to disable) */;
	u32 resd[8];
} qei_attr_t;

/*! \brief This defines a QEI action.
 */
typedef mcu_action_t qei_action_t;

#define I_QEI_GETVERSION _IOCTL(QEI_IOC_IDENT_CHAR, I_MCU_GETVERSION)


/*! \brief This request gets the QEI attributes.
 * \hideinitializer
 */
#define I_QEI_GETINFO _IOCTLR(QEI_IOC_IDENT_CHAR, I_MCU_GETINFO, qei_info_t)

/*! \brief This request sets the QEI attributes.
 * \hideinitializer
 */
#define I_QEI_SETATTR _IOCTLW(QEI_IOC_IDENT_CHAR, I_MCU_SETATTR, qei_attr_t)
/*! \brief This request sets the action to be taken when a QEI interrupt occurs.
 * \hideinitializer
 */
#define I_QEI_SETACTION _IOCTLW(QEI_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)

/*! \brief See details below.
 * \details This request reads the QEI position.
 *
 * Example:
 * \code
 * u32 value;
 * value = ioctl(qei_fd, I_QEI_READ);
 * \endcode
 * \hideinitializer
 */
#define I_QEI_GET _IOCTL(QEI_IOC_IDENT_CHAR, I_MCU_TOTAL + 0)


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
#define I_QEI_GETVELOCITY _IOCTL(QEI_IOC_IDENT_CHAR, I_MCU_TOTAL + 1)
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
#define I_QEI_GETINDEX _IOCTL(QEI_IOC_IDENT_CHAR, I_MCU_TOTAL + 2)
#define I_QEI_GET_INDEX I_QEI_GETINDEX


#define I_QEI_TOTAL 3

#ifdef __cplusplus
}
#endif


#endif /* SOS_DEV_QEI_H_ */

/*! @} */

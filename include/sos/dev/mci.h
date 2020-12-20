// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup MCI Multimedia Card Interface
 * @{
 *
 * \ingroup IFACE_DEV
 * \details The MCI peripheral is used to communicate with SD and MMC cards
 *
 */

/*! \file
 * \brief Multimedia Card Interface Header File
 *
 */

#ifndef SOS_DEV_MCI_H_
#define SOS_DEV_MCI_H_

#include <sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MCI_VERSION (0x000000)
#define MCI_IOC_IDENT_CHAR 'm'



typedef enum {
	MCI_MODE_WIDEBUS = (1<<0),
	MCI_MODE_MMC = (1<<1),
	MCI_MODE_SD = (1<<2)
} mci_mode_t;


/*! \brief PIO Attribute Data Structure
 * \details This structure defines how the control structure
 * for configuring the PIO port.
 */
typedef struct MCU_PACK {
	uint8_t pin_assign /*! \brief Pin assignment */;
	uint16_t block_size /*! \brief Block size of card */;
	uint16_t mode /*! \brief OR the mci_mode_t */;
	uint32_t freq /*! \brief MCI clock frequency */;
} mci_attr_t;

#define I_MCI_GETVERSION _IOCTL(MCI_IOC_IDENT_CHAR, I_MCU_GETVERSION)


/*! \brief IOCTL request to get the attributes.
 * \hideinitializer
 */
#define I_MCI_GETINFO _IOCTLR(MCI_IOC_IDENT_CHAR, I_MCU_GETINFO, mci_attr_t)

/*! \brief IOCTL request to set the attributes.
 * \hideinitializer
 */
#define I_MCI_SETATTR _IOCTLW(MCI_IOC_IDENT_CHAR, I_MCU_SETATTR, mci_attr_t)

/*! \brief IOCTL request to set the action.
 * \hideinitializer
 */
#define I_MCI_SETACTION _IOCTLW(MCI_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)


#define I_MCI_TOTAL 0

#ifdef __cplusplus
}
#endif

#endif /* SOS_DEV_MCI_H_ */

/*! @} */

// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef SOS_DEV_EEPROM_H_
#define SOS_DEV_EEPROM_H_

#include <sdk/types.h>

#define EEPROM_VERSION (0x030000)
#define EEPROM_IOC_IDENT_CHAR 'E'

/*! \brief EEPROM Attribute Data Structure
 * \details This structure defines how the control structure
 * for opening or reconfiguring the EEPROM port.
 */
typedef struct MCU_PACK {
	u32 size;
	u32 resd[8];
} eeprom_attr_t;

typedef struct MCU_PACK {
	u32 o_flags /*! EEPROM flags */;
	u32 size /*! EEPROM size */;
	u16 page_size /*! EEPROM page size */;
	u16 resd16;
	u32 resd[7];
} eeprom_info_t;

#define I_EEPROM_GETVERSION _IOCTL(EEPROM_IOC_IDENT_CHAR, I_MCU_GETVERSION)
#define I_EEPROM_GETINFO _IOCTLR(EEPROM_IOC_IDENT_CHAR, I_MCU_GETINFO, eeprom_info_t)
#define I_EEPROM_SETATTR _IOCTLW(EEPROM_IOC_IDENT_CHAR, I_MCU_SETATTR, eeprom_attr_t)
#define I_EEPROM_SETACTION _IOCTLW(EEPROM_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)



#define I_EEPROM_TOTAL 0



#endif /* SOS_DEV_EEPROM_H_ */

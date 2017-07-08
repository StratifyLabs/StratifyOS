/*
 * eeprom.h
 *
 *  Created on: Nov 2, 2013
 *      Author: tgil
 */

#ifndef SOS_DEV_EEPROM_H_
#define SOS_DEV_EEPROM_H_

#include "mcu/types.h"

#define EEPROM_IOC_IDENT_CHAR 'E'

/*! \brief EEPROM Attribute Data Structure
 * \details This structure defines how the control structure
 * for opening or reconfiguring the EEPROM port.
 */
typedef struct MCU_PACK {
	u32 size;
} eeprom_attr_t;

typedef struct MCU_PACK {
	u32 o_flags;
	u32 size;
} eeprom_info_t;

#define I_EEPROM_GETINFO _IOCTLR(EEPROM_IOC_IDENT_CHAR, I_MCU_GETINFO, eeprom_info_t)
#define I_EEPROM_SETATTR _IOCTLW(EEPROM_IOC_IDENT_CHAR, I_MCU_SETATTR, eeprom_attr_t)
#define I_EEPROM_SETACTION _IOCTLW(EEPROM_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)



#define I_EEPROM_TOTAL 0



#endif /* SOS_DEV_EEPROM_H_ */

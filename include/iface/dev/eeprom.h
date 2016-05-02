/*
 * eeprom.h
 *
 *  Created on: Nov 2, 2013
 *      Author: tgil
 */

#ifndef MCU_EEPROM_H_
#define MCU_EEPROM_H_

#include <stdint.h>
#include "ioctl.h"
#include "mcu/arch.h"

#define EEPROM_IOC_IDENT_CHAR 'E'

/*! \brief EEPROM Attribute Data Structure
 * \details This structure defines how the control structure
 * for opening or reconfiguring the EEPROM port.
 */
typedef struct MCU_PACK {
	uint32_t size;
} eeprom_attr_t;

#define I_EEPROM_GETATTR _IOCTLR(EEPROM_IOC_IDENT_CHAR, I_GLOBAL_GETATTR, mem_attr_t)
#define I_EEPROM_SETATTR _IOCTLW(EEPROM_IOC_IDENT_CHAR, I_GLOBAL_SETATTR, mem_attr_t)
#define I_EEPROM_SETACTION _IOCTLW(EEPROM_IOC_IDENT_CHAR, I_GLOBAL_SETACTION, mem_attr_t)



#define I_EEPROM_TOTAL 0



#endif /* EEPROM_H_ */

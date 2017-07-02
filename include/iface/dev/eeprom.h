/*
 * eeprom.h
 *
 *  Created on: Nov 2, 2013
 *      Author: tgil
 */

#ifndef IFACE_DEV_EEPROM_H_
#define IFACE_DEV_EEPROM_H_

#include <stdint.h>
#include "ioctl.h"
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

#define I_EEPROM_ATTR _IOCTLR(EEPROM_IOC_IDENT_CHAR, I_GLOBAL_ATTR, mem_attr_t)
#define I_EEPROM_GETATTR I_EEPROM_ATTR
#define I_EEPROM_SETATTR _IOCTLW(EEPROM_IOC_IDENT_CHAR, I_GLOBAL_SETATTR, mem_attr_t)
#define I_EEPROM_SETACTION _IOCTLW(EEPROM_IOC_IDENT_CHAR, I_GLOBAL_SETACTION, mem_attr_t)



#define I_EEPROM_TOTAL 0



#endif /* IFACE_DEV_EEPROM_H_ */

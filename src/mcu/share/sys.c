/*
 * sys.c
 *
 *  Created on: Dec 20, 2012
 *      Author: tgil
 */

#include "mcu/sys.h"


extern const uint32_t * _mcu_core_vector_table[];

static bool security_lock MCU_SYS_MEM;
static bool security_lock = true;

uint32_t _mcu_sys_getsecurity(){
	if( security_lock == true ){
		return (uint32_t)_mcu_core_vector_table[9];;
	}
	return 0xFFFFFFFF; //all allowed
}

void _mcu_sys_setsecuritylock(bool enabled){
	security_lock = enabled;
}

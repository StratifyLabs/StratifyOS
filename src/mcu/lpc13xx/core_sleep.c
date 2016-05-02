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

#include "mcu/mcu.h"
#include "mcu/core.h"
#include "core_flags.h"


static void priv_set_sleep_mode(int * level);

int _mcu_core_sleep(core_sleep_t level){

	mcu_core_privcall((core_privcall_t)priv_set_sleep_mode, &level);
	if ( level < 0 ){
		return level;
	}

	//Wait for an interrupts
	__WFI();
	return 0;
}

void priv_set_sleep_mode(int * level){
	SCB->SCR &= ~(1<<SCB_SCR_SLEEPDEEP_Pos);
	LPC_PMU->PCON = 0;
	switch(*level){
	case CORE_DEEPSLEEP_STOP:
		LPC_PMU->PCON = 1; //turn off the flash as well
	case CORE_DEEPSLEEP:
		SCB->SCR |= (1<<SCB_SCR_SLEEPDEEP_Pos);
		break;
	case CORE_SLEEP:
		break;
	case CORE_DEEPSLEEP_STANDBY:
		SCB->SCR |= (1<<SCB_SCR_SLEEPDEEP_Pos);
		LPC_PMU->PCON = 3;
		break;
	default:
		*level = -1;
		return;
	}
	*level = 0;
}



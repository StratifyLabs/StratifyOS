/* Copyright 2011-2017 Tyler Gilbert;
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

#ifndef LPC177X_8X_H_
#define LPC177X_8X_H_

#include <stdint.h>

#include <string.h>
#include "mcu/types.h"

#include "LPC177x_8x_cm3.h"
#include "delay_cm3.h"
#include "common_cm3.h"
#include "mcu_lpc_core.h"
#include "mcu_lpc_dma.h"

/*! \details This defines the timer channels.
 * It is assigned to the channel member of \ref mcu_action_t
 * and \ref mcu_channel_t.
 */
typedef enum {
	TMR_ACTION_CHANNEL_OC0 /*! Output compare channel 0 */,
	TMR_ACTION_CHANNEL_OC1 /*! Output compare channel 1 */,
	TMR_ACTION_CHANNEL_OC2 /*! Output compare channel 2 */,
	TMR_ACTION_CHANNEL_OC3 /*! Output compare channel 3 */,
	TMR_ACTION_CHANNEL_IC0 /*! Input capture channel 0 */,
	TMR_ACTION_CHANNEL_IC1 /*! Input capture channel 1 */,
} lpc177x_8x_tmr_action_channel_t;

typedef lpc177x_8x_tmr_action_channel_t tmr_action_channel_t;

#include "mcu_lpcxx7x_8x.h"

#endif /* LPC177X_8X_H_ */

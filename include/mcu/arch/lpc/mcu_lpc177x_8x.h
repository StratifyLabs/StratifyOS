/*
 * lpc177x_8x.h
 *
 *  Created on: Sep 27, 2013
 *      Author: tgil
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

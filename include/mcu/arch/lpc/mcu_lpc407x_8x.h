

#ifndef LPC407X_8X_H_
#define LPC407X_8X_H_

#include <string.h>
#include "mcu/types.h"

#if !defined CORE_M4
#define CORE_M4
#endif

#if defined __SOFTFP__
#undef __SOFTFP__
#endif

#include "cmsis/LPC407x_8x_177x_8x.h"
#include "../cortex_m.h"
#include "mcu_lpc_core.h"

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
} lpc407x_8x_tmr_action_channel_t;

typedef lpc407x_8x_tmr_action_channel_t tmr_action_channel_t;



#include "mcu_lpcxx7x_8x.h"
#include "mcu_lpc_dma.h"


#endif /* LPC407X_8X_H_ */

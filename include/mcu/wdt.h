// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef WDT_H_
#define WDT_H_

#include <sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  WDT_MODE_RESET = (1 << 0),
  WDT_MODE_INTERRUPT = (1 << 1),
  WDT_MODE_CLK_SRC_INTERNAL_RC = (0 << 2),
  WDT_MODE_CLK_SRC_MAIN = (1 << 2),
  WDT_MODE_CLK_SRC_RTC = (2 << 2),
  WDT_MODE_CLK_SRC_WDT_OSC = (3 << 2)
} wdt_mode_t;

#define WDT_MODE_CLK_SRC_MASK (0x07 << 2)

int mcu_wdt_init(int mode, int interval) MCU_ROOT_CODE;
void mcu_wdt_reset();
void mcu_wdt_root_reset(void *args) MCU_ROOT_CODE;
int mcu_wdt_setaction(int (*action)(const void *, void *)) MCU_ROOT_CODE;
int mcu_wdt_setinterval(int interval) MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif

#endif /* WDT_H_ */

/*! @} */

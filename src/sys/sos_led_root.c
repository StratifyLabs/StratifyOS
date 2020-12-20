// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "cortexm/cortexm.h"
#include "mcu/core.h"
#include "mcu/pio.h"
#include "sos/sos.h"
#include "sos/sos_config.h"

void sos_led_svcall_enable(void *args) {
  CORTEXM_SVCALL_ENTER();
  MCU_UNUSED_ARGUMENT(args);
  sos_led_root_enable();
}

void sos_led_root_enable() {
  if (sos_config.debug.enable_led) {
    sos_config.debug.enable_led();
  }
}

void sos_led_svcall_disable(void *args) {
  CORTEXM_SVCALL_ENTER();
  MCU_UNUSED_ARGUMENT(args);
  sos_led_root_disable();
}

void sos_led_root_disable() {
  if (sos_config.debug.disable_led) {
    sos_config.debug.disable_led();
  }
}

void sos_led_svcall_error(void *args) {
  CORTEXM_SVCALL_ENTER();
  MCU_UNUSED_ARGUMENT(args);
  sos_led_root_error();
}

void sos_led_root_error() {
  while (1) {
    sos_led_root_enable();
    cortexm_delay_ms(50);
    sos_led_root_disable();
    cortexm_delay_ms(50);
  }
}

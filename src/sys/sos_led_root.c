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
  sos_config.event_handler(SOS_EVENT_ROOT_ENABLE_DEBUG_LED, NULL);
}

void sos_led_svcall_disable(void *args) {
  CORTEXM_SVCALL_ENTER();
  MCU_UNUSED_ARGUMENT(args);
  sos_led_root_disable();
}

void sos_led_root_disable() {
  sos_config.event_handler(SOS_EVENT_ROOT_DISABLE_DEBUG_LED, NULL);
}

void sos_led_svcall_error(void *args) {
  CORTEXM_SVCALL_ENTER();
  MCU_UNUSED_ARGUMENT(args);
  sos_led_root_error();
}

void sos_led_root_error() {
  while (1) {
    sos_led_svcall_enable(0);
    cortexm_delay_ms(50);
    sos_led_svcall_disable(0);
    cortexm_delay_ms(50);
  }
}

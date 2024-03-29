// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "cortexm/cortexm.h"
#include "sos/led.h"
#include "sos/sos.h"

void sos_led_startup() {
  const int factor = 10;
  int duty;
  for (int i = 0; i < 100; i++) {
    duty = i * factor;
    cortexm_svcall(sos_led_svcall_enable, 0);
    usleep(duty);
    cortexm_svcall(sos_led_svcall_disable, 0);
    usleep(100 * factor - duty);
  }

  for (int i = 0; i < 100; i++) {
    duty = i * factor;
    cortexm_svcall(sos_led_svcall_enable, 0);
    usleep(100 * factor - duty);
    cortexm_svcall(sos_led_svcall_disable, 0);
    usleep(duty);
  }
}

void sos_led_boot_startup(){
  for(int i = 0; i < 3; i++){
    cortexm_svcall(sos_led_svcall_enable, 0);
    usleep(100000);
    cortexm_svcall(sos_led_svcall_disable, 0);
    usleep(100000);
  }
}


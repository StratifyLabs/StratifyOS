// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "device/led_pwm.h"
#include "mcu/pio.h"
#include <errno.h>
#include <stddef.h>

#include "sos/debug.h"

static int led_setattr(const devfs_handle_t *handle, const led_attr_t *attr);

int led_pwm_open(const devfs_handle_t *handle) { return mcu_pwm_open(handle); }

int led_pwm_ioctl(const devfs_handle_t *handle, int request, void *ctl) {
  led_info_t *info = ctl;
  switch (request) {
  case I_LED_GETINFO:
    info->o_flags = LED_FLAG_ENABLE | LED_FLAG_DISABLE | LED_FLAG_IS_DUTY_CYCLE;
    info->o_events = 0;
    break;

  case I_LED_SETATTR:
    return led_setattr(handle, ctl);

  case I_LED_SETACTION:
    return SYSFS_SET_RETURN(ENOTSUP);

  default:
    return SYSFS_SET_RETURN(EINVAL);
  }

  return 0;
}

int led_pwm_read(const devfs_handle_t *handle, devfs_async_t *rop) {
  return SYSFS_SET_RETURN(ENOTSUP);
}

int led_pwm_write(const devfs_handle_t *handle, devfs_async_t *wop) {
  return SYSFS_SET_RETURN(ENOTSUP);
}

int led_pwm_close(const devfs_handle_t *handle) { return mcu_pwm_close(handle); }

int led_setattr(const devfs_handle_t *handle, const led_attr_t *attr) {
  const led_pwm_config_t *config = handle->config;
  u32 o_flags = attr->o_flags;
  pwm_attr_t pwm_attr;
  mcu_channel_t channel;

  if (o_flags & LED_FLAG_ENABLE) {
    if (o_flags & LED_FLAG_IS_DUTY_CYCLE) {
      pwm_attr.period = attr->period;
      channel.value = attr->duty_cycle;
    } else {
      pwm_attr.period = 5000;
      channel.value = 5000;
    }
    pwm_attr.freq = 1000000;
    // led attr period is in us - so pwm freq will be 1MHz
    memcpy(
      &(pwm_attr.pin_assignment), &config->pwm.attr.pin_assignment,
      sizeof(pwm_pin_assignment_t));
    pwm_attr.o_flags = PWM_FLAG_SET_TIMER | PWM_FLAG_IS_ENABLED;

    if (mcu_pwm_setattr(handle, &pwm_attr) < 0) {
      return -1;
    }

    channel.loc = config->loc;
    if (mcu_pwm_setchannel(handle, &channel) < 0) {
      return -1;
    }

  } else if (o_flags & LED_FLAG_DISABLE) {

    channel.loc = config->loc;
    channel.value = 0;

    if (mcu_pwm_setchannel(handle, &channel) < 0) {
      return -1;
    }
  }
  return 0;
}

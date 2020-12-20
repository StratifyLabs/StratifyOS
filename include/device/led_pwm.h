// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEVICE_LED_PWM_H_
#define DEVICE_LED_PWM_H_

#include "mcu/pwm.h"
#include "sos/dev/led.h"
#include "sos/fs/devfs.h"


enum {
	LED_PWM_CONFIG_FLAG_IS_ACTIVE_HIGH = (1<<0),
};

typedef struct MCU_PACK {
	pwm_config_t pwm;
	u32 loc; //location of the channel
	u32 o_flags;
} led_pwm_config_t;

int led_pwm_open(const devfs_handle_t * handle);
int led_pwm_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int led_pwm_read(const devfs_handle_t * handle, devfs_async_t * rop);
int led_pwm_write(const devfs_handle_t * handle, devfs_async_t * wop);
int led_pwm_close(const devfs_handle_t * handle);


#endif /* DEVICE_LED_PWM_H_ */

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

#include <errno.h>
#include <stddef.h>
#include "mcu/pio.h"
#include "device/led_pwm.h"

#include "mcu/debug.h"

static void led_setattr(const devfs_handle_t * handle, const led_attr_t * attr);

int led_pwm_open(const devfs_handle_t * handle){
	return 0;
}

int led_pwm_ioctl(const devfs_handle_t * handle, int request, void * ctl){
	led_info_t * info = ctl;
	switch(request){
	case I_LED_GETINFO:
		info->o_flags = LED_FLAG_ENABLE | LED_FLAG_DISABLE | LED_FLAG_IS_HIGH_IMPEDANCE;
		info->o_events = 0;
		break;

	case I_LED_SETATTR:
		led_setattr(handle, ctl);
		break;

	case I_LED_SETACTION:
		errno = ENOTSUP;
		return -1;

	default:
		errno = EINVAL;
		return -1;

	}

	return 0;
}

int led_pwm_read(const devfs_handle_t * handle, devfs_async_t * rop){
	errno = ENOTSUP;
	return -1;
}

int led_pwm_write(const devfs_handle_t * handle, devfs_async_t * wop){
	errno = ENOTSUP;
	return -1;
}

int led_pwm_close(const devfs_handle_t * handle){
	return 0;
}

void led_setattr(const devfs_handle_t * handle, const led_attr_t * attr){
	const led_pwm_config_t * config = handle->config;
	u32 o_flags = attr->o_flags;
	pio_attr_t pio_attr;
	mcu_pin_t pin;
	devfs_handle_t pio_handle;
	pwm_attr_t pwm_attr;
	mcu_channel_t channel;

	pin = config->pwm.attr.pin_assignment.channel[0];
	pio_handle.port = pin.port;
	pio_handle.config = 0;


	if( o_flags & LED_FLAG_ENABLE ){

		if( o_flags & LED_FLAG_IS_DUTY_CYCLE ){

			//led attr period is in us - so pwm freq will be 1MHz
			memset(&(pwm_attr.pin_assignment), 0xff, sizeof(pwm_pin_assignment_t));
			pwm_attr.o_flags = PWM_FLAG_IS_ACTIVE_HIGH;
			pwm_attr.pin_assignment.channel[0] = pin;
			pwm_attr.freq = 1000000;
			pwm_attr.period = attr->period;
			mcu_pwm_setattr(handle, &pwm_attr);

			channel.loc = config->loc;
			channel.value = attr->duty_cycle;
			mcu_pwm_set(handle, &channel);

		} else {
			//just use PIO
			pio_attr.o_flags = PIO_FLAG_SET_OUTPUT;
			mcu_pio_setattr(&pio_handle, &pio_attr);

			if( config->o_flags & LED_PWM_CONFIG_FLAG_IS_ACTIVE_HIGH ){
				mcu_pio_setmask(&pio_handle, (void*)pio_attr.o_pinmask);
			} else {
				mcu_pio_clrmask(&pio_handle, (void*)pio_attr.o_pinmask);
			}

		}

	} else if( o_flags & LED_FLAG_DISABLE ){

		if( o_flags & LED_FLAG_IS_HIGH_IMPEDANCE ){
			pio_attr.o_flags = PIO_FLAG_SET_INPUT | PIO_FLAG_IS_FLOAT;
			mcu_pio_setattr(&pio_handle, &pio_attr);
		} else {

			pio_attr.o_flags = PIO_FLAG_SET_OUTPUT;
			mcu_pio_setattr(&pio_handle, &pio_attr);

			//set high or low based on IS_ACTIVE_HIGH config values
			if( config->o_flags & LED_PWM_CONFIG_FLAG_IS_ACTIVE_HIGH ){

			}

		}
	}
}



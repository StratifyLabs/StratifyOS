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
#include "iface/device_config.h"
#include "mcu/pwm.h"


/* \details This function checks to see if the PWM
 * port is configured to use the channels specified
 * by the configuration.  If the PWM port is closed,
 * it is opened and configured using this function.
 * \return Zero if the port is ready to use; an
 * error code if it could not be configured.
 */
int mcu_check_pwm_port(const device_cfg_t * cfgp){
	int err;
	int i;
	uint16_t enabled_channels;
	pwm_attr_t pwm_cfg;

	enabled_channels = 0;
	for(i=0; i < DEVICE_MAX_PWM_CHANNELS; i++){
		if ( cfgp->pcfg.pwm.channels[i] != -1 ){
			if ( cfgp->pcfg.pwm.channels[i] < DEVICE_MAX_PWM_CHANNELS ){
				enabled_channels |= (1<<cfgp->pcfg.pwm.channels[i]);
			}
		}
	}

	err = mcu_pwm_ioctl(cfgp, I_PWM_GETATTR, &pwm_cfg);
	if ( err == 0 ){ //port is already open
		//Check the port configuration

		if ( pwm_cfg.pin_assign != cfgp->pin_assign ){
			errno = EINVAL;
			return -1;
		}

		if ( pwm_cfg.freq != cfgp->pcfg.pwm.freq ){
			errno = EINVAL;
			return -1;
		}

		if ( pwm_cfg.top != cfgp->pcfg.pwm.top ){
			errno = EINVAL;
			return -1;
		}

		//Now enable the channels
		pwm_cfg.enabled_channels |= enabled_channels;
		if ( (err = mcu_pwm_ioctl(cfgp, I_PWM_SETATTR, &pwm_cfg) ) ){
			return err;
		}

	} else {
		//Open the PWM port
		pwm_cfg.pin_assign = cfgp->pin_assign;
		pwm_cfg.enabled_channels = enabled_channels;
		pwm_cfg.freq = cfgp->pcfg.pwm.freq;
		pwm_cfg.top = cfgp->pcfg.pwm.top;

		err = mcu_pwm_open(cfgp);
		if ( err ){
			return err;
		}
		return mcu_pwm_ioctl(cfgp, I_PWM_SETATTR, &pwm_cfg);
	}

	return 0;
}

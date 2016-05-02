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
#include "mcu/i2c.h"



/* \details This function checks to see if the ADC
 * port is configured to use the channels specified
 * by the configuration.  If the ADC port is closed,
 * it is opened and configured using this function.
 * \return Zero if the port is ready to use; an
 * error code if it could not be configured.
 */
int mcu_check_i2c_port(const device_cfg_t * cfgp){
	int err;
	i2c_attr_t i2c_cfg;


	err = mcu_i2c_ioctl(cfgp, I_I2C_GETATTR, &i2c_cfg);

	if ( err == 0 ){ //port is already open
		//Check the port configuration

		if ( i2c_cfg.pin_assign != cfgp->pin_assign ){
			errno = EINVAL;
			return -1;
		}

		if ( i2c_cfg.bitrate > cfgp->bitrate ){
			errno = EINVAL;
			return -1;
		}

	} else {
		//Open the ADC port
		i2c_cfg.pin_assign = cfgp->pin_assign;
		i2c_cfg.bitrate = cfgp->bitrate;
		err = mcu_i2c_open(cfgp);
		if ( err < 0 ){
			return err;
		}
		return mcu_i2c_ioctl(cfgp, I_I2C_SETATTR, &i2c_cfg);
	}

	return 0;
}


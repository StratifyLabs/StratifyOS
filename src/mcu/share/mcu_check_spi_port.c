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
#include "mcu/debug.h"
#include "mcu/spi.h"

/* \details This function checks to see if a SPI port is opened
 * and configured in a way that is compatible with the
 * configuration specified.  If the SPI port is not
 * already open, this function will open it with
 * the configuration specified.
 * \return 0 if the port is ready to use or an error
 * code if the port is already open with
 * incompatible settings.
 */
int mcu_check_spi_port(const device_cfg_t * cfgp){
	int err;
	spi_attr_t spi_cfg;

	//check to see if the SPI port is already configured
	err = mcu_spi_ioctl(cfgp, I_SPI_GETATTR, &spi_cfg);
	if ( err == 0 ){
		if ( spi_cfg.pin_assign != cfgp->pin_assign ){
			errno = EINVAL;
			return -1 - offsetof(spi_attr_t, pin_assign);
		}

		if ( spi_cfg.width != cfgp->pcfg.spi.width ){
			errno = EINVAL;
			return -1 - offsetof(spi_attr_t, width);
		}

		if ( spi_cfg.mode != cfgp->pcfg.spi.mode ){
			errno = EINVAL;
			return -1 - offsetof(spi_attr_t, mode);
		}

		if ( spi_cfg.format != cfgp->pcfg.spi.format ){
			errno = EINVAL;
			return -1 - offsetof(spi_attr_t, format);
		}

		if ( spi_cfg.bitrate > cfgp->bitrate ){
			errno = EINVAL;
			return -1 - offsetof(spi_attr_t, bitrate);
		}

	} else {
		//Open the port with the desired configuration
		spi_cfg.pin_assign = cfgp->pin_assign;
		spi_cfg.width = cfgp->pcfg.spi.width;
		spi_cfg.mode = cfgp->pcfg.spi.mode;
		spi_cfg.format = cfgp->pcfg.spi.format;
		spi_cfg.bitrate = cfgp->bitrate;
		spi_cfg.master = SPI_ATTR_MASTER;
		err = mcu_spi_open(cfgp);
		if ( err < 0 ){
			return err;
		}

		if( (err = mcu_spi_ioctl(cfgp, I_SPI_SETATTR, &spi_cfg)) < 0 ){
			return err;
		}
		return 0;
	}

	return 0;
}

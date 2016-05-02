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
#include "mcu/mcu.h"
#include "mcu/uart.h"

/* \details This function checks to see if a UART port is opened
 * and configured in a way that is compatible with the
 * configuration specified.  If the UART port is not
 * already open, this function will open it with
 * the configuration specified.
 * \return 0 if the port is ready to use or an error
 * code if the port is already open with
 * incompatible settings.
 */
int mcu_check_uart_port(const device_cfg_t * cfgp){
	int err;
	uart_attr_t attr;

	//check to see if the UART port is already configured
	err = mcu_uart_ioctl(cfgp, I_UART_GETATTR, &attr);
	if ( err == 0 ){

		if ( attr.pin_assign != cfgp->pin_assign ){
			errno = EINVAL;
			return -1;
		}

		if ( attr.width != cfgp->pcfg.uart.width ){
			errno = EINVAL;
			return -1;
		}

		if ( attr.baudrate != cfgp->bitrate ){
			errno = EINVAL;
			return -1;
		}

		if ( attr.parity != cfgp->pcfg.uart.parity ){
			errno = EINVAL;
			return -1;
		}

		if ( attr.stop != cfgp->pcfg.uart.stop_bits ){
			errno = EINVAL;
			return -1;
		}


	} else {
		//Open the port with the desired configuration
		attr.pin_assign = cfgp->pin_assign;
		attr.width = cfgp->pcfg.spi.width;
		attr.baudrate = cfgp->bitrate;
		attr.stop = cfgp->pcfg.uart.stop_bits;
		attr.parity = cfgp->pcfg.uart.parity;
		err = mcu_uart_open(cfgp);
		if ( err < 0 ){
			return err;
		}

		return mcu_uart_ioctl(cfgp, I_UART_SETATTR, &attr);
	}

	return 0;
}

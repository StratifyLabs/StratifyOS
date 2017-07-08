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


#include <stdio.h>
#include "mcu/usb_dev.h"

#include "mcu/core.h"
#include "mcu/usb.h"

#if defined __lpc13uxx
extern int mcu_usb_rd_setup_ep(int port, u32 endpoint_num, void * dest);
#endif


char htoc(int nibble){
	if ( nibble >= 0 && nibble < 10 ){
		return nibble + '0';
	} else {
		return nibble + 'A' - 10;
	}
}

/*! \details This function reads the setup packet as part of the setup stage.
 */
void usb_dev_std_setup_stage(usb_dev_context_t * context){
#if defined __lpc13uxx
	mcu_usb_rd_setup_ep(context->constants->port, 0x00, (uint8_t *)&(context->setup_pkt));
#else
	mcu_usb_rd_ep(context->constants->port, 0x00, (uint8_t *)&(context->setup_pkt));
#endif
}


/*! \details
 */

void usb_dev_std_datain_stage(usb_dev_context_t * context) {
	u32 cnt;
	if (context->ep0_data.cnt > mcu_board_config.usb_max_packet_zero) {
		cnt = mcu_board_config.usb_max_packet_zero;
	} else {
		cnt = context->ep0_data.cnt;
	}
	cnt = mcu_usb_wr_ep(context->constants->port, 0x80, context->ep0_data.dptr, cnt);
	context->ep0_data.dptr += cnt;
	context->ep0_data.cnt -= cnt;
}


/*! \details
 */
void usb_dev_std_dataout_stage (usb_dev_context_t * context){
	u32 cnt;
	cnt = mcu_usb_rd_ep(context->constants->port, 0x00, context->ep0_data.dptr);
	context->ep0_data.dptr += cnt;
	context->ep0_data.cnt -= cnt;
}


/*! \details
 */
void usb_dev_std_statusin_stage (usb_dev_context_t * context){
	mcu_usb_wr_ep(context->constants->port, 0x80, NULL, 0);
}

/*! \details
 */
void usb_dev_std_statusout_stage (usb_dev_context_t * context){
	mcu_usb_rd_ep(context->constants->port, 0x00, context->ep0_buf);
}





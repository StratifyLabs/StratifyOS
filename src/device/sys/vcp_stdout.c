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

#include <fcntl.h>
#include <errno.h>
#include <stddef.h>
#include "dev/vcp_stdout.h"
#include "iface/dev/usbfifo.h"
#include "mcu/usb.h"
#include "mcu/debug.h"
#include "stratify/usb_dev_typedefs.h"
#include "stratify/usb_dev_cdc.h"

int vcp_stdout_open(const device_cfg_t * cfg){
	return fifo_open(cfg);
}

int vcp_stdout_ioctl(const device_cfg_t * cfg, int request, void * ctl){
	return fifo_ioctl(cfg, request, ctl);
}

int vcp_stdout_read(const device_cfg_t * cfg, device_transfer_t * rop){
	return fifo_read(cfg, rop);
}

int vcp_stdout_write(const device_cfg_t * cfg, device_transfer_t * wop){
	int ret;
	device_cfg_t usb_port;
	char buffer[32];
	const vcp_stdout_cfg_t * cfgp = (const vcp_stdout_cfg_t*)cfg->dcfg;
	vcp_stdout_state_t * state = cfg->state;

	//send a SERIAL_STATE packet out on the interrupt endpoint
	state->int_op.loc = cfgp->usb_interrupt_endpoint;
	state->int_op.buf = &(state->serial_state);
	state->int_op.nbyte = sizeof(usb_cdc_acm_notification_serialstate_t);
	state->int_op.tid = wop->tid;
	state->int_op.callback = 0;
	state->int_op.context = 0;
	state->int_op.flags = O_NONBLOCK | O_RDWR;

	state->serial_state.bmRequestType = 0xA1;
	state->serial_state.bRequest = SERIAL_STATE;
	state->serial_state.wValue = 0;
	state->serial_state.wIndex = cfgp->usb_interface;
	state->serial_state.wLength = 2;

	state->serial_state.serial_state ^= (SERIAL_STATE_RX_CARRIER | SERIAL_STATE_TX_CARRIER | SERIAL_STATE_RING);


	usb_port.periph.port = cfgp->usb_port;
	ret = mcu_usb_write(&usb_port, &(state->int_op));

	sprintf(buffer, "Write Int status %d %d %d 0x%X %d\n", ret, errno,
			cfgp->usb_port,
			cfgp->usb_interrupt_endpoint,
			cfgp->usb_interface);

	wop->cbuf = buffer;
	wop->nbyte = strlen(buffer);

	return fifo_write(cfg, wop);
}

int vcp_stdout_close(const device_cfg_t * cfg){
	//use I_FIFO_EXIT to close the USB
	return 0;
}


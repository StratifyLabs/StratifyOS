/* Copyright 2011-2018 Tyler Gilbert;
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
#include "mcu/mcu.h"
#include "mcu/core.h"

void mcu_board_execute_event_handler(int event, void * args){
	if( mcu_board_config.event_handler != 0 ){
		mcu_board_config.event_handler(event, args);
	}
}

//deprecated -- stop using this -- this is the old way -- use devfs_execute_event_handler()
int mcu_execute_event_handler(mcu_event_handler_t * handler, u32 o_events, void * data){
	int ret = 0;
	mcu_event_t event;
	mcu_callback_t callback;
	if( handler->callback ){
		event.o_events = o_events;
		event.data = data;
		callback = handler->callback;
		handler->callback = 0; //the callback might want to read/write the device and callback needs to be NULL to allow that
		ret = callback(handler->context, &event);
		if( ret != 0 ){
			handler->callback = callback; //If the callback returns non-zero, re-instate the same callback
		}
	}
	return ret;
}

const void * mcu_select_attr(const devfs_handle_t * handle, void * ctl){
	if( ctl == 0 ){
		if( handle->config != 0 ){
			return handle->config;
		}
	}
	return ctl;
}

const void * mcu_select_pin_assignment(const void * attr_pin_assignment,
													const void * config_pin_assignment,
													int count){
	int i;
	int is_default = 1;
	const void * pin_assignment;
	for(i=0; i < count; i++){
		const mcu_pin_t * pin = mcu_pin_at(attr_pin_assignment, i);
		if( mcu_is_port_valid(pin->port) ){
			is_default = 0;
			break;
		}
	}

	if( is_default && (config_pin_assignment != 0) ){
		pin_assignment = config_pin_assignment;
	} else {
		pin_assignment = attr_pin_assignment;
	}

	return pin_assignment;
}

int mcu_set_pin_assignment(
		const void * attr_pin_assignment,
		const void * config_pin_assignment,
		int count,
		int periph,
		int periph_port,
		void (*pre_configure_pin)(const mcu_pin_t *, void*),
		void (*post_configure_pin)(const mcu_pin_t *, void*),
		void * arg){
	int i;
	const void * pin_assignment;

	pin_assignment = mcu_select_pin_assignment(attr_pin_assignment, config_pin_assignment, count);

	for(i=0; i < count; i++){
		const mcu_pin_t * pin = mcu_pin_at(pin_assignment, i);
		if( mcu_is_port_valid(pin->port) ){

			if( pre_configure_pin ){
				pre_configure_pin(pin,arg);
			}

			if ( mcu_core_set_pinsel_func(pin, periph, periph_port) < 0 ){
				return SYSFS_SET_RETURN_WITH_VALUE(EINVAL, i);
			}

			if( post_configure_pin ){
				post_configure_pin(pin,arg);
			}
        }
	}
	return 0;
}


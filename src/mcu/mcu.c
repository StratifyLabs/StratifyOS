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

int mcu_execute_transfer_handler(mcu_event_handler_t * handler, u32 o_events, void * data);

void mcu_board_execute_event_handler(int event, void * args){
	if( mcu_board_config.event_handler != 0 ){
		mcu_board_config.event_handler(event, args);
	}
}

//execute the read/write transfer handlers if they are available
void mcu_execute_transfer_handlers(devfs_transfer_handler_t * transfer_handler, void * data, int nbyte, u32 o_flags){
    if( transfer_handler->read ){
        devfs_async_t * async = transfer_handler->read;
        transfer_handler->read = 0;
        async->nbyte = nbyte;
        mcu_execute_transfer_handler(&async->handler, o_flags | MCU_EVENT_FLAG_DATA_READY, data);
    }

    if( transfer_handler->write ){
        devfs_async_t * async = transfer_handler->write;
        transfer_handler->write = 0;
        async->nbyte = nbyte;
        mcu_execute_transfer_handler(&async->handler, o_flags | MCU_EVENT_FLAG_WRITE_COMPLETE, data);
    }
}

//execute when a read completes successfully
int mcu_execute_read_handler(devfs_transfer_handler_t * transfer_handler, void * data, int nbyte){
    return mcu_execute_read_handler_with_flags(transfer_handler, data, nbyte, MCU_EVENT_FLAG_DATA_READY);
}

//executes when a read completes unsuccessfully
int mcu_execute_read_handler_with_flags(devfs_transfer_handler_t * transfer_handler, void * data, int nbyte, u32 o_flags){
    if( transfer_handler->read ){
        devfs_async_t * async = transfer_handler->read;
        transfer_handler->read = 0;
        if( nbyte ){ async->nbyte = nbyte; }
        return mcu_execute_transfer_handler(&async->handler, o_flags, data);
    }
    return 0;
}

//execute when a write completes successfully
int mcu_execute_write_handler(devfs_transfer_handler_t * transfer_handler, void * data, int nbyte){
    return mcu_execute_write_handler_with_flags(transfer_handler, data, nbyte, MCU_EVENT_FLAG_WRITE_COMPLETE);
}

//executes when a write completes unsuccessfully
int mcu_execute_write_handler_with_flags(devfs_transfer_handler_t * transfer_handler, void * data, int nbyte, u32 o_flags){
    if( transfer_handler->write ){
        devfs_async_t * async = transfer_handler->write;
        transfer_handler->write = 0;
        if( nbyte ){ async->nbyte = nbyte; }
        return mcu_execute_transfer_handler(&async->handler, o_flags, data);
    }
    return 0;
}

//used to execute any handler
int mcu_execute_transfer_handler(mcu_event_handler_t * handler, u32 o_events, void * data){
    int ret = 0;
    mcu_event_t event;
    if( handler->callback ){
        event.o_events = o_events;
        event.data = data;
        ret = handler->callback(handler->context, &event);
    }
    return ret;
}

//deprecated -- stop using this -- this is the old way -- use mcu_execute_transfer_event_handler()
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
			handler->callback = callback; //If the callback returns non-zero, re-instate the callback
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


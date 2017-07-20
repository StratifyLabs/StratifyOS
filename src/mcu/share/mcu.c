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


#include "mcu/mcu.h"

void mcu_board_execute_event_handler(int event, void * args){
	if( mcu_board_config.event_handler != 0 ){
		mcu_board_config.event_handler(event, args);
	}
}


int mcu_execute_event_handler(mcu_event_handler_t * handler, u32 o_events, void * data){
	int ret = 0;
	mcu_event_t event;
	if( handler->callback ){
		event.o_events = o_events;
		event.data = data;
		ret = handler->callback(handler->context, &event);
		if( ret == 0 ){
			handler->callback = 0;
		}
	}
	return ret;
}


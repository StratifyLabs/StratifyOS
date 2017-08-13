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

#include "link_local.h"

int link_debug_level = 0;

void (*link_debug_write_callback)(link_debug_context_t*) = 0;


void link_set_debug(int debug_level){
	link_debug_level = debug_level;
}

void link_set_debug_callback(void (*write_callback)(link_debug_context_t*)){
	link_debug_write_callback = write_callback;
}

void link_write_debug_message(link_debug_context_t * context){
	if( link_debug_write_callback != 0 ){
		link_debug_write_callback(context);
	}

	if( context->type <= link_debug_level ){
		printf("%s-%s\n", context->function, context->msg);
		fflush(stdout);
	}

}


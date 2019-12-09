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

void link_set_debug(int debug_level){
	link_debug_level = debug_level;
}


int link_debug_printf(
		int x,
		const char * function,
		int line,
		const char * fmt,
		...){

	if( x <= link_debug_level ){

		switch(x){
			case LINK_DEBUG_FATAL: printf("FATAL:"); break;
			case LINK_DEBUG_CRITICAL: printf("CRITICAL:"); break;
			case LINK_DEBUG_WARNING: printf("WARNING:"); break;
			case LINK_DEBUG_INFO: printf("INFO:"); break;
			case LINK_DEBUG_MESSAGE: printf("MESSAGE:"); break;
			case LINK_DEBUG_DEBUG: printf("DEBUG:"); break;
		}

		printf("%s():%d -> ", function, line);

		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end (args);

		printf("\n");
		fflush(stdout);

		return 1;
	}

	return 0;
}




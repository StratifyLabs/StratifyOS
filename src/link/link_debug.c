/*
 * link_debug.c
 *
 *  Created on: Jul 16, 2014
 *      Author: tgil
 */


#include "link_flags.h"

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
	} else {

		if( context->type <= link_debug_level ){
			printf("%s-%s\n", context->function, context->msg);
			fflush(stdout);
		}

	}
}


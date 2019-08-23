
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sos/link/transport.h"

#include "sos/sos.h"
#include	"mcu/debug.h"

void transport_svcall_fatal(void * args){
	CORTEXM_SVCALL_ENTER();
	mcu_board_config.event_handler(MCU_BOARD_CONFIG_EVENT_ROOT_FATAL, args);
}

int link_transport_slaveread(
		link_transport_driver_t * driver,
		void * buf,
		int nbyte,
		int (*callback)(void*,void*,int),
		void * context
		){
	//get sos board config for link transport
	if( driver->transport_read == 0 ){
		cortexm_svcall(transport_svcall_fatal, "transport read");
	}
	return driver->transport_read(driver, buf, nbyte, callback, context);
}

int link_transport_slavewrite(
		link_transport_driver_t * driver,
		const void * buf, int nbyte,
		int (*callback)(void*,void*,int),
		void * context
		){
	//get sos board config for link transport
	if( driver->transport_write == 0 ){
		cortexm_svcall(transport_svcall_fatal, "transport write");
	}
	return driver->transport_write(driver, buf, nbyte, callback, context);
}


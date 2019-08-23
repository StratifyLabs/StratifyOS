
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sos/link.h"
#include "sos/fs/sysfs.h"

#ifdef __win32
#define TIMEOUT_VALUE 500
#else
#define INITIAL_TIMEOUT_VALUE 500 //used for first ping
#define TIMEOUT_VALUE 5000 //used for more complex operations
#endif

#define INITIAL_TIMEOUT_VALUE 500 //used for first ping


#define pkt_checksum(pktp) ((pktp)->data[(pktp)->size])

static int wait_ack(link_transport_mdriver_t * driver, uint8_t checksum, int timeout);
static int m_timeout_value = TIMEOUT_VALUE;
static int resolve_protocol(link_transport_mdriver_t * driver);

void link_transport_mastersettimeout(link_transport_mdriver_t * driver, int t){
	if( resolve_protocol(driver) < 0 ){
		return;
	}

	if( driver->transport_version == 1 ){
		return link1_transport_mastersettimeout(driver, t);
	}

	if( driver->transport_version == 2 ){
		return link2_transport_mastersettimeout(driver, t);
	}

}

int link_transport_masterread(link_transport_mdriver_t * driver, void * buf, int nbyte){

	if( resolve_protocol(driver) < 0 ){
		return LINK_PROT_ERROR;
	}

	if( driver->transport_version == 1 ){
		return link1_transport_masterread(driver, buf, nbyte);
	}

	if( driver->transport_version == 2 ){
		return link2_transport_masterread(driver, buf, nbyte);
	}

	return LINK_PROT_ERROR;
}

int link_transport_masterwrite(link_transport_mdriver_t * driver, const void * buf, int nbyte){
	if( resolve_protocol(driver) < 0 ){
		return LINK_PROT_ERROR;
	}

	if( driver->transport_version == 1 ){
		return link1_transport_masterwrite(driver, buf, nbyte);
	}

	if( driver->transport_version == 2 ){
		return link2_transport_masterwrite(driver, buf, nbyte);
	}

	return LINK_PROT_ERROR;
}

int resolve_protocol(link_transport_mdriver_t * driver){
	if( driver->transport_version == 0 ){
		//need to do protocol resolution starting with link1
		int result = link1_transport_masterwrite(driver, 0, 0);
		if( result == 0 ){
			//printf("------------------- Resolved to Link1 -------------------\n");
			driver->transport_version = 1;
		} else {
			int nack = SYSFS_GET_RETURN_ERRNO(result);
			if( nack == LINK2_PACKET_NACK ){
				//printf("------------------- Resolved to Link2 -------------------\n");
				driver->transport_version = 2;
			} else {
				//printf("------------------- Not Resolved -------------------\n");
				return -1;
			}
		}
	}

	if( driver->transport_version > 2 ){
		driver->transport_version = 0;
		return -1;
	}

	return driver->transport_version;
}
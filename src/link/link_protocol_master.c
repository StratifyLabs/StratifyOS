
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "iface/link.h"

#include "link_flags.h"

#include "mcu/debug.h"


#ifdef __win32
#define TIMEOUT_VALUE 500
#else
#define TIMEOUT_VALUE 2000
#endif

#if defined __link
#define link_phy_read link_driver()->read
#define link_phy_write link_driver()->write
#define link_phy_wait link_driver()->wait
#define link_phy_flush link_driver()->flush
#endif


#define pkt_checksum(pktp) ((pktp)->data[(pktp)->size])

static int wait_ack(link_transport_phy_t handle, uint8_t checksum, int timeout);
static int timeout_value = TIMEOUT_VALUE;

void link_transport_mastersettimeout(int t){
	if ( t == 0 ){
		timeout_value = TIMEOUT_VALUE;
	} else {
		timeout_value = t;
	}
}

int link_transport_masterread(link_transport_phy_t handle, void * buf, int nbyte){
	link_pkt_t pkt;
	char * p;
	int bytes;
	int err;

	bytes = 0;
	p = buf;
	do {

		if( (err = link_transport_wait_start(handle, &pkt, timeout_value)) < 0 ){
			link_phy_flush(handle);
			return err;
		}

		if( (err = link_transport_wait_packet(handle, &pkt, timeout_value)) < 0 ){
			link_phy_flush(handle);
			return err;
		}

		//a packet has arrived -- checksum it
		if( link_transport_checksum_isok(&pkt) == false ){
			return LINK_PROT_ERROR;
		}

		//callback to handle incoming data as it arrives
		//copy the valid data to the buffer
		memcpy(p, pkt.data, pkt.size);
		bytes += pkt.size;
		p += pkt.size;

	} while( (bytes < nbyte) && (pkt.size == LINK_PACKET_DATA_SIZE));

	return bytes;
}

int link_transport_masterwrite(link_transport_phy_t handle, const void * buf, int nbyte){
	link_pkt_t pkt;
	char * p;
	int bytes;
	int err;

	bytes = 0;
	p = (void*)buf;
	pkt.start = LINK_PACKET_START;
	do {

		if( (nbyte - bytes) > LINK_PACKET_DATA_SIZE ){
			pkt.size = LINK_PACKET_DATA_SIZE;
		} else {
			pkt.size = nbyte - bytes;
		}

		memcpy(pkt.data, p, pkt.size);

		link_transport_insert_checksum(&pkt);

		//send packet
		if( link_phy_write(handle, &pkt, pkt.size + LINK_PACKET_HEADER_SIZE) != (pkt.size + LINK_PACKET_HEADER_SIZE) ){
			link_error("Link PHY write failed");
			return LINK_PHY_ERROR;
		}

		//received ack of the checksum
		if( (err = wait_ack(handle, pkt_checksum(&pkt), timeout_value)) < 0 ){
			link_error("wait ack failed");
			link_phy_flush(handle);
			return err;
		}

		bytes += pkt.size;
		p += pkt.size;

	} while( (bytes < nbyte) && (pkt.size == LINK_PACKET_DATA_SIZE) );

	return bytes;
}


int wait_ack(link_transport_phy_t handle, uint8_t checksum, int timeout){
	link_ack_t ack;
	char * p;
	int count;
	int bytes_read;
	int ret;

	count = 0;
	p = (char*)&ack;
	bytes_read = 0;
	do {
		ret = link_phy_read(handle, p, sizeof(ack) - bytes_read);
		if( ret < 0 ){
			link_error("read failed");
			return LINK_PHY_ERROR;
		}

		if( ret > 0 ){
			link_debug(LINK_DEBUG_MESSAGE, "Got %d bytes", ret);
		}
		if( ret > 0 ){
			bytes_read += ret;
			p += ret;
			count = 0;
		} else {
//#ifndef __WINDOWS
			link_phy_wait(1);
//#endif
			count+=1;
			if( count >= timeout ){
				link_error("timeout");
				return LINK_PROT_ERROR;
			}
		}
	} while(bytes_read < sizeof(ack));


	if( ack.checksum != checksum ){
		return LINK_PROT_ERROR;
	}

	return ack.ack;
}


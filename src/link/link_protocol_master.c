
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "link_local.h"

#include "mcu/debug.h"


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

void link_transport_mastersettimeout(int t){
	if ( t == 0 ){
		m_timeout_value = TIMEOUT_VALUE;
	} else {
		m_timeout_value = t;
	}
}

int link_transport_masterread(link_transport_mdriver_t * driver, void * buf, int nbyte){
	link_pkt_t pkt;
	char * p;
	int bytes;
	int err;

	bytes = 0;
	p = buf;
	do {

		if( (err = link_transport_wait_start(&driver->dev, &pkt, m_timeout_value)) < 0 ){
			driver->dev.flush(driver->dev.handle);
			return err;
		}

		if( (err = link_transport_wait_packet(&driver->dev, &pkt, m_timeout_value)) < 0 ){
			driver->dev.flush(driver->dev.handle);
			return err;
		}

		//a packet has arrived -- checksum it
		if( link_transport_checksum_isok(&pkt) == false ){
			return LINK_PROT_ERROR;
		}

		//callback to handle incoming data as it arrives
		//copy the valid data to the buffer
		if( pkt.size + bytes > nbyte ){
			//if the target device has a bug, this will prevent a seg fault
			pkt.size = nbyte - bytes;
		}
		memcpy(p, pkt.data, pkt.size);
		bytes += pkt.size;
		p += pkt.size;

	} while( (bytes < nbyte) && (pkt.size == LINK_PACKET_DATA_SIZE));

	return bytes;
}

int link_transport_masterwrite(link_transport_mdriver_t * driver, const void * buf, int nbyte){
	link_pkt_t pkt;
	char * p;
	int bytes;
	int err;

	if( driver == 0 ){
		link_error("Driver is not available\n");
		return -1;
	}

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
		if( driver->dev.write(driver->dev.handle, &pkt, pkt.size + LINK_PACKET_HEADER_SIZE) != (pkt.size + LINK_PACKET_HEADER_SIZE) ){
			link_error("Link PHY write failed");
			return LINK_PHY_ERROR;
		}

		//received ack of the checksum
		if( (err = wait_ack(driver, pkt_checksum(&pkt), INITIAL_TIMEOUT_VALUE)) < 0 ){
			link_error("wait ack failed");
			driver->dev.flush(driver->dev.handle);
			return err;
		}

		bytes += pkt.size;
		p += pkt.size;

	} while( (bytes < nbyte) && (pkt.size == LINK_PACKET_DATA_SIZE) );

	return bytes;
}


int wait_ack(link_transport_mdriver_t * driver, uint8_t checksum, int timeout){
	link_ack_t ack;
	char * p;
	int count;
	int bytes_read;
	int ret;

	count = 0;
	p = (char*)&ack;
	bytes_read = 0;
	do {
		ret = driver->dev.read(driver->dev.handle, p, sizeof(ack) - bytes_read);
		if( ret < 0 ){
			link_error("read failed");
			return LINK_PHY_ERROR;
		}

		if( ret > 0 ){
			link_debug(LINK_DEBUG_MESSAGE, "Got %d bytes: %X", ret, *p);
			bytes_read += ret;
			p += ret;
			count = 0;
		} else {
#if defined __win32
			//windows waits too long with Sleep, so delay is built into comm
#else
			driver->dev.wait(1);
#endif
			count+=1;
			if( count >= timeout ){
				link_error("timeout %d of %d (%d) -- %d >= %d",
							  ack.checksum, checksum, ack.ack, count, timeout);
				return LINK_TIMEOUT_ERROR;
			}
		}
	} while(bytes_read < sizeof(ack));


	if( ack.checksum != checksum ){
		link_debug(LINK_DEBUG_WARNING, "Checksum failed 0x%X != 0x%X (0x%X)", ack.checksum, checksum, ack.ack);
		return LINK_PROT_ERROR;
	}

	return ack.ack;
}

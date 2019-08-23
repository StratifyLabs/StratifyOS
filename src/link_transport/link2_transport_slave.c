
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sos/link/transport.h"

#include "mcu/core.h"
#include "mcu/debug.h"


#define pkt_checksum(pktp) ((pktp)->data[(pktp)->size])

static int send_ack(link_transport_driver_t * driver, u8 ack, u8 checksum);

int link2_transport_slaveread(
		link_transport_driver_t * driver,
		void * buf,
		int nbyte,
		int (*callback)(void*,void*,int),
		void * context
		){
	link2_pkt_t pkt;
	char * p;
	int bytes;
	u16 checksum;
	int err;
	int ret;

	bytes = 0;
	p = buf;
	do {

		if( (err = link2_transport_wait_start(driver, &pkt, driver->timeout)) < 0 ){
			driver->flush(driver->handle);
			send_ack(driver, LINK2_PACKET_NACK, 0);
			return -1;
		}

		if( (err = link2_transport_wait_packet(driver, &pkt, driver->timeout)) < 0 ){
			driver->flush(driver->handle);
			send_ack(driver, LINK2_PACKET_NACK, 0);
			return -2;
		}

		if( pkt.start != LINK2_PACKET_START ){
			//if packet does not start with the start byte then it is not a packet
			driver->flush(driver->handle);
			send_ack(driver, LINK2_PACKET_NACK, 0);
			return -3;
		}

		//a packet has arrived -- checksum it
		if( driver->o_flags & LINK2_FLAG_IS_CHECKSUM ){
			checksum = pkt_checksum(&pkt);
			if( link2_transport_checksum_isok(&pkt) == false ){
				//bad checksum on packet -- treat as a non-packet
				driver->flush(driver->handle);
				send_ack(driver, LINK2_PACKET_NACK, checksum);
				return -4;
			}
		} else {
			checksum = 0;
		}

		//callback to handle incoming data as it arrives
		if( callback == NULL ){
			//copy the valid data to the buffer
			memcpy(p, pkt.data, pkt.size);
			bytes += pkt.size;
			p += pkt.size;
			send_ack(driver, LINK2_PACKET_ACK, checksum);
		} else {
			if( (ret = callback(context, pkt.data, pkt.size)) < 0 ){
				send_ack(driver, LINK2_PACKET_NACK, checksum);
				return ret;
			} else {
				bytes += pkt.size;
				if( send_ack(driver, LINK2_PACKET_ACK, checksum) < 0 ){
					return -6;
				}
			}
		}

	} while( (bytes < nbyte) && (pkt.size == LINK2_PACKET_DATA_SIZE) );

	if( bytes == 0 ){
		driver->flush(driver->handle);
	}

	return bytes;
}

int link2_transport_slavewrite(
		link_transport_driver_t * driver,
		const void * buf,
		int nbyte,
		int (*callback)(void*,void*,int),
		void * context
		){
	link2_pkt_t pkt;
	char * p;
	int bytes;
	int ret;

	bytes = 0;
	p = (void*)buf;
	pkt.start = LINK2_PACKET_START;
	pkt.o_flags = driver->o_flags;

	do {

		if( (nbyte - bytes) > LINK2_PACKET_DATA_SIZE ){
			pkt.size = LINK2_PACKET_DATA_SIZE;
		} else {
			pkt.size = nbyte - bytes;
		}

		if( callback != NULL ){
			if( (ret = callback(context, pkt.data, pkt.size)) < 0 ){
				//could not get the desired data
				pkt.size = 0;
			} else {
				pkt.size = ret;
			}
		} else {
			//copy data from buf
			memcpy(pkt.data, p, pkt.size);
		}

		if( driver->o_flags & LINK2_FLAG_IS_CHECKSUM ){
			link2_transport_insert_checksum(&pkt);
		}

		//send packet
		if( driver->write(
				 driver->handle,
				 &pkt,
				 pkt.size + LINK2_PACKET_HEADER_SIZE
				 ) != (pkt.size + LINK2_PACKET_HEADER_SIZE)
			 ){
			MCU_DEBUG_LINE_TRACE();
			return -1;
		}

		bytes += pkt.size;
		p += pkt.size;

	} while( (bytes < nbyte) && (pkt.size == LINK2_PACKET_DATA_SIZE) );

	if( callback && (bytes == 0) ){
		bytes = ret;
	}

	return bytes;
}

int send_ack(link_transport_driver_t * driver, u8 ack, u8 checksum){
	link_ack_t ack_pkt;
	ack_pkt.ack = ack;
	ack_pkt.checksum = checksum;
	return driver->write(driver->handle, &ack_pkt, sizeof(ack_pkt));
}


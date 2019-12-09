
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sos/link/transport.h"
#include "sos/link.h"
#include "sos/fs/sysfs.h"
#include "../link/link_local.h"


#ifdef __win32
#define DEFAULT_TIMEOUT_VALUE 500
#else
#define DEFAULT_TIMEOUT_VALUE 500 //used for more complex operations
#endif


#define pkt_checksum(pktp) ((pktp)->data[(pktp)->size])

static int wait_ack(link_transport_mdriver_t * driver, uint8_t checksum, int timeout);

void link1_transport_mastersettimeout(link_transport_mdriver_t * driver, int t){
	if ( t == 0 ){
		driver->phy_driver.timeout = DEFAULT_TIMEOUT_VALUE;
	} else {
		driver->phy_driver.timeout = t;
	}
}

int link1_transport_masterread(link_transport_mdriver_t * driver, void * buf, int nbyte){
	link_pkt_t pkt;
	char * p;
	int bytes;
	int err;

	bytes = 0;
	p = buf;
	link_debug(LINK_DEBUG_MESSAGE, "link1 transport master read %d bytes", nbyte);
	do {

		if( (err = link1_transport_wait_start(&driver->phy_driver, &pkt, driver->phy_driver.timeout)) < 0 ){
			driver->phy_driver.flush(driver->phy_driver.handle);
			link_debug(LINK_DEBUG_MESSAGE, "wait start failed %d, flushed", err);
			return err;
		}

		if( (err = link1_transport_wait_packet(&driver->phy_driver, &pkt, driver->phy_driver.timeout)) < 0 ){
			driver->phy_driver.flush(driver->phy_driver.handle);
			link_debug(LINK_DEBUG_MESSAGE, "wait packet failed %d, flushed", err);
			return err;
		}

		//a packet has arrived -- checksum it
		if( link1_transport_checksum_isok(&pkt) == false ){
			driver->phy_driver.flush(driver->phy_driver.handle);
			link_debug(
						LINK_DEBUG_MESSAGE,
						"bad checksum on packet"
						);
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

int link1_transport_masterwrite(
		link_transport_mdriver_t * driver,
		const void * buf,
		int nbyte
		){
	link_pkt_t pkt;
	char * p;
	int bytes;
	int err;

	if( driver == 0 ){
		return -1;
	}

	bytes = 0;
	p = (void*)buf;
	link_debug(LINK_DEBUG_DEBUG, "link1 transport master write %d bytes", nbyte);
	pkt.start = LINK_PACKET_START;
	do {

		if( (nbyte - bytes) > LINK_PACKET_DATA_SIZE ){
			pkt.size = LINK_PACKET_DATA_SIZE;
		} else {
			pkt.size = nbyte - bytes;
		}

		memcpy(pkt.data, p, pkt.size);

		link1_transport_insert_checksum(&pkt);

		//send packet
		if( (err = driver->phy_driver.write(
				 driver->phy_driver.handle,
				 &pkt, pkt.size + LINK_PACKET_HEADER_SIZE)) != (pkt.size + LINK_PACKET_HEADER_SIZE)
			 ){
			link_debug(
						LINK_DEBUG_MESSAGE,
						"phy write error to %p (%d)",
						driver->phy_driver.handle,
						err
						);

			return LINK_PHY_ERROR;
		}

		//received ack of the checksum
		if( (err = wait_ack(
				  driver,
				  pkt_checksum(&pkt),
				  driver->phy_driver.timeout
				  )) < 0 ){
			driver->phy_driver.flush(driver->phy_driver.handle);
			link_debug(LINK_DEBUG_MESSAGE,
						"wait ack error %d on timeout %d",
						err,
						driver->phy_driver.timeout);
			return err;
		}

		if( err != LINK_PACKET_ACK ){
			link_debug(LINK_DEBUG_MESSAGE, "wrong ack value %d", err);
			return SYSFS_SET_RETURN(err);
		}

		bytes += pkt.size;
		p += pkt.size;

	} while( (bytes < nbyte) && (pkt.size == LINK_PACKET_DATA_SIZE) );

	return bytes;
}


int wait_ack(
		link_transport_mdriver_t * driver,
		uint8_t checksum,
		int timeout
		){
	link_ack_t ack;
	char * p;
	int count;
	int bytes_read;
	int ret;

	count = 0;
	p = (char*)&ack;
	bytes_read = 0;
	do {
		ret = driver->phy_driver.read(driver->phy_driver.handle, p, sizeof(ack) - bytes_read);
		if( ret < 0 ){
			return LINK_PHY_ERROR;
		}

		if( ret > 0 ){
			bytes_read += ret;
			p += ret;
			count = 0;
		} else {
#if defined __win32
			//windows waits too long with Sleep, so delay is built into comm
#else
			driver->phy_driver.wait(1);

#endif
			count+=1;
			if( count >= timeout ){
				return LINK_TIMEOUT_ERROR;
			}
		}
	} while(bytes_read < sizeof(ack));

	if( ack.checksum != checksum ){
		link_debug(LINK_DEBUG_MESSAGE, "checksum mismatch 0x%X != 0x%X",
					  ack.checksum, checksum);
		return LINK_PROT_ERROR;
	}

	return ack.ack;
}

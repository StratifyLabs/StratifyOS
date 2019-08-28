
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sos/link.h"
#include "sos/fs/sysfs.h"

#ifdef __win32
#define DEFAULT_TIMEOUT_VALUE 500
#else
#define DEFAULT_TIMEOUT_VALUE 500
#endif


#define pkt_checksum(pktp) ((pktp)->data[(pktp)->size])

static int wait_ack(
		link_transport_mdriver_t * driver,
		u8 checksum,
		int timeout
		);

void link2_transport_mastersettimeout(link_transport_mdriver_t * driver, int t){
	if ( t == 0 ){
		driver->phy_driver.timeout = DEFAULT_TIMEOUT_VALUE;
	} else {
		driver->phy_driver.timeout = t;
	}
}

int link2_transport_masterread(link_transport_mdriver_t * driver, void * buf, int nbyte){
	link2_pkt_t pkt;
	char * p;
	int bytes;
	int err;

	bytes = 0;
	p = buf;
	do {

		if( (err = link2_transport_wait_start(&driver->phy_driver, &pkt, driver->phy_driver.timeout)) < 0 ){
			//printf("\nerror %s():%d result:%d\n", __FUNCTION__, __LINE__, err);
			driver->phy_driver.flush(driver->phy_driver.handle);
			return err;
		}

		if( (err = link2_transport_wait_packet(&driver->phy_driver, &pkt, driver->phy_driver.timeout)) < 0 ){
			driver->phy_driver.flush(driver->phy_driver.handle);
			return err;
		}

		if( driver->phy_driver.o_flags & LINK2_FLAG_IS_CHECKSUM ){
			//a packet has arrived -- checksum it
			if( link2_transport_checksum_isok(&pkt) == false ){
				return SYSFS_SET_RETURN(1);
			}
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

	} while( (bytes < nbyte) && (pkt.size == LINK2_PACKET_DATA_SIZE));

	return bytes;
}

int link2_transport_masterwrite(link_transport_mdriver_t * driver, const void * buf, int nbyte){
	link2_pkt_t pkt;
	char * p;
	int bytes;
	int err;

	if( driver == 0 ){
		return -1;
	}

	bytes = 0;
	p = (void*)buf;
	memset(&pkt, 0, sizeof(pkt));
	pkt.start = LINK2_PACKET_START;
	pkt.o_flags = driver->phy_driver.o_flags;

	do {

		if( (nbyte - bytes) > LINK2_PACKET_DATA_SIZE ){
			pkt.size = LINK2_PACKET_DATA_SIZE;
		} else {
			pkt.size = nbyte - bytes;
		}

		memcpy(pkt.data, p, pkt.size);

		if( driver->phy_driver.o_flags & LINK2_FLAG_IS_CHECKSUM ){
			link2_transport_insert_checksum(&pkt);
		} else {
			//checksum is set to zero
			pkt_checksum(&pkt) = 0;
		}

		//send packet
		if( driver->phy_driver.write(
				 driver->phy_driver.handle,
				 &pkt,
				 pkt.size + LINK2_PACKET_HEADER_SIZE
				 ) != (pkt.size + LINK2_PACKET_HEADER_SIZE) ){
			return SYSFS_SET_RETURN(1);
		}

		//received ack of the checksum
		if( (err = wait_ack(
					driver,
					pkt_checksum(&pkt),
					driver->phy_driver.timeout
					)) < 0 ){
			driver->phy_driver.flush(driver->phy_driver.handle);
#if 0
			printf("\nerror %s():%d 0x%X-%d (%d)\n",
					 __FUNCTION__,
					 __LINE__,
					 err,
					 err,
					 driver->phy_driver.timeout);
#endif
			return err;
		}

		if( err != LINK2_PACKET_ACK ){
			return SYSFS_SET_RETURN(1);
		}

		bytes += pkt.size;
		p += pkt.size;

	} while( (bytes < nbyte) && (pkt.size == LINK2_PACKET_DATA_SIZE) );

	return bytes;
}


int wait_ack(link_transport_mdriver_t * driver, u8 checksum, int timeout){
	link_ack_t ack;
	char * p;
	int count;
	int bytes_read;
	int ret;

	count = 0;
	p = (char*)&ack;
	bytes_read = 0;
	int loop_count = 0;
	do {

		ret = driver->phy_driver.read(
					driver->phy_driver.handle,
					p,
					sizeof(ack) - bytes_read
					);

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
		return LINK_PROT_ERROR;
	}

	return ack.ack;
}

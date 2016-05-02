/*
 * link_protocol.h
 *
 *  Created on: Mar 25, 2016
 *      Author: tgil
 */

#ifndef IFACE_LINK_TRANSPORT_H_
#define IFACE_LINK_TRANSPORT_H_

#include "../mcu/types.h"

#define LINK_MAX_TRANSFER_SIZE 64
#define LINK_MAX_PACKET_SIZE (63)
#define LINK_PACKET_HEADER_SIZE (3)  //start, size and checksum
#define LINK_PACKET_DATA_SIZE (LINK_MAX_PACKET_SIZE - LINK_PACKET_HEADER_SIZE)


#define LINK_PACKET_STDIO_FLAG (1<<15)

#define LINK_PACKET_START (16)  //DLE character
#define LINK_PACKET_ACK (0x06)
#define LINK_PACKET_NACK (0x53)


typedef struct MCU_PACK {
	u8 ack;
	u8 checksum;
} link_ack_t;

typedef struct MCU_PACK {
	u8 start;
	u8 size;
	u8 data[LINK_PACKET_DATA_SIZE+1];
} link_pkt_t;

#if defined __link
typedef void * link_transport_phy_t;
#define LINK_PHY_OPEN_ERROR ((void*)0)
#else
typedef int link_transport_phy_t;
#define LINK_PHY_OPEN_ERROR (-1)
#endif

typedef struct {
	link_transport_phy_t handle;
	link_transport_phy_t (*open)(const char *, int);
	int (*write)(link_transport_phy_t, const void*, int);
	int (*read)(link_transport_phy_t, void*, int);
	int (*close)(link_transport_phy_t);
	void (*wait)(int);
	void (*flush)(link_transport_phy_t);
	int timeout;
} link_transport_driver_t;

typedef struct {
	int (*getname)(char * dest, const char * last, int len);
	link_transport_phy_t (*open)(const char * name, int baudrate);
	int (*lock)(link_transport_phy_t handle);
	int (*unlock)(link_transport_phy_t handle);
	int (*status)(link_transport_phy_t handle);
	link_transport_driver_t driver;
} link_transport_mdriver_t;

void link_transport_mastersettimeout(int t);
int link_transport_masterwrite(link_transport_mdriver_t * driver, const void * buf, int nbyte);
int link_transport_masterread(link_transport_mdriver_t * driver, void * buf, int nbyte);

int link_transport_slavewrite(link_transport_driver_t * driver, const void * buf, int nbyte, int (*callback)(void*,void*,int), void * context);
int link_transport_slaveread(link_transport_driver_t * driver, void * buf, int nbyte, int (*callback)(void*,void*,int), void * context);

void link_transport_insert_checksum(link_pkt_t * pkt);
bool link_transport_checksum_isok(link_pkt_t * pkt);
int link_transport_wait_packet(link_transport_driver_t * driver, link_pkt_t * pkt, int timeout);
int link_transport_wait_start(link_transport_driver_t * driver, link_pkt_t * pkt, int timeout);


#endif /* IFACE_LINK_TRANSPORT_H_ */

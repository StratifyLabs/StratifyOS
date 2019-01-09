/* Copyright 2011-2018 Tyler Gilbert;
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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef DEV_LINK_TRANSPORT_H_
#define DEV_LINK_TRANSPORT_H_

#include "mcu/types.h"

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

typedef struct {
	const void * buf;
	int nbyte;
} link_transport_notify_t;

#endif


typedef struct {
		int baudrate;
		int stop_bits;
		int parity;
} link_transport_serial_options_t;


typedef struct {
	link_transport_phy_t handle;
	link_transport_phy_t (*open)(const char *, const void * options);
	int (*write)(link_transport_phy_t, const void*, int);
	int (*read)(link_transport_phy_t, void*, int);
	int (*close)(link_transport_phy_t*);
	void (*wait)(int);
	void (*flush)(link_transport_phy_t);
	void (*request)(link_transport_phy_t);
	int timeout;
} link_transport_driver_t;

typedef struct {
	int (*getname)(char * dest, const char * last, int len);
	int (*lock)(link_transport_phy_t handle);
	int (*unlock)(link_transport_phy_t handle);
	int (*status)(link_transport_phy_t handle);
	link_transport_driver_t dev;
	char dev_name[64];
	char notify_name[64];
	const void * options;
} link_transport_mdriver_t;

#if defined __cplusplus
extern "C" {
#endif

void link_transport_mastersettimeout(int t);
int link_transport_masterwrite(link_transport_mdriver_t * driver, const void * buf, int nbyte);
int link_transport_masterread(link_transport_mdriver_t * driver, void * buf, int nbyte);

int link_transport_slavewrite(link_transport_driver_t * driver, const void * buf, int nbyte, int (*callback)(void*,void*,int), void * context);
int link_transport_slaveread(link_transport_driver_t * driver, void * buf, int nbyte, int (*callback)(void*,void*,int), void * context);

void link_transport_insert_checksum(link_pkt_t * pkt);
bool link_transport_checksum_isok(link_pkt_t * pkt);
int link_transport_wait_packet(link_transport_driver_t * driver, link_pkt_t * pkt, int timeout);
int link_transport_wait_start(link_transport_driver_t * driver, link_pkt_t * pkt, int timeout);

#if defined __cplusplus
}
#endif

#endif /* DEV_LINK_TRANSPORT_H_ */

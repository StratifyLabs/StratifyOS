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
#define LINK_PACKET_HEADER_SIZE (3)  //start, size and checksum (1 byte)
#define LINK_PACKET_DATA_SIZE (LINK_MAX_PACKET_SIZE - LINK_PACKET_HEADER_SIZE)


#define LINK_PACKET_STDIO_FLAG (1<<15)

#define LINK_PACKET_START (16)  //DLE character
#define LINK_PACKET_ACK (0x06)
#define LINK_PACKET_NACK (0x53)

#define LINK2_PACKET_START (17)
#define LINK2_PACKET_HEADER_SIZE (6)  //start, size and checksum (2 bytes)
#define LINK2_MAX_PACKET_SIZE (1024+LINK2_PACKET_HEADER_SIZE)
#define LINK2_PACKET_DATA_SIZE (LINK2_MAX_PACKET_SIZE - LINK2_PACKET_HEADER_SIZE)
#define LINK2_PACKET_ACK (0x07)
#define LINK2_PACKET_NACK (0x54)


enum link2_flags {
	LINK2_FLAG_IS_CHECKSUM = (1<<0)
};


typedef struct MCU_PACK {
	u8 ack;
	u8 checksum;
} link_ack_t;

typedef struct MCU_PACK {
	u8 start;
	u8 size;
	u8 data[LINK_PACKET_DATA_SIZE+1];
} link_pkt_t;

typedef struct MCU_PACK {
	u8 start;
	u8 o_flags;
	u16 size;
	u8 data[LINK2_PACKET_DATA_SIZE+2]; //2 checksum bytes
} link2_pkt_t;

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

typedef struct link_transport_driver {
	link_transport_phy_t handle;
	link_transport_phy_t (*open)(const char *, const void * options);
	int (*write)(link_transport_phy_t, const void*, int);
	int (*read)(link_transport_phy_t, void*, int);
	int (*close)(link_transport_phy_t*);
	void (*wait)(int);
	void (*flush)(link_transport_phy_t);
	void (*request)(link_transport_phy_t);
	int (*transport_read)(struct link_transport_driver * driver, void * buf, int nbyte, int (*callback)(void*,void*,int), void * context);
	int (*transport_write)(struct link_transport_driver * driver, const void * buf, int nbyte, int (*callback)(void*,void*,int), void * context);
	int timeout;
	u8 o_flags;
} link_transport_driver_t;

typedef struct {
	int (*getname)(char * dest, const char * last, int len);
	int (*lock)(link_transport_phy_t handle);
	int (*unlock)(link_transport_phy_t handle);
	int (*status)(link_transport_phy_t handle);
	link_transport_driver_t phy_driver;
	char dev_name[64];
	char notify_name[64];
	const void * options;
	u32 transport_version; //which version of the protocol is the slave running
} link_transport_mdriver_t;


typedef struct {
	void (*set_timeout)(link_transport_mdriver_t * driver, int t);
	int (*write)(link_transport_mdriver_t * driver, const void * buf, int nbyte);
	int (*read)(link_transport_mdriver_t * driver, void * buf, int nbyte);
} link_transport_master_t;

typedef struct {
	int (*write)(link_transport_driver_t * driver, const void * buf, int nbyte, int (*callback)(void*,void*,int), void * context);
	int (*read)(link_transport_driver_t * driver, void * buf, int nbyte, int (*callback)(void*,void*,int), void * context);
} link_transport_slave_t;

#if defined __cplusplus
extern "C" {
#endif

void link_transport_mastersettimeout(link_transport_mdriver_t * driver, int t);
int link_transport_masterwrite(link_transport_mdriver_t * driver, const void * buf, int nbyte);
int link_transport_masterread(link_transport_mdriver_t * driver, void * buf, int nbyte);

int link_transport_slavewrite(link_transport_driver_t * driver, const void * buf, int nbyte, int (*callback)(void*,void*,int), void * context);
int link_transport_slaveread(link_transport_driver_t * driver, void * buf, int nbyte, int (*callback)(void*,void*,int), void * context);


void link1_transport_mastersettimeout(link_transport_mdriver_t * driver, int t);
int link1_transport_masterwrite(link_transport_mdriver_t * driver, const void * buf, int nbyte);
int link1_transport_masterread(link_transport_mdriver_t * driver, void * buf, int nbyte);
int link1_transport_slavewrite(link_transport_driver_t * driver, const void * buf, int nbyte, int (*callback)(void*,void*,int), void * context);
int link1_transport_slaveread(link_transport_driver_t * driver, void * buf, int nbyte, int (*callback)(void*,void*,int), void * context);

void link1_transport_insert_checksum(link_pkt_t * pkt);
bool link1_transport_checksum_isok(link_pkt_t * pkt);
int link1_transport_wait_packet(link_transport_driver_t * driver, link_pkt_t * pkt, int timeout);
int link1_transport_wait_start(link_transport_driver_t * driver, link_pkt_t * pkt, int timeout);


void link2_transport_mastersettimeout(link_transport_mdriver_t * driver, int t);
int link2_transport_masterwrite(link_transport_mdriver_t * driver, const void * buf, int nbyte);
int link2_transport_masterread(link_transport_mdriver_t * driver, void * buf, int nbyte);
int link2_transport_slavewrite(link_transport_driver_t * driver, const void * buf, int nbyte, int (*callback)(void*,void*,int), void * context);
int link2_transport_slaveread(link_transport_driver_t * driver, void * buf, int nbyte, int (*callback)(void*,void*,int), void * context);
void link2_transport_insert_checksum(link2_pkt_t * pkt);
bool link2_transport_checksum_isok(link2_pkt_t * pkt);
int link2_transport_wait_packet(link_transport_driver_t * driver, link2_pkt_t * pkt, int timeout);
int link2_transport_wait_start(link_transport_driver_t * driver, link2_pkt_t * pkt, int timeout);



#if defined __cplusplus
}
#endif

#endif /* DEV_LINK_TRANSPORT_H_ */

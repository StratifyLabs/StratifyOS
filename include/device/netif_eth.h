// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEVICE_NETIF_ENET_H_
#define DEVICE_NETIF_ENET_H_

#include "sos/fs/devfs.h"
#include "sos/dev/netif.h"
#include "mcu/eth.h"

typedef struct {
	const char * buf;
	int * nbyte;
	int count;
	int timeout;
	uint8_t cmd[16];
	devfs_async_t op;
	mcu_event_handler_t handler;
	u32 flags;
} netif_eth_state_t;

typedef struct {
    eth_config_t sdio;
} netif_eth_config_t;


int netif_eth_open(const devfs_handle_t * handle);
int netif_eth_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int netif_eth_read(const devfs_handle_t * handle, devfs_async_t * rop);
int netif_eth_write(const devfs_handle_t * handle, devfs_async_t * wop);
int netif_eth_close(const devfs_handle_t * handle);

int netif_eth_dma_open(const devfs_handle_t * handle);
int netif_eth_dma_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int netif_eth_dma_read(const devfs_handle_t * handle, devfs_async_t * rop);
int netif_eth_dma_write(const devfs_handle_t * handle, devfs_async_t * wop);
int netif_eth_dma_close(const devfs_handle_t * handle);


#endif /* DEVICE_NETIF_ENET_H_ */

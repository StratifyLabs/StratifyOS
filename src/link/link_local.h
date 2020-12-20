// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef LINK_LOCAL_H_
#define LINK_LOCAL_H_

#include <stdio.h>
#include <stdarg.h>
#include "sos/link.h"

#ifdef __cplusplus
extern "C" {
#endif



#define LINK_PHY_NAME_MAX 64

#define LINK_MAX_SN_SIZE 512

#define LINK_DEVICE_PRESENT_BUT_NOT_BOOTLOADER (-8183650)

int link_handle_err(link_transport_mdriver_t * driver, int err);
int link_ioctl_delay(link_transport_mdriver_t * driver, int fildes, int request, void * argp, int arg, int delay);


//Functions
int link_rd_err(link_transport_phy_t dev);

#define SET_FD_PORT(fd,port) (fd|(port<<8))
#define GET_FD(fd_port) (fd_port & 0xFF)
#define GET_PORT(fd_port) (fd_port >> 8)

#ifdef __link
int link_debug_printf(
      int x,
      const char * function,
      int line,
      const char * fmt,
      ...);

#define link_debug(x, ...) link_debug_printf(x, __FUNCTION__, __LINE__, __VA_ARGS__)
#define link_error(...) link_debug_printf(LINK_DEBUG_ERROR, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define link_debug(x, ...)
#define link_error(...)
#endif


//default driver
int link_phy_getname(char * dest, const char * last, int len);
link_transport_phy_t link_phy_open(const char * name, const void * options);
int link_phy_status(link_transport_phy_t handle);
int link_phy_write(link_transport_phy_t handle, const void * buf, int nbyte);
int link_phy_read(link_transport_phy_t handle, void * buf, int nbyte);
int link_phy_close(link_transport_phy_t * handle);
void link_phy_flush(link_transport_phy_t handle);
int link_phy_lock(link_transport_phy_t phy);
int link_phy_unlock(link_transport_phy_t phy);
void link_phy_wait(int msec);


#ifdef __cplusplus
}
#endif


#endif /* LINK_LOCAL_H_ */

// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef MCU_CRYPT_H_
#define MCU_CRYPT_H_

#include "sos/dev/crypt.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  crypt_attr_t attr;
  u32 port;
} crypt_config_t;

#define MCU_CRYPT_IOCTL_REQUEST_DECLARATION(driver_name) \
	DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getinfo); \
	DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setattr); \
	DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setaction); \
	DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getiv)

#define MCU_CRYPT_DRIVER_DECLARATION(variant) \
	DEVFS_DRIVER_DECLARTION(variant); \
	MCU_CRYPT_IOCTL_REQUEST_DECLARATION(variant)

MCU_CRYPT_DRIVER_DECLARATION(mcu_crypt);
MCU_CRYPT_DRIVER_DECLARATION(mcu_crypt_dma);


#ifdef __cplusplus
}
#endif


#endif /* MCU_CRYPT_H_ */

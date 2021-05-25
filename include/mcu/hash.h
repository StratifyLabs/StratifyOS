// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef MCU_HASH_H_
#define MCU_HASH_H_

#include "sos/dev/hash.h"
#include "sos/fs/devfs.h"

typedef struct {
  hash_attr_t attr;
  u32 port;
} hash_config_t;

typedef struct {
  u32 attr;
  u32 port;
} random_config_t;

typedef random_config_t rng_config_t;

#ifdef __cplusplus
extern "C" {
#endif

#define MCU_HASH_IOCTL_REQUEST_DECLARATION(driver_name) \
	DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getinfo); \
	DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setattr); \
	DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setaction)

#define MCU_HASH_DRIVER_DECLARATION(variant) \
	DEVFS_DRIVER_DECLARTION(variant); \
	MCU_HASH_IOCTL_REQUEST_DECLARATION(variant)

MCU_HASH_DRIVER_DECLARATION(mcu_hash);
MCU_HASH_DRIVER_DECLARATION(mcu_hash_dma);


#ifdef __cplusplus
}
#endif


#endif /* MCU_HASH_H_ */

// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef MCU_BOOTLOADER_H_
#define MCU_BOOTLOADER_H_

#include "../sos/dev/bootloader.h"

typedef struct {
  int abort;
  int bytes;
  int increment;
  int total;
} boot_event_flash_t;

typedef struct {
  void *dest;
  const void *src;
  const char *key;
} boot_event_crypt_t;

#endif /* MCU_BOOTLOADER_H_ */

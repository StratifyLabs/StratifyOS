// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef SOS_BOOT_BOOTLOADER_H_
#define SOS_BOOT_BOOTLOADER_H_

#include "../dev/bootloader.h"

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

void boot_main();
int boot_handle_auth_event(int event, void * args);

#endif /* SOS_BOOT_BOOTLOADER_H_ */

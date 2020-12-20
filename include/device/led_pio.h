// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef MCU_LED_H_
#define MCU_LED_H_

#include "sos/dev/led.h"
#include "sos/fs/devfs.h"


enum {
	LED_PIO_CONFIG_FLAG_IS_ACTIVE_HIGH = (1<<0),
};

typedef struct MCU_PACK {
	u32 o_flags;
	u32 pin;
} led_pio_config_t;

int led_pio_open(const devfs_handle_t * handle);
int led_pio_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int led_pio_read(const devfs_handle_t * handle, devfs_async_t * rop);
int led_pio_write(const devfs_handle_t * handle, devfs_async_t * wop);
int led_pio_close(const devfs_handle_t * handle);


#endif /* MCU_LED_H_ */

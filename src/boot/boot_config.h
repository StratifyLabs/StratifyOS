#ifndef BOOT_CONFIG_H_
#define BOOT_CONFIG_H_

#include "sos/dev/bootloader.h"

#define BCDVERSION 0x200

//Un-comment to use UART for debugging
#if defined ___debug
#define DEBUG_BOOTLOADER 1
#endif

void delay_ms(int ms);
extern const bootloader_board_config_t boot_board_config;


void boot_event(int event, void * args);

#endif /* BOOT_CONFIG_H_ */

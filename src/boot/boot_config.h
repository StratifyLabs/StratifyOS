#ifndef BOOT_CONFIG_H_
#define BOOT_CONFIG_H_

#include "sos/boot/bootloader.h"

//version 4 will store the first page and write it after verification
#define BCDVERSION 0x400

//Un-comment to use UART for debugging
#if defined ___debug
#define DEBUG_BOOTLOADER 1
#endif

void delay_ms(int ms);
void sos_handle_event(int event, void * args);

#endif /* BOOT_CONFIG_H_ */

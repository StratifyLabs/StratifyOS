/*
 * termios.h
 *
 *  Created on: Jan 27, 2012
 *      Author: tgil
 */

#ifndef TERMIOS_H_
#define TERMIOS_H_


#include <termios.h>


int termios_usbbulk_open(const device_cfg_t * cfg);
int termios_usbbulk_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int termios_usbbulk_write(const device_cfg_t * cfg, device_transfer_t * wop);
int termios_usbbulk_read(const device_cfg_t * cfg, device_transfer_t * rop);
int termios_usbbulk_close(const device_cfg_t * cfg);

int termios_uart_open(const device_cfg_t * cfg);
int termios_uart_ioctl(const device_cfg_t * cfg, int request, void * ctl);
int termios_uart_write(const device_cfg_t * cfg, device_transfer_t * wop);
int termios_uart_read(const device_cfg_t * cfg, device_transfer_t * rop);
int termios_uart_close(const device_cfg_t * cfg);


#endif /* TERMIOS_H_ */

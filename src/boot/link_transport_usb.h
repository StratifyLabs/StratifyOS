/*
 * link_phy_usb.h
 *
 *  Created on: Sep 28, 2013
 *      Author: tgil
 */

#ifndef LINK_TRANSPORT_USB_H_
#define LINK_TRANSPORT_USB_H_

#include "iface/link_transport.h"

link_transport_phy_t boot_stratify_link_transport_usb_open(const char * name, int);
int boot_stratify_link_transport_usb_read(link_transport_phy_t, void * buf, int nbyte);
int boot_stratify_link_transport_usb_write(link_transport_phy_t, const void * buf, int nbyte);
int boot_stratify_link_transport_usb_close(link_transport_phy_t handle);
void boot_stratify_link_transport_usb_wait(int msec);
void boot_stratify_link_transport_usb_flush(link_transport_phy_t handle);

#endif /* LINK_TRANSPORT_USB_H_ */

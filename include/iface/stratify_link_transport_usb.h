/*

Copyright 2011-2016 Tyler Gilbert

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/
#ifndef STRATIFY_LINK_TRANSPORT_USB_H_
#define STRATIFY_LINK_TRANSPORT_USB_H_

//#define __STDIO_VCP


#include "dev/usbfifo.h"
#include "link_transport.h"
#include "stratify/usb_dev.h"
#include "stratify/usb_dev_typedefs.h"

/* \details This structure defines the USB descriptors.  This
 * value is read over the control channel by the host to configure
 * the device.
 */
typedef struct MCU_PACK {
	usb_cfg_desc_t cfg /* The configuration descriptor */;
	usb_desc_vcp_t vcp0;
#ifdef __STDIO_VCP
	usb_desc_vcp_t vcp1;
#endif
	u8 terminator  /* A null terminator used by the driver (required) */;
} stratify_link_transport_usb_cfg_desc_t;


#define LINK_USB_DESC_MANUFACTURER_SIZE 13
#define LINK_USB_DESC_PRODUCT_SIZE 10
#define LINK_USB_DESC_SERIAL_SIZE 16
#define LINK_USB_DESC_MANUFACTUER_STRING 'S','t','r','a','t','i','f','y',' ','L','a','b','s'
#define LINK_USB_DESC_PRODUCT_STRING 'S','t','r','a','t','i','f','y','O','S'
#define LINK_USB_DESC_SERIAL_STRING '0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'

#define LINK_USB_DESC_VCP_0_SIZE 20
#define LINK_USB_DESC_VCP_1_SIZE 21
#define LINK_USB_DESC_VCP_0 'S','t','r','a','t','i','f','y','O','S',' ','L','i','n','k',' ','P','o','r','t'
#define LINK_USB_DESC_VCP_1 'S','t','r','a','t','i','f','y','O','S',' ','S','t','d','i','o',' ','P','o','r','t'


/*! \brief USB Link String Data
 * \details This structure defines the USB strings structure which includes
 * a string for the manufacturer, product, and serial number.
 */
struct MCU_PACK stratify_link_transport_usb_string_t {
	u8 bLength;
	u8 bDescriptorType;
	u16 wLANGID;
	usb_declare_string(LINK_USB_DESC_MANUFACTURER_SIZE) manufacturer;
	usb_declare_string(LINK_USB_DESC_PRODUCT_SIZE) product;
	usb_declare_string(LINK_USB_DESC_SERIAL_SIZE) serial;
	usb_declare_string(LINK_USB_DESC_VCP_0_SIZE) vcp0;
#ifdef __STDIO_VCP
	usb_declare_string(LINK_USB_DESC_VCP_1_SIZE) vcp1;
#endif
};

//replace these to customize the USB device descriptors
extern const usb_dev_desc_t stratify_link_transport_usb_dev_desc MCU_WEAK;
extern const stratify_link_transport_usb_cfg_desc_t stratify_link_transport_usb_cfg_desc MCU_WEAK;
extern const struct stratify_link_transport_usb_string_t stratify_link_transport_usb_string_desc MCU_WEAK;

extern const usb_dev_const_t stratify_link_transport_usb_constants;

link_transport_phy_t stratify_link_transport_usb_open(const char * name, int);
int stratify_link_transport_usb_read(link_transport_phy_t, void * buf, int nbyte);
int stratify_link_transport_usb_write(link_transport_phy_t, const void * buf, int nbyte);
int stratify_link_transport_usb_close(link_transport_phy_t handle);
void stratify_link_transport_usb_wait(int msec);
void stratify_link_transport_usb_flush(link_transport_phy_t handle);
int stratify_link_transport_usb_cdc_if_req(void * context, int event);

link_transport_phy_t boot_stratify_link_transport_usb_open(const char * name, int);
int boot_stratify_link_transport_usb_read(link_transport_phy_t, void * buf, int nbyte);
int boot_stratify_link_transport_usb_write(link_transport_phy_t, const void * buf, int nbyte);
int boot_stratify_link_transport_usb_close(link_transport_phy_t handle);
void boot_stratify_link_transport_usb_wait(int msec);
void boot_stratify_link_transport_usb_flush(link_transport_phy_t handle);

//provided for the link device fifo
//USBFIFO_DEVICE("link-phy-usb", &stratify_link_transport_usb_fifo_cfg, &stratify_link_transport_usb_fifo_state, 0666, USER_ROOT, GROUP_ROOT),
extern const usbfifo_cfg_t stratify_link_transport_usb_fifo_cfg;
extern usbfifo_state_t stratify_link_transport_usb_fifo_state MCU_SYS_MEM;


#endif /* STRATIFY_LINK_TRANSPORT_USB_H_ */

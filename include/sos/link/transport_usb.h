/*

Copyright 2011-2018 Tyler Gilbert

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
#ifndef SOS_LINK_TRANSPORT_USB_H_
#define SOS_LINK_TRANSPORT_USB_H_

#include "transport.h"
#include "device/usbfifo.h"
#include "usbd/control.h"
#include "usbd/cdc.h"

typedef struct MCU_PACK {
	usbd_interface_descriptor_t interface_data /* The interface descriptor */;
	usbd_endpoint_descriptor_t data_out /* Endpoint:  Bulk out */;
	usbd_endpoint_descriptor_t data_in /* Endpoint:  Bulk in */;
} sos_link_interface_configuration_descriptor_t;


/* \details This structure defines the USB descriptors.  This
 * value is read over the control channel by the host to configure
 * the device.
 */
typedef struct MCU_PACK {
	usbd_configuration_descriptor_t cfg /* The configuration descriptor */;
	usbd_cdc_configuration_descriptor_t vcp0;
	usbd_cdc_configuration_descriptor_t vcp1;
	u8 terminator  /* A null terminator used by the driver (required) */;
} sos_link_transport_usb_dual_vcp_configuration_descriptor_t;

typedef struct MCU_PACK {
	usbd_configuration_descriptor_t cfg /* The configuration descriptor */;
	usbd_cdc_configuration_descriptor_t vcp0;
	u8 terminator  /* A null terminator used by the driver (required) */;
} sos_link_transport_usb_configuration_descriptor_t;


#define SOS_LINK_TRANSPORT_USB_DESC_MANUFACTURER_SIZE 13
#define SOS_LINK_TRANSPORT_USB_DESC_PRODUCT_SIZE 11
#define SOS_LINK_TRANSPORT_USB_DESC_SERIAL_SIZE 16
#define SOS_LINK_TRANSPORT_USB_DESC_MANUFACTURER_STRING 'S','t','r','a','t','i','f','y',' ','L','a','b','s'
#define SOS_LINK_TRANSPORT_USB_DESC_PRODUCT_STRING 'S','t','r','a','t','i','f','y',' ','O','S'
#define SOS_LINK_TRANSPORT_USB_DESC_SERIAL_STRING '0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'


#define SOS_LINK_TRANSPORT_USB_DESCRIPTOR_SOS_LINK_SIZE 11
#define SOS_LINK_TRANSPORT_USB_DESCRIPTOR_SOS_LINK 'S','t','r','a','t','i','f','y',' ','O','S'

#define SOS_LINK_TRANSPORT_USB_DESC_VCP_0_SIZE 11
#define SOS_LINK_TRANSPORT_USB_DESC_VCP_1_SIZE 11
#define SOS_LINK_TRANSPORT_USB_DESC_VCP_0 'S','t','r','a','t','i','f','y',' ','O','S'
#define SOS_LINK_TRANSPORT_USB_DESC_VCP_1 'S','e','r','i','a','l',' ','P','o','r','t'


#define SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT (0x2)
#define SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_ALT (0x5)
#define SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_SIZE (LINK_BULK_ENDPOINT_SIZE)
#define SOS_LINK_TRANSPORT_USB_PORT 0
#define SOS_LINK_TRANSPORT_USB_INTIN 0x81
#define SOS_LINK_TRANSPORT_USB_INTIN_ALT 0x84
#define SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_IN (0x80|SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT)
#define SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_OUT (SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT)
#define SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_IN_ALT (0x80|SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_ALT)
#define SOS_LINK_TRANSPORT_USB_BULKOUT_ALT (SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_ALT)
#define SOS_LINK_TRANSPORT_ENDPOINT_SIZE SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_SIZE

/*! \brief USB Link String Data
 * \details This structure defines the USB strings structure which includes
 * a string for the manufacturer, product, and serial number.
 */
struct MCU_PACK sos_link_transport_usb_string_t {
	u8 bLength;
	u8 bDescriptorType;
	u16 wLANGID;
	usbd_declare_string(SOS_LINK_TRANSPORT_USB_DESC_MANUFACTURER_SIZE) manufacturer;
	usbd_declare_string(SOS_LINK_TRANSPORT_USB_DESC_PRODUCT_SIZE) product;
	usbd_declare_string(SOS_LINK_TRANSPORT_USB_DESC_SERIAL_SIZE) serial;
	usbd_declare_string(SOS_LINK_TRANSPORT_USB_DESC_VCP_0_SIZE) vcp0;
	usbd_declare_string(SOS_LINK_TRANSPORT_USB_DESC_VCP_1_SIZE) vcp1;
};

//replace these to customize the USB device descriptors
extern const usbd_device_descriptor_t sos_link_transport_usb_dual_vcp_dev_desc MCU_WEAK;
extern const sos_link_transport_usb_dual_vcp_configuration_descriptor_t sos_link_transport_usb_dual_vcp_cfg_desc MCU_WEAK;
extern const struct sos_link_transport_usb_string_t sos_link_transport_usb_dual_vcp_string_desc MCU_WEAK;
extern const usbd_control_constants_t sos_link_transport_usb_dual_vcp_constants;

link_transport_phy_t sos_link_transport_usb_open(const char * name,
		usbd_control_t * context,
		const usbd_control_constants_t * constants,
		const usb_attr_t * usb_attr,
		mcu_pin_t usb_up_pin,
		int usb_up_active_high);
int sos_link_transport_usb_read(link_transport_phy_t, void * buf, int nbyte);
int sos_link_transport_usb_write(link_transport_phy_t, const void * buf, int nbyte);
int sos_link_transport_usb_close(link_transport_phy_t * handle);
void sos_link_transport_usb_wait(int msec);
void sos_link_transport_usb_flush(link_transport_phy_t handle);

int sos_link_usbd_cdc_event_handler(void * context, const mcu_event_t * event);

//provided for the link device fifo
//DEVFS_DEVICE("link-phy-usb", 0, &sos_link_transport_usb_fifo_cfg, &sos_link_transport_usb_fifo_state, 0666, USER_ROOT, GROUP_ROOT),
extern const usbfifo_config_t sos_link_transport_usb_fifo_cfg;
extern usbfifo_state_t sos_link_transport_usb_fifo_state MCU_SYS_MEM;

//replace these to customize the USB device descriptors
extern const usbd_device_descriptor_t sos_link_transport_usb_dev_desc MCU_WEAK;
extern const usbd_qualifier_descriptor_t sos_link_transfer_usb_qualifer_desc MCU_WEAK;
extern const sos_link_transport_usb_configuration_descriptor_t sos_link_transport_usb_cfg_desc MCU_WEAK;
extern const struct sos_link_transport_usb_string_t sos_link_transport_usb_string_desc MCU_WEAK;
extern const usbd_control_constants_t sos_link_transport_usb_constants;


link_transport_phy_t boot_link_transport_usb_open(const char * name,
		usbd_control_t * context,
		const usbd_control_constants_t * constants,
		const usb_attr_t * usb_attr,
		mcu_pin_t usb_up_pin,
		int usb_up_active_high);
int boot_link_transport_usb_read(link_transport_phy_t, void * buf, int nbyte);
int boot_link_transport_usb_write(link_transport_phy_t, const void * buf, int nbyte);
int boot_link_transport_usb_close(link_transport_phy_t * handle);
void boot_link_transport_usb_wait(int msec);
void boot_link_transport_usb_flush(link_transport_phy_t handle);

extern const usbfifo_config_t sos_link_transport_usb_fifo_cfg;
extern usbfifo_state_t sos_link_transport_usb_fifo_state MCU_SYS_MEM;


#define SOS_LINK_TRANSPORT_USB_DECLARE_INTERFACE_CONFIGURATION_DESCRIPTOR(interface_string,interface_number,endpoint_number,endpoint_size) \
	.interface_data = { \
		.bLength = sizeof(usbd_interface_descriptor_t), \
		.bDescriptorType = USBD_DESCRIPTOR_TYPE_INTERFACE, \
		.bInterfaceNumber = interface_number, \
		.bAlternateSetting = 0x00, \
		.bNumEndpoints = 0x02, \
		.bInterfaceClass = 0xff, \
		.bInterfaceSubClass = 0xff, \
		.bInterfaceProtocol = 0xff, \
		.iInterface = interface_string \
	}, \
	.data_out = { \
		.bLength= sizeof(usbd_endpoint_descriptor_t), \
		.bDescriptorType=USBD_DESCRIPTOR_TYPE_ENDPOINT, \
		.bEndpointAddress=endpoint_number, \
		.bmAttributes=USBD_ENDPOINT_ATTRIBUTES_TYPE_BULK, \
		.wMaxPacketSize=endpoint_size, \
		.bInterval=1 \
	}, \
	.data_in = { \
		.bLength= sizeof(usbd_endpoint_descriptor_t), \
		.bDescriptorType=USBD_DESCRIPTOR_TYPE_ENDPOINT, \
		.bEndpointAddress=(endpoint_number)|0x80, \
		.bmAttributes=USBD_ENDPOINT_ATTRIBUTES_TYPE_BULK, \
		.wMaxPacketSize=endpoint_size, \
		.bInterval=1 \
	}

#endif /* SOS_LINK_TRANSPORT_USB_H_ */

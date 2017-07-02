

#include <errno.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include "mcu/cortexm.h"
#include "iface/link.h"
#include "mcu/mcu.h"
#include "iface/dev/usb.h"
#include "dev/usbfifo.h"
#include "stratify/usb_dev.h"
#include "stratify/usb_dev_cdc.h"
#include "mcu/cortexm.h"
#include "mcu/core.h"
#include "mcu/usb.h"
#include "mcu/debug.h"
#include "stratify/usb_dev_typedefs.h"
#include "stratify/usb_dev_defs.h"
#include "mcu/boot_debug.h"

#include "iface/stratify_link_transport_usb.h"

#define USBDEV_PORT 0

const device_cfg_t usb_dev = { .periph.port = USBDEV_PORT };

#define BUF_SIZE 256
static char rd_buffer[BUF_SIZE];
static int rd_tail;
static int rd_head;


#ifdef __STDIO_VCP
void init_stdio_vcp(){
	int fd;
	fd = open("/dev/stdio", O_RDWR);
	if( fd < 0 ){
		return;
	}

	ioctl(fd, I_FIFO_INIT);
	close(fd);
	return;
}
#endif

link_transport_phy_t stratify_link_boot_transport_usb_open(const char * name, usb_dev_context_t * context){
	usb_attr_t usb_attr;

	dstr("OPEN USB\n");
	//open USB
	_mcu_cortexm_delay_ms(250);
	mcu_usb_open(&usb_dev);

	dstr("USB OPEN\n");

	//set USB attributes
	usb_attr.pin_assign = mcu_board_config.usb_pin_assign;
	usb_attr.mode = USB_ATTR_MODE_DEVICE;
	usb_attr.crystal_freq = mcu_board_config.core_osc_freq;
	dstr("SET USB ATTR\n");
	if( mcu_usb_setattr(usb_dev.periph.port, &usb_attr) < 0 ){
		dstr("FAILED TO SET USB ATTR\n");
	}

	dstr("USB ATTR SET\n");
	//initialize USB device
	usb_dev_priv_init(context);
	dstr("USB CONNECT\n");


	rd_tail = 0;
	rd_head = 0;
	return 0;
}

int stratify_link_boot_transport_usb_write(link_transport_phy_t handle, const void * buf, int nbyte){
	int ret;
	ret = mcu_sync_io(&usb_dev, mcu_usb_write, STRATIFY_LINK_TRANSPORT_USB_BULK_ENDPOINT_IN, buf, nbyte, O_RDWR);
	return ret;
}

static int read_buffer(char * dest, int nbyte){
	int i;
	for(i=0; i < nbyte; i++){
		if ( rd_head == rd_tail ){ //check for data in the fifo buffer
			//there is no more data in the buffer
			break;
		} else {
			dest[i] = rd_buffer[rd_tail];
			rd_tail++;
			if ( rd_tail == BUF_SIZE){
				rd_tail = 0;
			}
		}
	}
	return i; //number of bytes read
}

static int write_buffer(const char * src, int nbyte){
	int i;
	for(i=0; i < nbyte; i++){
		if( ((rd_head+1) == rd_tail) ||
				((rd_tail == 0) && (rd_head == (BUF_SIZE-1))) ){
			break; //no more room
		}

		rd_buffer[rd_head] = src[i];
		rd_head++;
		if ( rd_head == BUF_SIZE ){
			rd_head = 0;
		}
	}
	return i; //number of bytes written
}

int stratify_link_boot_transport_usb_read(link_transport_phy_t handle, void * buf, int nbyte){
	int ret;
	int bytes_read;
	char buffer[STRATIFY_LINK_TRANSPORT_ENDPOINT_SIZE];
	bytes_read = 0;
	ret = read_buffer(buf, nbyte);
	bytes_read += ret;

	while( bytes_read < nbyte ){
		//need more data to service request
		ret = mcu_sync_io(&usb_dev, mcu_usb_read, STRATIFY_LINK_TRANSPORT_USB_BULK_ENDPOINT_OUT, buffer, STRATIFY_LINK_TRANSPORT_ENDPOINT_SIZE, O_RDWR | MCU_SYNC_IO_FLAG_READ);
		write_buffer(buffer, ret);
		ret = read_buffer(buf + bytes_read, nbyte - bytes_read);
		bytes_read += ret;
	}
	return nbyte;
}

int stratify_link_boot_transport_usb_close(link_transport_phy_t * handle){
	return mcu_usb_close(&usb_dev);
}

void stratify_link_boot_transport_usb_wait(int msec){
	int i;
	for(i = 0; i < msec; i++){
		_mcu_cortexm_delay_us(1000);
	}
}

void stratify_link_boot_transport_usb_flush(link_transport_phy_t handle){}


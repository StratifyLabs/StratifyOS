
#include "link_transport_usb.h"

#include <errno.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include "iface/link.h"
#include "mcu/mcu.h"
#include "iface/dev/usb.h"
#include "device/usbfifo.h"
#include "stratify/usb_dev.h"
#include "stratify/usb_dev_cdc.h"
#include "mcu/core.h"
#include "mcu/debug.h"
#include "stratify/usb_dev_typedefs.h"
#include "stratify/usb_dev_defs.h"
#include "mcu/boot_debug.h"

#include "iface/stratify_link_transport_usb.h"
#include "stratify_link_transport_usb_common.h"

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

static usb_dev_context_t usb_context;

link_transport_phy_t boot_stratify_link_transport_usb_open(const char * name, int baudrate){
	usb_attr_t usb_attr;

	dstr("OPEN USB\n");
	//open USB
	_mcu_core_delay_ms(250);
	mcu_usb_open(&usb_dev);

	dstr("USB OPEN\n");

	//set USB attributes
	usb_attr.pin_assign = 0;
	usb_attr.mode = USB_ATTR_MODE_DEVICE;
	usb_attr.crystal_freq = mcu_board_config.core_osc_freq;
	dstr("SET USB ATTR\n");
	if( mcu_usb_setattr(usb_dev.periph.port, &usb_attr) < 0 ){
		dstr("FAILED TO SET USB ATTR\n");
	}

	dstr("USB ATTR SET\n");
	//initialize USB device
	memset(&usb_context, 0, sizeof(usb_context));

	usb_context.constants = &stratify_link_transport_usb_constants;
	usb_dev_priv_init(&usb_context);
	dstr("USB CONNECT\n");


	rd_tail = 0;
	rd_head = 0;
	return 0;
}

int boot_stratify_link_transport_usb_write(link_transport_phy_t handle, const void * buf, int nbyte){
	int ret;
	ret = mcu_sync_io(&usb_dev, mcu_usb_write, USB_BULKIN, buf, nbyte, O_RDWR);
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

int boot_stratify_link_transport_usb_read(link_transport_phy_t handle, void * buf, int nbyte){
	int ret;
	int bytes_read;
	char buffer[ENDPOINT_SIZE];
	bytes_read = 0;
	ret = read_buffer(buf, nbyte);
	bytes_read += ret;

	while( bytes_read < nbyte ){
		//need more data to service request
		ret = mcu_sync_io(&usb_dev, mcu_usb_read, USB_BULKOUT, buffer, ENDPOINT_SIZE, O_RDWR | MCU_SYNC_IO_FLAG_READ);
		write_buffer(buffer, ret);
		ret = read_buffer(buf + bytes_read, nbyte - bytes_read);
		bytes_read += ret;
	}
	return nbyte;
}

int boot_stratify_link_transport_usb_close(link_transport_phy_t handle){
	return mcu_usb_close(&usb_dev);
}

void boot_stratify_link_transport_usb_wait(int msec){
	int i;
	for(i = 0; i < msec; i++){
		_mcu_core_delay_us(1000);
	}
}

void boot_stratify_link_transport_usb_flush(link_transport_phy_t handle){}


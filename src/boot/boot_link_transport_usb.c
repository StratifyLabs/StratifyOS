

#include <errno.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include "mcu/arch.h"
#include "cortexm/cortexm.h"
#include "sos/link.h"
#include "mcu/mcu.h"
#include "sos/dev/usb.h"
#include "device/usbfifo.h"
#include "usbd/control.h"
#include "usbd/cdc.h"
#include "cortexm/cortexm.h"
#include "mcu/core.h"
#include "mcu/usb.h"
#include "mcu/pio.h"
#include "mcu/debug.h"
#include "mcu/boot_debug.h"

#include "sos/link/transport_usb.h"

#define BUF_SIZE 256
static char m_read_buffer[BUF_SIZE];
static int m_read_tail;
static int m_read_head;

static  usbd_control_t * m_context;

link_transport_phy_t boot_link_transport_usb_open(const char * name,
																  usbd_control_t * context,
																  const usbd_control_constants_t * constants,
																  const usb_attr_t * usb_attr,
																  mcu_pin_t usb_up_pin,
																  int usb_up_active_high){

	pio_attr_t pio_attr;
	devfs_handle_t pio_handle;
	pio_handle.port = usb_up_pin.port;
	pio_handle.config = 0;
	pio_handle.state = 0;

	if( usb_up_pin.port != 0xff ){
		dstr("CONFIGURE UP PIN\n");
		mcu_pio_open(&pio_handle);
		pio_attr.o_pinmask = (1<<usb_up_pin.pin);
		if( usb_up_active_high ){
			mcu_pio_clrmask(&pio_handle, (void*)(pio_attr.o_pinmask));
		} else {
			mcu_pio_setmask(&pio_handle, (void*)(pio_attr.o_pinmask));
		}
		pio_attr.o_flags = PIO_FLAG_SET_OUTPUT | PIO_FLAG_IS_DIRONLY;
		mcu_pio_setattr(&pio_handle, &pio_attr);
	}
	memset(context, 0, sizeof(usbd_control_t));
	context->constants = constants;
	context->handle = &(constants->handle);

	dstr("OPEN USB\n");
	//open USB
	cortexm_delay_ms(100);

	if( mcu_usb_open(context->handle) < 0 ){ return -1; }

	cortexm_delay_ms(100);

	if( mcu_usb_setattr(context->handle, (void*)usb_attr) < 0 ){
		return -1;
	}

	cortexm_delay_ms(100);

	dstr("USB INIT\n");
	//initialize USB device
	usbd_control_svcall_init(context);

	m_read_tail = 0;
	m_read_head = 0;

	dstr("USB CONNECT\n");
	if( usb_up_pin.port != 0xff ){
		if( usb_up_active_high ){
			dstr("HIGH\n");
			mcu_pio_setmask(&pio_handle, (void*)(pio_attr.o_pinmask));
		} else {
			mcu_pio_clrmask(&pio_handle, (void*)(pio_attr.o_pinmask));
		}
	}

	m_context = context;

	return 0;
}

int boot_link_transport_usb_write(link_transport_phy_t handle, const void * buf, int nbyte){
	int ret;
	ret = mcu_sync_io(
				m_context->handle,
				mcu_usb_write,
				SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_IN,
				buf,
				nbyte,
				O_RDWR
				);
	return ret;
}

static int read_buffer(
		char * dest,
		int nbyte
		){
	int i;
	for(i=0; i < nbyte; i++){
		if ( m_read_head == m_read_tail ){ //check for data in the fifo buffer
			//there is no more data in the buffer
			break;
		} else {
			dest[i] = m_read_buffer[m_read_tail];
			m_read_tail++;
			if ( m_read_tail == BUF_SIZE){
				m_read_tail = 0;
			}
		}
	}
	return i; //number of bytes read
}

static int write_buffer(
		const char * src,
		int nbyte
		){
	int i;
	for(i=0; i < nbyte; i++){
		if( ((m_read_head+1) == m_read_tail) ||
			 ((m_read_tail == 0) && (m_read_head == (BUF_SIZE-1))) ){
			break; //no more room
		}

		m_read_buffer[m_read_head] = src[i];
		m_read_head++;
		if ( m_read_head == BUF_SIZE ){
			m_read_head = 0;
		}
	}
	return i; //number of bytes written
}

int boot_link_transport_usb_read(
		link_transport_phy_t handle,
		void * buf,
		int nbyte
		){
	int ret;
	int bytes_read;
	char buffer[SOS_LINK_TRANSPORT_ENDPOINT_SIZE];
	bytes_read = 0;
	ret = read_buffer(buf, nbyte);
	bytes_read += ret;

	while( bytes_read < nbyte ){
		//need more data to service request

		ret = mcu_sync_io(
					m_context->handle,
					mcu_usb_read,
					SOS_LINK_TRANSPORT_USB_BULK_ENDPOINT_OUT,
					buffer,
					SOS_LINK_TRANSPORT_ENDPOINT_SIZE,
					O_RDWR | MCU_SYNC_IO_FLAG_READ
					);

		if( ret > 0 ){
			write_buffer(buffer, ret);
		}

		ret = read_buffer(buf + bytes_read, nbyte - bytes_read);
		bytes_read += ret;
	}
	return nbyte;
}

int boot_link_transport_usb_close(link_transport_phy_t * handle){
	return mcu_usb_close(m_context->handle);
}

void boot_link_transport_usb_wait(int msec){
	int i;
	for(i = 0; i < msec; i++){
		cortexm_delay_us(1000);
	}
}

void boot_link_transport_usb_flush(link_transport_phy_t handle){}


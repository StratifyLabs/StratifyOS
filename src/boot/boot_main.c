

//#include "config.h"
#include <sys/lock.h>
#include "mcu/mcu.h"
#include "sos/dev/usb.h"
#include "sos/fs/devfs.h"
#include "cortexm/cortexm.h"
#include "mcu/core.h"
#include "mcu/uart.h"
#include "mcu/pio.h"
#include "mcu/usb.h"
#include "mcu/debug.h"
#include "usbd/control.h"
#include "mcu/boot_debug.h"
#include "boot_link.h"
#include "boot_config.h"


#define USBD_DFU_TRANSFER_SIZE 1024
char dnload_buf[USBD_DFU_TRANSFER_SIZE];
#define SW_BOOT_APP_OVERRIDE 0x55664422

void exec_bootloader(void * args){
	//write SW location with key and then reset
	u32 * bootloader_start = (u32*)boot_board_config.sw_req_loc;
	*bootloader_start = boot_board_config.sw_req_value;
	mcu_core_reset(0, 0);
}

void boot_event(int event, void * args){
	mcu_board_execute_event_handler(event, args);
}

extern u32 _etext;

const bootloader_api_t mcu_core_bootloader_api = {
		.code_size = (u32)&_etext,
		.exec = exec_bootloader,
		.usbd_control_root_init = usbd_control_root_init,
		.event = boot_event
};



void gled_on();
void gled_off();

void init_hw();

static void * stack_ptr;
static void (*app_reset)();


void run_bootloader();

void delay_ms(int ms){
	int i;
	for(i=0; i < ms; i++){
		cortexm_delay_us(1000);
	}
}

void led_flash(){
	gled_on();
	delay_ms(500);
	gled_off();
	delay_ms(500);
}

void led_error(){
	while(1){
		led_flash();
	}
}

void led_flash_run_bootloader(){
	int i;
	for(i=0; i < 3; i++){
		gled_on();
		delay_ms(3);
		gled_off();
		delay_ms(3);
	}
}


static int check_run_app();
void run_bootloader();

/*! \details
 */
int boot_main(){
	stack_ptr = (void*)(((uint32_t*)boot_board_config.program_start_addr)[0]);
	app_reset = (void (*)())( (((uint32_t*)boot_board_config.program_start_addr)[1]) );

	boot_event(BOOT_EVENT_START, 0);

	if ( check_run_app() ){
		app_reset();
		while(1);
	} else {
		led_flash_run_bootloader();
		run_bootloader();
	}

	while(1);
	return 0;
}

void run_bootloader(){
	init_hw();

	//initialize link and run link update
	dstr("LINK Start\n");
	boot_link_update((void*)boot_board_config.link_transport_driver);
	while(1);
}

/*! \details This function checks to see if the application should be run
 * or if the device should enter DFU mode.
 * \return Non-zero if the application should be run.
 */
int check_run_app(){
	//! \todo Check to see if end of text is less than app program start
	u32 * bootloader_start = (u32*)boot_board_config.sw_req_loc;
	u32 hw_req_value;
	pio_attr_t pio_attr;
	devfs_handle_t hw_req_handle;
	hw_req_handle.port = boot_board_config.hw_req.port;

	if ( (uint32_t)stack_ptr == 0xFFFFFFFF ){
		//code is not valid
		*bootloader_start = 0;
		return 0;
	}

	pio_attr.o_pinmask = (1<<boot_board_config.hw_req.pin);

	if( boot_board_config.o_flags & BOOT_BOARD_CONFIG_FLAG_HW_REQ_PULLUP ){
		pio_attr.o_flags = PIO_FLAG_SET_INPUT | PIO_FLAG_IS_PULLUP;
		mcu_pio_setattr(&hw_req_handle, &pio_attr);
	} else if( boot_board_config.o_flags & BOOT_BOARD_CONFIG_FLAG_HW_REQ_PULLDOWN ){
		pio_attr.o_flags = PIO_FLAG_SET_INPUT | PIO_FLAG_IS_PULLDOWN;
		mcu_pio_setattr(&hw_req_handle, &pio_attr);
	}

	cortexm_delay_us(500);

	hw_req_value = ((mcu_pio_get(&hw_req_handle, 0) & pio_attr.o_pinmask) != 0);

	if( boot_board_config.o_flags & BOOT_BOARD_CONFIG_FLAG_HW_REQ_ACTIVE_HIGH ){
		if( hw_req_value ){ //pin is high and pin is active high
			*bootloader_start = 0;
			return 0;
		}
	} else { //request is active low
		if( hw_req_value == 0 ){ //pin is active low
			*bootloader_start = 0;
			return 0;
		}
	}

	if ( !(mcu_pio_get(&hw_req_handle, 0) & (1<<boot_board_config.hw_req.pin)) ){
		//*bootloader_start = 0;
		//return 0;
	}

	if ( *bootloader_start == boot_board_config.sw_req_value ){
		*bootloader_start = 0;
		return 0;
	}

	return 1;
}


#ifdef DEBUG_BOOTLOADER
static int debug_write_func(const void * buf, int nbyte){
	mcu_debug_root_write_uart(buf, nbyte);
	return nbyte;
}
#endif

void init_hw(){
	mcu_core_initclock(1);
	cortexm_enable_interrupts(NULL); //Enable the interrupts

	delay_ms(50);


#ifdef DEBUG_BOOTLOADER
	u32 * bootloader_start = (u32*)boot_board_config.sw_req_loc;
	devfs_handle_t handle;
	handle.port = boot_board_config.hw_req.port;
	mcu_debug_init();
	dsetmode(0);
	dsetwritefunc(debug_write_func);

	dstr("STARTING UP\n");

	if ( (uint32_t)stack_ptr == 0xFFFFFFFF ){
		dstr("Stack pointer is invalid\n");
	}

	if ( !(mcu_pio_get(&handle, 0) & (1<<boot_board_config.hw_req.pin)) ){
		dstr("Hardware bootloader request\n");
	}

	if ( *bootloader_start == boot_board_config.sw_req_value ){
		dstr("Software bootloader request\n");
	}


	dstr("STACK: "); dhex((uint32_t)stack_ptr); dstr("\n");
	dstr("APP: "); dhex((uint32_t)app_reset); dstr("\n");
#endif

	boot_event(BOOT_EVENT_INIT, 0);

}

void gled_on(){
	if( mcu_board_config.led.port != 255 ){
		pio_attr_t attr;
		devfs_handle_t handle;
		handle.port = mcu_board_config.led.port;
		attr.o_pinmask = (1<<mcu_board_config.led.pin);
		attr.o_flags = PIO_FLAG_SET_OUTPUT | PIO_FLAG_IS_DIRONLY;
		mcu_pio_setattr(&handle, &attr);
		if( mcu_board_config.o_flags & MCU_BOARD_CONFIG_FLAG_LED_ACTIVE_HIGH ){
			//LED is active low
			mcu_pio_setmask(&handle, (void*)attr.o_pinmask);
		} else {
			mcu_pio_clrmask(&handle, (void*)attr.o_pinmask);
		}
	}
}


void gled_off(){
	if( mcu_board_config.led.port != 255 ){
		pio_attr_t attr;
		devfs_handle_t handle;
		handle.port = mcu_board_config.led.port;
		attr.o_pinmask = (1<<mcu_board_config.led.pin);
		attr.o_flags = PIO_FLAG_SET_INPUT | PIO_FLAG_IS_DIRONLY;
		mcu_pio_setattr(&handle, &attr);
	}
}

//prevent linkage to real handlers
void mcu_core_fault_handler(){}
void mcu_core_hardfault_handler(){}
void mcu_core_memfault_handler(){}
void mcu_core_busfault_handler(){}
void mcu_core_usagefault_handler(){}
void mcu_core_systick_handler(){}
void mcu_core_svcall_handler(){}
void mcu_core_pendsv_handler(){}

/*! @} */

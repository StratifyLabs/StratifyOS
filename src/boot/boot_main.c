

//#include "config.h"
#include <sys/lock.h>
#include "mcu/mcu.h"
#include "sos/dev/usb.h"
#include "sos/fs/devfs.h"
#include "mcu/cortexm.h"
#include "mcu/core.h"
#include "mcu/uart.h"
#include "mcu/pio.h"
#include "mcu/usb.h"
#include "mcu/debug.h"
#include "mcu/usbd/control.h"
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
	if( mcu_board_config.event != 0 ){
		mcu_board_config.event(event, args);
	}
}

extern u32 _etext;

const bootloader_api_t _mcu_core_bootloader_api = {
		.code_size = (u32)&_etext,
		.exec = exec_bootloader,
		.usbd_control_priv_init = usbd_control_priv_init,
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
		_mcu_cortexm_delay_us(1000);
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

	if ( (uint32_t)stack_ptr == 0xFFFFFFFF ){
		//code is not valid
		*bootloader_start = 0;
		return 0;
	}

	_mcu_cortexm_delay_us(500);

	if ( !(mcu_pio_get(boot_board_config.hw_req.port, 0) & (1<<boot_board_config.hw_req.pin)) ){
		*bootloader_start = 0;
		return 0;
	}

	if ( *bootloader_start == boot_board_config.sw_req_value ){
		*bootloader_start = 0;
		return 0;
	}

	return 1;
}


#ifdef DEBUG_BOOTLOADER
static int debug_write_func(const void * buf, int nbyte){
	memset(mcu_debug_buffer, 0, MCU_DEBUG_BUFFER_SIZE);
	memcpy(mcu_debug_buffer, buf, nbyte);
	mcu_priv_write_debug_uart(NULL);
	return nbyte;
}
#endif

void init_hw(){
	mcu_action_t action;
	_mcu_core_initclock(1);
	_mcu_cortexm_priv_enable_interrupts(NULL); //Enable the interrupts

	delay_ms(50);

	//This only needs to be enabled for debugging
	action.handler.callback = 0;
	action.handler.context = 0;
	action.channel = 0;
	action.prio = 128;
	mcu_uart_setaction(0, &action);

	action.prio = 1;
	mcu_usb_setaction(0, &action);


#ifdef DEBUG_BOOTLOADER
	u32 * bootloader_start = (u32*)boot_board_config.sw_req_loc;
	mcu_debug_init();
	dsetmode(0);
	dsetwritefunc(debug_write_func);
	dstr("STARTING UP\n");

	if ( (uint32_t)stack_ptr == 0xFFFFFFFF ){
		dstr("Stack pointer is invalid\n");
	}

	if ( !(mcu_pio_get(boot_board_config.hw_req.port, 0) & (1<<boot_board_config.hw_req.pin)) ){
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
		attr.o_pinmask = (1<<mcu_board_config.led.pin);
		attr.o_flags = PIO_FLAG_SET_OUTPUT | PIO_FLAG_IS_DIRONLY;
		mcu_pio_setattr(mcu_board_config.led.port, &attr);
		if( mcu_board_config.o_flags & MCU_BOARD_CONFIG_FLAG_LED_ACTIVE_HIGH ){
			//LED is active low
			mcu_pio_setmask(mcu_board_config.led.port, (void*)attr.o_pinmask);
		} else {
			mcu_pio_clrmask(mcu_board_config.led.port, (void*)attr.o_pinmask);
		}
	}
}

/*! \details This function turns the green LED off by setting the line to high impedance.
 *
 */
void gled_off(){
	if( mcu_board_config.led.port != 255 ){
		pio_attr_t attr;
		attr.o_pinmask = (1<<mcu_board_config.led.pin);
		attr.o_flags = PIO_FLAG_SET_INPUT | PIO_FLAG_IS_DIRONLY;
		mcu_pio_setattr(mcu_board_config.led.port, &attr);
	}
}

void _mcu_core_fault_handler(){}
void _mcu_core_hardfault_handler(){}
void _mcu_core_memfault_handler(){}
void _mcu_core_busfault_handler(){}
void _mcu_core_usagefault_handler(){}

/*! @} */

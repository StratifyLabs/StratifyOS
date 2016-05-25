

//#include "config.h"
#include <sys/lock.h>
#include "mcu/mcu.h"
#include "iface/dev/usb.h"
#include "iface/device_config.h"
#include "mcu/core.h"
#include "mcu/uart.h"
#include "mcu/usb.h"
#include "mcu/debug.h"
#include "stratify/usb_dev.h"
#include "mcu/boot_debug.h"
#include "boot_link.h"
#include "boot_config.h"


#define USB_DEV_DFU_TRANSFER_SIZE 1024
char dnload_buf[USB_DEV_DFU_TRANSFER_SIZE];
#define SW_BOOT_APP_OVERRIDE 0x55664422

void exec_bootloader(void * args){
	//write SW location with key and then reset
	u32 * bootloader_start = (u32*)boot_board_config.sw_req_loc;
	*bootloader_start = boot_board_config.sw_req_value;
	mcu_core_reset(0, 0);
}

void boot_api_event(int event, void * args){
	if( boot_board_config.event != 0 ){
		boot_board_config.event(event, args);
	}
}

extern u32 _etext;

const bootloader_api_t _mcu_core_bootloader_api = {
		.code_size = (u32)&_etext,
		.exec = exec_bootloader,
		.usb_dev_priv_init = usb_dev_priv_init,
		.event = boot_api_event
};



void gled_on(void);
void gled_off(void);

void init_hw(void);

static void * stack_ptr;
static void (*app_reset)(void);


void run_bootloader(void);

void delay_ms(int ms){
	int i;
	for(i=0; i < ms; i++){
		_mcu_core_delay_us(1000);
	}
}

void led_flash(void){
	gled_on();
	delay_ms(500);
	gled_off();
	delay_ms(500);
}

void led_error(void){
	while(1){
		led_flash();
	}
}

void led_flash_run_bootloader(void){
	int i;
	for(i=0; i < 3; i++){
		gled_on();
		delay_ms(3);
		gled_off();
		delay_ms(3);
	}
}


static int check_run_app(void);
void run_bootloader(void);

/*! \details
 */
int boot_main(void){
	stack_ptr = (void*)(((uint32_t*)boot_board_config.program_start_addr)[0]);
	app_reset = (void (*)(void))( (((uint32_t*)boot_board_config.program_start_addr)[1]) );

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

void boot_event(int event, void * args){
	if( boot_board_config.event != 0 ){
		boot_board_config.event(event, args);
	}
}


void run_bootloader(void){
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
int check_run_app(void){
	//! \todo Check to see if end of text is less than app program start
	u32 * bootloader_start = (u32*)boot_board_config.sw_req_loc;

	if ( (uint32_t)stack_ptr == 0xFFFFFFFF ){
		//code is not valid
		*bootloader_start = 0;
		return 0;
	}

	_mcu_core_delay_us(500);

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

void init_hw(void){
	mcu_action_t action;
	_mcu_core_initclock(1);
	_mcu_core_priv_enable_interrupts(NULL); //Enable the interrupts

	delay_ms(50);

	//This only needs to be enabled for debugging
	action.callback = 0;
	action.context = 0;
	action.channel = 0;
	action.prio = 2;
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

void gled_on(void){
	pio_attr_t attr;
	attr.mask = (1<<mcu_board_config.led.pin);
	attr.mode = PIO_MODE_OUTPUT | PIO_MODE_DIRONLY;
	mcu_pio_setattr(mcu_board_config.led.port, &attr);
	if( mcu_board_config.flags & MCU_BOARD_CONFIG_FLAG_LED_ACTIVE_HIGH ){
		//LED is active low
		mcu_pio_setmask(mcu_board_config.led.port, (void*)attr.mask);
	} else {
		mcu_pio_clrmask(mcu_board_config.led.port, (void*)attr.mask);
	}
}

/*! \details This function turns the green LED off by setting the line to high impedance.
 *
 */
void gled_off(void){
	pio_attr_t attr;
	attr.mask = (1<<mcu_board_config.led.pin);
	attr.mode = PIO_MODE_INPUT | PIO_MODE_DIRONLY;
	mcu_pio_setattr(mcu_board_config.led.port, &attr);
}

void _mcu_core_fault_handler(void){}
void _mcu_core_hardfault_handler(void){}
void _mcu_core_memfault_handler(void){}
void _mcu_core_busfault_handler(void){}
void _mcu_core_usagefault_handler(void){}

/*! @} */

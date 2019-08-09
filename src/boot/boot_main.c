
#include "mcu/arch.h"

//#include "config.h"
#include <sys/lock.h>
#include "mcu/mcu.h"
#include "sos/sos.h"
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


void exec_bootloader(void * args){
	//write SW location with key and then reset
	u32 * bootloader_start = (u32*)boot_board_config.sw_req_loc;
	*bootloader_start = boot_board_config.sw_req_value;
	cortexm_reset(0);
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


void init_hw();

static void * stack_ptr;
static void (*app_reset)();

void run_bootloader();

void led_flash(){
	sos_led_svcall_enable(0);
	cortexm_delay_ms(500);
	sos_led_svcall_disable(0);
	cortexm_delay_ms(500);
}

void led_flash_run_bootloader(){
	int i;
	for(i=0; i < 3; i++){

		sos_led_svcall_enable(0);
		cortexm_delay_ms(50);
		sos_led_svcall_disable(0);
		cortexm_delay_ms(50);
	}
}


static int check_run_app();
void run_bootloader();

/*! \details
 */
int boot_main(){

	boot_event(BOOT_EVENT_START, 0);

	stack_ptr = (void*)(((u32*)boot_board_config.program_start_addr)[0]);
	app_reset = (void (*)())( (((u32*)boot_board_config.program_start_addr)[1]) );

	if ( check_run_app() ){
		boot_event(BOOT_EVENT_RUN_APP, 0);
		//assign stack pointer to stack value
		app_reset();
	} else {
		boot_event(BOOT_EVENT_RUN_BOOTLOADER, 0);
		run_bootloader();
	}

	while(1);
	return 0;
}

void run_bootloader(){
	init_hw();

	led_flash_run_bootloader();

	//initialize link and run link update
	dstr("Link Start\n");
	boot_link_update((void*)boot_board_config.link_transport_driver);
}

/*! \details This function checks to see if the application should be run
 * or if the device should enter DFU mode.
 * \return Non-zero if the application should be run.
 */
int check_run_app(){
	//! \todo Check to see if end of text is less than app program start
	volatile u32 * bootloader_start = (u32*)boot_board_config.sw_req_loc;
	u32 hw_req_value;
	u32 pio_value;
	pio_attr_t pio_attr;
	devfs_handle_t hw_req_handle;

	boot_event(BOOT_EVENT_CHECK_APP_EXISTS, 0);
	if ( (u32)stack_ptr == 0xFFFFFFFF ){
		//code is not valid
		*bootloader_start = 0;
		return 0;
	}


	boot_event(BOOT_EVENT_CHECK_SOFTWARE_BOOT_REQUEST, 0);
	if ( *bootloader_start == boot_board_config.sw_req_value ){
		*bootloader_start = 0;
		return 0;
	}

	boot_event(BOOT_EVENT_CHECK_HARDWARE_BOOT_REQUEST, 0);
	if( boot_board_config.hw_req.port != 0xff ){

		hw_req_handle.port = boot_board_config.hw_req.port;
		hw_req_handle.config = 0;
		hw_req_handle.state = 0;
		pio_attr.o_pinmask = (1<<boot_board_config.hw_req.pin);

		pio_attr.o_flags = PIO_FLAG_SET_INPUT;
		if( boot_board_config.o_flags & BOOT_BOARD_CONFIG_FLAG_HW_REQ_PULLUP ){
			pio_attr.o_flags |= PIO_FLAG_IS_PULLUP;
		} else if( boot_board_config.o_flags & BOOT_BOARD_CONFIG_FLAG_HW_REQ_PULLDOWN ){
			pio_attr.o_flags |= PIO_FLAG_IS_PULLDOWN;
		}
		mcu_pio_setattr(&hw_req_handle, &pio_attr);

		mcu_pio_get(&hw_req_handle, &pio_value);
		hw_req_value = ((pio_value & pio_attr.o_pinmask) != 0);

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
	boot_event(BOOT_EVENT_INIT_CLOCK, 0);
	mcu_core_initclock(1);

#if defined DEBUG_BOOTLOADER
	if( mcu_debug_init() < 0 ){
		sos_led_root_error(0);
	}

	dsetmode(0);
	dsetwritefunc(debug_write_func);

	dstr("STACK:"); dhex((u32)stack_ptr); dstr("\n");
	dstr("APP:"); dhex((u32)app_reset); dstr("\n");
#endif
	cortexm_delay_ms(50);
	cortexm_enable_interrupts(); //Enable the interrupts
	boot_event(BOOT_EVENT_INIT, 0);
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
void mcu_core_wdt_isr(){}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr){
	return 0;
}

/*! @} */

/* Copyright 2011-2016 Tyler Gilbert; 
 * This file is part of Stratify OS.
 *
 * Stratify OS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stratify OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * 
 */

#include "mcu/mcu.h"

void _mcu_core_reset_handler();
void _mcu_core_nmi_isr() MCU_WEAK;
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) MCU_WEAK;

/*! \details
 */
void _mcu_core_fault_handler() MCU_WEAK;
void _mcu_core_hardfault_handler() MCU_ALIAS(_mcu_core_fault_handler);
void _mcu_core_memfault_handler() MCU_ALIAS(_mcu_core_fault_handler);
void _mcu_core_busfault_handler() MCU_ALIAS(_mcu_core_fault_handler);
void _mcu_core_usagefault_handler() MCU_ALIAS(_mcu_core_fault_handler);

void dled_fault_flash() MCU_WEAK;
void dled_nmi_flash() MCU_WEAK;
void dled_isr_flash() MCU_WEAK;

void _mcu_core_default_isr() MCU_WEAK;
void _mcu_core_os_handler() MCU_WEAK;
void _mcu_core_svcall_handler() MCU_ALIAS(_mcu_core_os_handler); //Supervisor call (OS environment)
void _mcu_core_debugmon_handler() MCU_ALIAS(_mcu_core_os_handler);
void _mcu_core_pendsv_handler() MCU_ALIAS(_mcu_core_os_handler); //Interrupt request for system-level service (OS environment)
void _mcu_core_systick_handler() MCU_ALIAS(_mcu_core_os_handler);


//ISR's -- weakly bound to default handler
void _mcu_core_wdt_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_flash_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_tmr0_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_tmr1_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_tmr2_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_tmr3_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_rit_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_uart0_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_i2c0_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_ssp0_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_ssp1_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_pio0_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_pio1_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_adc_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_bod_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_usb_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_usbfiq_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_usbwakeup_isr() MCU_ALIAS(_mcu_core_default_isr);

void _mcu_core_eint0_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint1_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint2_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint3_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint4_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint5_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint6_isr() MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint7_isr() MCU_ALIAS(_mcu_core_default_isr);

void (* const _mcu_core_vector_table[])() __attribute__ ((section(".startup"))) = {
		(void *)&_top_of_stack, // The initial stack pointer
		_mcu_core_reset_handler, 	 // The reset handler
		_mcu_core_nmi_isr, 	// The NMI handler -14
		_mcu_core_hardfault_handler, // The hard fault handler -13
		_mcu_core_memfault_handler, // The MPU fault handler -12
		_mcu_core_busfault_handler, // The bus fault handler -11
		_mcu_core_usagefault_handler, // The usage fault handler -10
		0, 	// Reserved
		0, 	// Reserved
		0, 	// Reserved
		0, 	// Reserved
		_mcu_core_svcall_handler, 	// SVCall handler -5
		_mcu_core_debugmon_handler, // Debug monitor handler -4
 		0, 	// Reserved
		_mcu_core_pendsv_handler, 	// The PendSV handler -2
		_mcu_core_systick_handler, // The SysTick handler -1

		_mcu_core_eint0_isr,  //0
		_mcu_core_eint1_isr,
		_mcu_core_eint2_isr,
		_mcu_core_eint3_isr,
		_mcu_core_eint4_isr,
		_mcu_core_eint5_isr,
		_mcu_core_eint6_isr,
		_mcu_core_eint7_isr,
		_mcu_core_pio0_isr, // PIO INT0 8
		_mcu_core_pio1_isr, // PIO INT1 9
		_mcu_core_os_handler, //reserved 10
		_mcu_core_os_handler, //reserved 11
		_mcu_core_rit_isr, //RIT 12
		_mcu_core_os_handler, //reserved 13
		_mcu_core_ssp1_isr, //SSP 1 14
		_mcu_core_i2c0_isr, // I2C0 -- 15
		_mcu_core_tmr0_isr, // CT16B0 (16-bit Timer 0)
		_mcu_core_tmr1_isr, // CT16B1 (16-bit Timer 1)
		_mcu_core_tmr2_isr, // CT32B0 (32-bit Timer 0)
		_mcu_core_tmr3_isr, // CT32B1 (32-bit Timer 1)
		_mcu_core_ssp0_isr, //SSP0 20
		_mcu_core_uart0_isr, //UART0 --21
		_mcu_core_usb_isr, //USB IRQ 22
		_mcu_core_usbfiq_isr, //USB FIQ 23
		_mcu_core_adc_isr, // ADC  (A/D Converter) 24
		_mcu_core_wdt_isr, // WDT  (Watchdog Timer) 25
		_mcu_core_bod_isr, // BOD  (Brownout Detect) 26
		_mcu_core_flash_isr, // flash 27
		_mcu_core_os_handler, // resd 28
		_mcu_core_os_handler, // resd 29
		_mcu_core_usbwakeup_isr, //resd 30
		_mcu_core_os_handler //resd 31
};

static void core_init();
const char sys_proc_name[] = "sys";

void core_init(){
	uint32_t *src, *dest;
	//Copy the data section stored in Flash to RAM
	src = &_etext;
	for(dest = &_data; dest < &_edata; ){ *dest++ = *src++; }
	for(src = &_bss; src < &_ebss; ) *src++ = 0;

	//Zero out the sysmem as well
	for(src = &_sys; src < &_esys; ) *src++ = 0;

	//Re-entrancy initialization
	_REENT->procmem_base = (proc_mem_t*)&_ebss;
	_REENT->procmem_base->size = 0;
	_REENT->procmem_base->proc_name = sys_proc_name;

	memset(_REENT->procmem_base->open_file, 0, sizeof(open_file_t)*OPEN_MAX);

	//Initialize the global mutexes
	__lock_init_recursive_global(__malloc_lock_object);
	_REENT->procmem_base->__malloc_lock_object.flags |= PTHREAD_MUTEX_FLAGS_PSHARED; //Make the malloc lock pshared

	__lock_init_global(__tz_lock_object);
	__lock_init_recursive_global(__atexit_lock);
	__lock_init_recursive_global(__sfp_lock);
	__lock_init_recursive_global(__sinit_lock);
	__lock_init_recursive_global(__env_lock_object);

	//This is the de facto MCU initialization -- turn off power to peripherals that must be "open()"ed.
	LPC_SYSCON->SYSAHBCLKCTRL = (1<<PCGPIO)|(1<<PCROM)|(1<<PCRAM)|(1<<PCFLASHREG)|(1<<PCFLASHARRAY)|(1<<PCIOCON)|(1<<PCSYS);
}

extern int _main();

void _mcu_core_getserialno(uint32_t * serial_number){
	_mcu_lpc_flash_get_serialno(serial_number);
}

void _mcu_core_reset_handler(){
	core_init();
	_mcu_core_priv_setvectortableaddr((void*)_mcu_core_vector_table);
	_main(); //This function should never return
	while(1);
}

/*! \details
 */
void _mcu_core_nmi_isr(){
}

/*! \details
 */
void _mcu_core_fault_handler(){
}

/*! \brief
 * \details
 */
void _mcu_core_os_handler(){
}

/*! \brief
 * \details
 */
void _mcu_core_default_isr(){

}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr){
	return 0;
}

void dled_fault_flash(){}
void dled_nmi_flash(){}
void dled_isr_flash(){}

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
#include "mcu/debug.h"
#include "iface/dev/bootloader.h"

static void core_init(void);
static const char sys_proc_name[] = "sys";
extern int _main(void);

const bootloader_api_t _mcu_core_bootloader_api MCU_WEAK;
const bootloader_api_t _mcu_core_bootloader_api = {
		.code_size = 0,
};


void _mcu_core_hardware_id(void) MCU_ALIAS(_mcu_core_default_isr);

void _mcu_core_reset_handler(void) __attribute__ ((section(".reset_vector")));
void _mcu_core_nmi_isr(void) MCU_WEAK;
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) MCU_WEAK;

void _mcu_core_hardfault_handler(void) MCU_WEAK;
void _mcu_core_memfault_handler(void) MCU_WEAK;
void _mcu_core_busfault_handler(void) MCU_WEAK;
void _mcu_core_usagefault_handler(void) MCU_WEAK;

void dled_fault_flash(void) MCU_WEAK;
void dled_nmi_flash(void) MCU_WEAK;
void dled_isr_flash(void) MCU_WEAK;

void _mcu_core_default_isr(void) MCU_WEAK;
void _mcu_core_os_handler(void) MCU_WEAK;
void _mcu_core_svcall_handler(void);
void _mcu_core_debugmon_handler(void) MCU_ALIAS(_mcu_core_os_handler);
void _mcu_core_pendsv_handler(void);
void _mcu_core_systick_handler(void);

#define _DECLARE_ISR(name) void _mcu_core_##name##_isr(void) MCU_ALIAS(_mcu_core_default_isr)
#define _ISR(name) _mcu_core_##name##_isr

#if defined __lpc43xx
//ISR's -- weakly bound to default handler
_DECLARE_ISR(dac0);
_DECLARE_ISR(m0core);
_DECLARE_ISR(dma);
_DECLARE_ISR(sw0);
_DECLARE_ISR(sw1);
_DECLARE_ISR(enet0);
_DECLARE_ISR(sdio0);
_DECLARE_ISR(lcd0);
_DECLARE_ISR(usb0);
_DECLARE_ISR(usb1);
_DECLARE_ISR(sct0);
_DECLARE_ISR(rit0);
_DECLARE_ISR(tmr0);
_DECLARE_ISR(tmr1);
_DECLARE_ISR(tmr2);
_DECLARE_ISR(tmr3);
_DECLARE_ISR(mcpwm0);
_DECLARE_ISR(adc0);
_DECLARE_ISR(i2c0);
_DECLARE_ISR(i2c1);
_DECLARE_ISR(spi0);
_DECLARE_ISR(adc1);
_DECLARE_ISR(ssp0);
_DECLARE_ISR(ssp1);
_DECLARE_ISR(uart0);
_DECLARE_ISR(uart1);
_DECLARE_ISR(uart2);
_DECLARE_ISR(uart3);
_DECLARE_ISR(i2s0);
_DECLARE_ISR(i2s1);
_DECLARE_ISR(sw3);
_DECLARE_ISR(sgpio0);
_DECLARE_ISR(eint0);
_DECLARE_ISR(eint1);
_DECLARE_ISR(eint2);
_DECLARE_ISR(eint3);
_DECLARE_ISR(eint4);
_DECLARE_ISR(eint5);
_DECLARE_ISR(eint6);
_DECLARE_ISR(eint7);
_DECLARE_ISR(pio0);
_DECLARE_ISR(pio1);
_DECLARE_ISR(eventrouter0);
_DECLARE_ISR(can1);
_DECLARE_ISR(sw4);
_DECLARE_ISR(sw5);
_DECLARE_ISR(atmr0);
_DECLARE_ISR(rtc0);
_DECLARE_ISR(sw6);
_DECLARE_ISR(wdt0);
_DECLARE_ISR(sw7);
_DECLARE_ISR(can0);
_DECLARE_ISR(qei);
#endif

#if defined LPCXX7X_8X
//ISR's -- weakly bound to default handler
_DECLARE_ISR(wdt);  //0
_DECLARE_ISR(tmr0);
_DECLARE_ISR(tmr1);
_DECLARE_ISR(tmr2);
_DECLARE_ISR(tmr3);
_DECLARE_ISR(uart0);
_DECLARE_ISR(uart1);
_DECLARE_ISR(uart2);
_DECLARE_ISR(uart3);
_DECLARE_ISR(pwm1);
_DECLARE_ISR(i2c0); //10
_DECLARE_ISR(i2c1);
_DECLARE_ISR(i2c2);
_DECLARE_ISR(sw0);
_DECLARE_ISR(ssp0);
_DECLARE_ISR(ssp1);
_DECLARE_ISR(pll0);
_DECLARE_ISR(rtc0);
_DECLARE_ISR(eint0);
_DECLARE_ISR(eint1);
_DECLARE_ISR(eint2); //20
_DECLARE_ISR(eint3);
_DECLARE_ISR(adc);
_DECLARE_ISR(bod);
_DECLARE_ISR(usb0);
_DECLARE_ISR(can0);
_DECLARE_ISR(dma);
_DECLARE_ISR(i2s0);
_DECLARE_ISR(enet0);
_DECLARE_ISR(mci0);
_DECLARE_ISR(mcpwm0); //30
_DECLARE_ISR(qei0);
_DECLARE_ISR(pll1);
_DECLARE_ISR(usb_activity);
_DECLARE_ISR(can_activity);
_DECLARE_ISR(uart4);
_DECLARE_ISR(ssp2);
_DECLARE_ISR(lcd0);
_DECLARE_ISR(gpio0);
_DECLARE_ISR(pwm0);
_DECLARE_ISR(eeprom0); //40
_DECLARE_ISR(cmp0);
_DECLARE_ISR(cmp1);


#endif

#if defined __lpc17xx
//ISR's -- weakly bound to default handler
_DECLARE_ISR(dac);
_DECLARE_ISR(wdt);
_DECLARE_ISR(tmr0);
_DECLARE_ISR(tmr1);
_DECLARE_ISR(tmr2);
_DECLARE_ISR(tmr3);

_DECLARE_ISR(uart0);
_DECLARE_ISR(uart1);
_DECLARE_ISR(uart2);
_DECLARE_ISR(uart3);

void _mcu_core_pwm1_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_i2c0_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_i2c1_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_i2c2_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_ssp0_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_ssp1_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_pll0_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_rtc_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint0_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint1_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint2_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint3_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_adc_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_bod_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_usb_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_can_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_dma_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_i2s_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_enet_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_mcpwm_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_qei_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_pll1_isr(void) MCU_ALIAS(_mcu_core_default_isr);
#ifdef LPCXX7X_8X
void _mcu_core_mci_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_usbactivity_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_canactivity_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_uart4_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_ssp2_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_lcd_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_pio_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_pwm0_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eeprom_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_cmp0_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_cmp1_isr(void) MCU_ALIAS(_mcu_core_default_isr);
#else
void _mcu_core_rit_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_spi_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_sw0_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_sw1_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_sw2_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_sw3_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_sw4_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_sw5_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_sw6_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_sw7_isr(void) MCU_ALIAS(_mcu_core_default_isr);
#endif

#endif

/*! \details This is the startup code which gets written to
 * address 0 (or wherever the text starts if there is another bootloader) in flash memory
 */

void (* const _mcu_core_vector_table[])(void) __attribute__ ((section(".startup"))) = {
		// Core Level - CM3
		(void*)&_top_of_stack,					// The initial stack pointer
		_mcu_core_reset_handler,						// The reset handler
		_mcu_core_nmi_isr,							// The NMI handler
		_mcu_core_hardfault_handler,					// The hard fault handler
		_mcu_core_memfault_handler,					// The MPU fault handler
		_mcu_core_busfault_handler,					// The bus fault handler
		_mcu_core_usagefault_handler,				// The usage fault handler
		_mcu_core_hardware_id,					// Reserved -- this is the checksum addr for ISP programming 0x1C
		0,										// Reserved -- this is the hwpl security word 0x20
		(void*)&_mcu_core_bootloader_api,										// Reserved -- this is the kernel signature checksum value 0x24
		0,										// Reserved
		_mcu_core_svcall_handler,					// SVCall handler
		_mcu_core_debugmon_handler,					// Debug monitor handler
		0,										// Reserved
		_mcu_core_pendsv_handler,					// The PendSV handler
		_mcu_core_systick_handler,					// The SysTick handler
		//Non Cortex M3 interrupts

#if defined __lpc43xx
		_ISR(dac0),  //0
		_ISR(m0core),
		_ISR(dma),
		_ISR(sw0),
		_ISR(sw1),
		_ISR(enet0),
		_ISR(sdio0),
		_ISR(lcd0),
		_ISR(usb0),
		_ISR(usb1),
		_ISR(sct0), //10
		_ISR(rit0),
		_ISR(tmr0),
		_ISR(tmr1),
		_ISR(tmr2),
		_ISR(tmr3),
		_ISR(mcpwm0),
		_ISR(adc0),
		_ISR(i2c0),
		_ISR(i2c1),
		_ISR(spi0), //20
		_ISR(adc1),
		_ISR(ssp0),
		_ISR(ssp1),
		_ISR(uart0),
		_ISR(uart1),
		_ISR(uart2),
		_ISR(uart3),
		_ISR(i2s0),
		_ISR(i2s1),
		_ISR(sw3), //30
		_ISR(sgpio0),
		_ISR(eint0),
		_ISR(eint1),
		_ISR(eint2),
		_ISR(eint3),
		_ISR(eint4),
		_ISR(eint5),
		_ISR(eint6),
		_ISR(eint7),
		_ISR(pio0), //40
		_ISR(pio1),
		_ISR(eventrouter0),
		_ISR(can1),
		_ISR(sw4),
		_ISR(sw5),
		_ISR(atmr0),
		_ISR(rtc0),
		_ISR(sw6),
		_ISR(wdt0),
		_ISR(sw7), //50
		_ISR(can0),
		_ISR(qei)
#endif

#if defined LPCXX7X_8X

		_ISR(wdt),  //0
		_ISR(tmr0),
		_ISR(tmr1),
		_ISR(tmr2),
		_ISR(tmr3),
		_ISR(uart0),
		_ISR(uart1),
		_ISR(uart2),
		_ISR(uart3),
		_ISR(pwm1),
		_ISR(i2c0), //10
		_ISR(i2c1),
		_ISR(i2c2),
		_ISR(sw0),
		_ISR(ssp0),
		_ISR(ssp1),
		_ISR(pll0),
		_ISR(rtc0),
		_ISR(eint0),
		_ISR(eint1),
		_ISR(eint2), //20
		_ISR(eint3),
		_ISR(adc),
		_ISR(bod),
		_ISR(usb0),
		_ISR(can0),
		_ISR(dma),
		_ISR(i2s0),
		_ISR(enet0),
		_ISR(mci0),
		_ISR(mcpwm0), //30
		_ISR(qei0),
		_ISR(pll1),
		_ISR(usb_activity),
		_ISR(can_activity),
		_ISR(uart4),
		_ISR(ssp2),
		_ISR(lcd0),
		_ISR(gpio0),
		_ISR(pwm0),
		_ISR(eeprom0), //40
		_ISR(cmp0),
		_ISR(cmp1)
#endif

#if defined __lpc17xx


#endif

};

int _mcu_core_getsignature(int port, void * arg){
	const uint32_t ** ptr;
	ptr = (const uint32_t**)_mcu_core_vector_table;
	return (uint32_t)(ptr[9]);
}

void _mcu_core_getserialno(sn_t * serial_number){
	_mcu_lpc_flash_get_serialno(serial_number->sn);
}

void core_init(void){
	uint32_t *src, *dest;
	src = &_etext; //point src to copy of data that is stored in flash
	for(dest = &_data; dest < &_edata; ){ *dest++ = *src++; } //Copy from flash to RAM (data)
	for(src = &_bss; src < &_ebss; ) *src++ = 0; //Zero out BSS section
	for(src = &_sys; src < &_esys; ) *src++ = 0; //Zero out sysmem


	//Re-entrancy initialization
	//If the program faults on the next line, make sure the etext and data are aligned properly in the linker script (4 byte boundary)
	_REENT->procmem_base = (proc_mem_t*)&_ebss;
	_REENT->procmem_base->size = 0;
	_REENT->procmem_base->sigactions = NULL;
	_REENT->procmem_base->siginfos = NULL;
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
#if defined LPC_SC
	LPC_SC->PCONP = (1<<PCGPIO)|(1<<PCRTC);
#else

#endif
}

void _mcu_core_reset_handler(void){
	core_init();
	_mcu_core_priv_setvectortableaddr((void*)_mcu_core_vector_table);
	_main(); //This function should never return
	while(1);
}

/*! \details
 */
void _mcu_core_nmi_isr(void){
	while(1){
		dled_nmi_flash();
	}
}

/*! \brief
 * \details
 */
void _mcu_core_os_handler(void){
	return;
}

/*! \brief
 * \details
 */
void _mcu_core_default_isr(void){
	while(1){
		dled_isr_flash();
	}
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr){
	return 0;
}

void dled_fault_flash(void){
}
void dled_nmi_flash(void){
}
void dled_isr_flash(void){
}





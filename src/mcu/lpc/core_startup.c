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

//ISR's -- weakly bound to default handler
void _mcu_core_wdt_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_tmr0_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_tmr1_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_tmr2_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_tmr3_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_uart0_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_uart1_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_uart2_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_uart3_isr(void) MCU_ALIAS(_mcu_core_default_isr);
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
		_mcu_core_wdt_isr              , // 16: WDT
		_mcu_core_tmr0_isr           , // 17: tmr0
		_mcu_core_tmr1_isr           , // 18: tmr1
		_mcu_core_tmr2_isr           , // 19: tmr2
		_mcu_core_tmr3_isr           , // 20: tmr3
		_mcu_core_uart0_isr            , // 21: uart0
		_mcu_core_uart1_isr            , // 22: uart1
		_mcu_core_uart2_isr            , // 23: uart2
		_mcu_core_uart3_isr            , // 24: uart3
		_mcu_core_pwm1_isr             , // 25: PWM1
		_mcu_core_i2c0_isr             , // 26: I2C0
		_mcu_core_i2c1_isr             , // 27: I2C1
		_mcu_core_i2c2_isr             , // 28: I2C2
#ifdef __lpc17xx
		_mcu_core_spi_isr              , // 29: SPI
#endif
#ifdef LPCXX7X_8X
		_mcu_core_default_isr, //reserved
#endif
		_mcu_core_ssp0_isr             , // 30: SSP0
		_mcu_core_ssp1_isr             , // 31: SSP1
		_mcu_core_pll0_isr             , // 32: PLL0 Lock (Main PLL)
		_mcu_core_rtc_isr              , // 33: Real Time Clock
		_mcu_core_eint0_isr            , // 34: External Interrupt 0
		_mcu_core_eint1_isr            , // 35: External Interrupt 1
		_mcu_core_eint2_isr            , // 36: External Interrupt 2
		_mcu_core_eint3_isr            , // 37: External Interrupt 3 -- shared with GPIO
		_mcu_core_adc_isr              , // 38: A/D Converter
		_mcu_core_bod_isr              , // 39: Brown-Out Detect
		_mcu_core_usb_isr              , // 40: USB
		_mcu_core_can_isr              , // 41: CAN
		_mcu_core_dma_isr              , // 42: General Purpose DMA
		_mcu_core_i2s_isr              , // 43: I2S
		_mcu_core_enet_isr             , // 44: Ethernet
#ifdef LPCXX7X_8X
		_mcu_core_mci_isr              , // 45: MCI (SD/MMC)
#endif
#ifdef __lpc17xx
		_mcu_core_rit_isr              , // 45: RIT
#endif
		_mcu_core_mcpwm_isr            , // 46: Motor Control PWM
		_mcu_core_qei_isr              , // 47: Quadrature Encoder Interface
		_mcu_core_pll1_isr             , // 48: PLL1 Lock (USB PLL)

#ifdef LPCXX7X_8X
		_mcu_core_usbactivity_isr,  //
		_mcu_core_canactivity_isr,
		_mcu_core_uart4_isr,
		_mcu_core_ssp2_isr,
		_mcu_core_lcd_isr,
		_mcu_core_pio_isr,
		_mcu_core_pwm0_isr,
		_mcu_core_eeprom_isr,
		_mcu_core_cmp0_isr,
		_mcu_core_cmp1_isr
#else
		_mcu_core_sw0_isr,
		_mcu_core_sw1_isr,
		_mcu_core_sw2_isr,
		_mcu_core_sw3_isr,
		_mcu_core_sw4_isr,
		_mcu_core_sw5_isr,
		_mcu_core_sw6_isr,
		_mcu_core_sw7_isr
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
	LPC_SC->PCONP = (1<<PCGPIO)|(1<<PCRTC);
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





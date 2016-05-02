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

void _mcu_core_reset_handler(void);
void _mcu_core_nmi_isr(void) MCU_WEAK;
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) MCU_WEAK;

/*! \details
 */
void _mcu_core_fault_handler(void) MCU_WEAK;
void _mcu_core_hardfault_handler(void) MCU_ALIAS(_mcu_core_fault_handler);

void dled_fault_flash(void) MCU_WEAK;
void dled_nmi_flash(void) MCU_WEAK;
void dled_isr_flash(void) MCU_WEAK;

void _mcu_core_default_isr(void) MCU_WEAK;
void _mcu_core_os_handler(void) MCU_WEAK;
void _mcu_core_svcall_handler(void) MCU_ALIAS(_mcu_core_os_handler); //Supervisor call (OS environment)
void _mcu_core_pendsv_handler(void) MCU_ALIAS(_mcu_core_os_handler); //Interrupt request for system-level service (OS environment)
void _mcu_core_systick_handler(void) MCU_ALIAS(_mcu_core_os_handler);


//ISR's -- weakly bound to default handler
void _mcu_core_wdt_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_flash_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_sct0_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_mtr0_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_wkttmr_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_uart0_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_uart1_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_uart2_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_i2c0_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_i2c1_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_i2c2_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_i2c3_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_spi0_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_spi1_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_adcseqa_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_adcseqb_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_adcthres_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_adcover_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_cmp_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_bod_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_dma_isr(void) MCU_ALIAS(_mcu_core_default_isr);

void _mcu_core_eint0_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint1_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint2_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint3_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint4_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint5_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint6_isr(void) MCU_ALIAS(_mcu_core_default_isr);
void _mcu_core_eint7_isr(void) MCU_ALIAS(_mcu_core_default_isr);

void (* const _mcu_core_vector_table[])(void) __attribute__ ((section(".startup"))) = {
		(void *)&_top_of_stack, // The initial stack pointer
		_mcu_core_reset_handler, 	 // The reset handler
		_mcu_core_nmi_isr, 	// The NMI handler -14
		_mcu_core_hardfault_handler, // The hard fault handler -13
		0, // The MPU fault handler -12
		0, // The bus fault handler -11
		0, // The usage fault handler -10
		0, 	// Reserved
		0, 	// Reserved
		0, 	// Reserved
		0, 	// Reserved
		_mcu_core_svcall_handler, 	// SVCall handler -5
		0, // Debug monitor handler -4
 		0, 	// Reserved
		_mcu_core_pendsv_handler, 	// The PendSV handler -2
		_mcu_core_systick_handler, // The SysTick handler -1

		_mcu_core_spi0_isr,  //0
		_mcu_core_spi1_isr,
		0,
		_mcu_core_uart0_isr,
		_mcu_core_uart1_isr,
		_mcu_core_uart2_isr,
		0,
		_mcu_core_i2c1_isr,
		_mcu_core_i2c0_isr, // I2C0 8
		_mcu_core_sct0_isr, // SCT 9
		_mcu_core_mtr0_isr, //Multi-rate timer 10
		_mcu_core_cmp_isr, //Comparator 11
		_mcu_core_wdt_isr, //WDT 12
		_mcu_core_bod_isr, // 13
		_mcu_core_flash_isr, //FLASH 14
		_mcu_core_wkttmr_isr, // Wakeup timer -- 15
		_mcu_core_adcseqa_isr, // ADC SEQ A (16-bit Timer 0)
		_mcu_core_adcseqb_isr, // ADC SEQ B (16-bit Timer 1)
		_mcu_core_adcthres_isr, // ADC THRESHOLD (32-bit Timer 0)
		_mcu_core_adcover_isr, // ADC OVERRUN (32-bit Timer 1)
		_mcu_core_dma_isr, //DMA 20
		_mcu_core_i2c2_isr, //I2C2 --21
		_mcu_core_i2c3_isr, //I2C3 22
		0, //USB FIQ 23
		_mcu_core_eint0_isr, // Pin Interrupt 24
		_mcu_core_eint1_isr, // Pin Interrupt 25
		_mcu_core_eint2_isr, // Pin Interrupt 26
		_mcu_core_eint3_isr, // Pin Interrupt 27
		_mcu_core_eint4_isr, // Pin Interrupt 28
		_mcu_core_eint5_isr, // Pin Interrupt 29
		_mcu_core_eint6_isr, // Pin Interrupt 30
		_mcu_core_eint7_isr  // Pin Interrupt 31
};

static void core_init(void);
const char sys_proc_name[] = "sys";

void core_init(void){
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

	//This is the de facto MCU initialization -- turn off power to peripherals that must be "open()"ed.
	LPC_SYSCON->SYSAHBCLKCTRL = (1<<PCGPIO)|(1<<PCROM)|(1<<PCRAM)|(1<<PCFLASHREG)|(1<<PCFLASHARRAY)|(1<<PCIOCON)|(1<<PCSYS);
}

extern int _main(void);

void _mcu_core_getserialno(uint32_t * serial_number){
	_mcu_lpc_flash_get_serialno(serial_number);
}

void _mcu_core_reset_handler(void){
	core_init();
	//_mcu_core_priv_setvectortableaddr((void*)_mcu_core_vector_table);
	_main(); //This function should never return
	while(1);
}

/*! \details
 */
void _mcu_core_nmi_isr(void){
}

/*! \details
 */
void _mcu_core_fault_handler(void){
}

/*! \brief
 * \details
 */
void _mcu_core_os_handler(void){
}

/*! \brief
 * \details
 */
void _mcu_core_default_isr(void){

}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr){
	return 0;
}

void dled_fault_flash(void){}
void dled_nmi_flash(void){}
void dled_isr_flash(void){}

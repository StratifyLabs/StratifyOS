#include "mcu/mcu.h"
#include "mcu/core.h"
#include "device/sys.h"
#include "iface/dev/bootloader.h"


//This is 6 clock cycles per loop
void _delay_loop_1(uint32_t __count){
	asm volatile (
			"L_%=:"	"subs	%0, #1" "\n\t"
			"uxth	%0, %0" "\n\t"
			"cmp	%0, #0" "\n\t"
			"bne.n	L_%=" "\n\t"
			: "=r" (__count)
			: "0" (__count)
			);
}

static inline void _delay_us(uint32_t __us) MCU_ALWAYS_INLINE __attribute__((optimize("3")));

static inline void _delay_us(uint32_t __us){
	uint32_t __ticks;
	__ticks = mcu_board_config.core_cpu_freq / 1000000;
	__ticks *= __us;
	__ticks /= 6;
	_delay_loop_1(__ticks);
}

void _mcu_core_delay_us(u32 us){ _delay_us(us); }
void _mcu_core_delay_ms(u32 ms){ _delay_us(ms*1000); }


void _mcu_core_unprivileged_mode(){
	register uint32_t control;
	control = __get_CONTROL();
	control |= 0x01;
	__set_CONTROL(control);
}

void _mcu_core_thread_mode(){
	register uint32_t control;
	control = __get_CONTROL();
	control |= 0x02;
	__set_CONTROL(control);
}

void mcu_core_privcall(core_privcall_t call, void * args) MCU_WEAK;

void mcu_core_privcall(core_privcall_t call, void * args){
	asm volatile("SVC 0\n");
}

void _mcu_core_svcall_handler(){
	register uint32_t * frame;
	register core_privcall_t call;
	register void * args;
	asm volatile ("MRS %0, psp\n\t" : "=r" (frame) );
	call = (core_privcall_t)frame[0];
	args = (void*)(frame[1]);
	call(args);
}

int _mcu_core_priv_validate_callback(mcu_callback_t callback){
	return 0;
}

void _mcu_core_priv_reset(void * args){
	NVIC_SystemReset();
}

void _mcu_core_priv_disable_irq(void * x){
	NVIC_DisableIRQ((IRQn_Type)x);
}

void _mcu_core_priv_enable_irq(void * x){
	NVIC_EnableIRQ((IRQn_Type)x);
}

void _mcu_core_priv_disable_interrupts(void * args){
	asm volatile ("cpsid i");
}

void _mcu_core_priv_enable_interrupts(void * args){
	asm volatile ("cpsie i");
}

void _mcu_core_priv_get_stack_ptr(void * ptr){
	asm volatile ("MRS %0, msp\n\t" : "=r" (ptr) );
}

void _mcu_core_priv_set_stack_ptr(void * ptr){
	asm volatile ("MSR msp, %0\n\t" : : "r" (ptr) );
}

void _mcu_core_priv_get_thread_stack_ptr(void * ptr){
	void ** ptrp = (void**)ptr;
	void * result=NULL;
	asm volatile ("MRS %0, psp\n\t" : "=r" (result) );
	*ptrp = result;
}

void _mcu_core_priv_set_thread_stack_ptr(void * ptr){
	asm volatile ("MSR psp, %0\n\t" : : "r" (ptr) );
}

void _mcu_core_priv_bootloader_api(void * args){
	void * ptr;
	memcpy(&ptr, (void*)(36), sizeof(void*)); //get pointer to boot api
	memcpy(args, ptr, sizeof(bootloader_api_t)); //copy boot api
}


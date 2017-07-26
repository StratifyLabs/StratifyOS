

#ifndef MCU_CORTEXM_H_
#define MCU_CORTEXM_H_

#include "mcu/arch.h"
#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __link

int cortexm_sleep(int level) MCU_PRIV_CODE;

void cortexm_enable_interrupts(void * args) MCU_PRIV_CODE;
void cortexm_disable_interrupts(void * args) MCU_PRIV_CODE;
void cortexm_enable_irq(void * x) MCU_PRIV_CODE;
void cortexm_disable_irq(void * x) MCU_PRIV_CODE;
void cortexm_reset(void * args) MCU_PRIV_CODE;
void cortexm_get_stack_ptr(void * ptr) MCU_PRIV_CODE;
void cortexm_set_stack_ptr(void * ptr) MCU_PRIV_CODE;
void cortexm_get_thread_stack_ptr(void * ptr) MCU_PRIV_CODE;
void cortexm_set_thread_stack_ptr(void * ptr) MCU_PRIV_CODE;
int cortexm_validate_callback(mcu_callback_t callback) MCU_PRIV_CODE;
int cortexm_set_irq_prio(int irq, int prio) MCU_PRIV_CODE;

typedef void (*cortexm_svcall_t)(void*);

#ifndef __link
/*! \details This performs a privileged call.
 *
 */
void cortexm_svcall(cortexm_svcall_t call, void * args) __attribute__((optimize("1")));
#endif

void cortexm_set_unprivileged_mode() MCU_PRIV_CODE;
void cortexm_set_thread_mode() MCU_PRIV_CODE;

void cortexm_delay_us(u32 us);
void cortexm_delay_ms(u32 ms);

#define SYSTICK_CTRL_TICKINT (1<<1)

static inline void cortexm_enable_systick_irq() MCU_ALWAYS_INLINE;
void cortexm_enable_systick_irq(){
	SysTick->CTRL |= SYSTICK_CTRL_TICKINT;
}

static inline void cortexm_disable_systick_irq() MCU_ALWAYS_INLINE;
void cortexm_disable_systick_irq(){
	SysTick->CTRL &= ~SYSTICK_CTRL_TICKINT;
}

static inline void cortexm_set_vector_table_addr(void * addr) MCU_ALWAYS_INLINE;
void cortexm_set_vector_table_addr(void * addr){
	SCB->VTOR = (uint32_t)addr;
}

#define r(x) register long x asm("lr")

#endif

#ifdef __cplusplus
}
#endif


#endif /* MCU_CORTEXM_H_ */

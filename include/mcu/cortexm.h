

#ifndef MCU_CORTEXM_H_
#define MCU_CORTEXM_H_

#include "arch.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __link

int mcu_cortexm_sleep(int level) MCU_PRIV_CODE;

void mcu_cortexm_priv_enable_interrupts(void * args) MCU_PRIV_CODE;
void mcu_cortexm_priv_disable_interrupts(void * args) MCU_PRIV_CODE;
void mcu_cortexm_priv_enable_irq(void * x) MCU_PRIV_CODE;
void mcu_cortexm_priv_disable_irq(void * x) MCU_PRIV_CODE;
void mcu_cortexm_priv_reset(void * args) MCU_PRIV_CODE;
void mcu_cortexm_priv_get_stack_ptr(void * ptr) MCU_PRIV_CODE;
void mcu_cortexm_priv_set_stack_ptr(void * ptr) MCU_PRIV_CODE;
void mcu_cortexm_priv_get_thread_stack_ptr(void * ptr) MCU_PRIV_CODE;
void mcu_cortexm_priv_set_thread_stack_ptr(void * ptr) MCU_PRIV_CODE;
int mcu_cortexm_priv_validate_callback(mcu_callback_t callback) MCU_PRIV_CODE;
int mcu_cortexm_set_irq_prio(int irq, int prio) MCU_PRIV_CODE;

typedef void (*core_privcall_t)(void*);
typedef void (*cortexm_svcall_t)(void*);

#ifndef __link
/*! \details This performs a privileged call.
 *
 */
void mcu_core_privcall(core_privcall_t call, void * args) __attribute__((optimize("1")));
void mcu_cortexm_svcall(cortexm_svcall_t call, void * args) __attribute__((optimize("1")));
#endif

void mcu_cortexm_priv_set_unprivileged_mode() MCU_PRIV_CODE;
void mcu_cortexm_set_thread_mode() MCU_PRIV_CODE;

void mcu_cortexm_delay_us(u32 us);
void mcu_cortexm_delay_ms(u32 ms);

#define SYSTICK_CTRL_TICKINT (1<<1)

static inline void mcu_cortexm_enable_systick_irq() MCU_ALWAYS_INLINE;
void mcu_cortexm_enable_systick_irq(){
	SysTick->CTRL |= SYSTICK_CTRL_TICKINT;
}

static inline void mcu_cortexm_disable_systick_irq() MCU_ALWAYS_INLINE;
void mcu_cortexm_disable_systick_irq(){
	SysTick->CTRL &= ~SYSTICK_CTRL_TICKINT;
}

static inline void mcu_cortexm_priv_set_vector_table_addr(void * addr) MCU_ALWAYS_INLINE;
void mcu_cortexm_priv_set_vector_table_addr(void * addr){
	SCB->VTOR = (uint32_t)addr;
}

#define r(x) register long x asm("lr")

#endif

#ifdef __cplusplus
}
#endif


#endif /* MCU_CORTEXM_H_ */

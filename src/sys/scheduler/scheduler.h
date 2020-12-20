// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup SCHED Scheduler
 * @{
 *
 * \ingroup MULTITASK
 *
 * \details This software module is the CAOS scheduler.  It manages processes in multi-threaded
 * applications.  The basic approach is to execute the highest priority tasks in a round robin
 * fashion.  A task with a lower priority will only be executed if all higher priority tasks
 * are in the idle state.  Hardware interrupts have priority over all tasks (as well as any
 * callbacks which have been attached to those interrupts).
 *
 * The following is a checklist for things that must be done before calling the scheduler() function:
 *  - Enable the RTC -- if RTC is not enabled sleep() won't work properly
 *  - Call sched_hwcfg_init(): enables core processes and microsecond timer
 *  - Enable interrupts
 *  - Call scheduler_init()
 *  - Call scheduler()
 *
 *
 */


/*! \file */
#ifndef SCHEDULER_H_
#define SCHEDULER_H_

//#include "config.h"

#include <pthread.h>

#include "mcu/mcu.h"

#ifdef __cplusplus
extern "C" {
#endif

void scheduler_start(void *(*init)(void *));
void scheduler_init();

//Schedule Debugging
void scheduler_debug_print_tasks();
void scheduler_debug_print_active();
void scheduler_debug_print_executing();
void scheduler_debug_print_timers();

void scheduler_set_fault(int pid, int tid, fault_t * fault);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDULER_H_ */

/*! @} */

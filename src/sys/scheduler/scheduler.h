/* Copyright 2011-2018 Tyler Gilbert; 
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

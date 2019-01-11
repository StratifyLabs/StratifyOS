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

/*! \addtogroup SCHED
 * @{
 *
 */

/*! \file */

#include "config.h"
#include <stdbool.h>
#include <pthread.h>
#include <reent.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <malloc.h>
#include <sys/scheduler/scheduler_local.h>

#include "sched.h"
#include "../unistd/unistd_local.h"
#include "mcu/wdt.h"
#include "mcu/debug.h"
#include "mcu/rtc.h"

#include "trace.h"


static int start_first_thread();
static void root_fault_logged(void * args) MCU_ROOT_EXEC_CODE;

static int check_faults();

int scheduler_check_tid(int id){
    if( id < task_get_total() ){
        if ( task_enabled(id) ){
            return 0;
        }
    }
    return -1;
}


/* \details This function should be called in main() after all hardware
 * and devices have been initialized.
 *
 * The scheduler executes as task 0.  It is run as part of the round robin with all
 * active tasks of the highest priority.
 *
 * When a task calls sleep() or usleep(), it goes inactive.  When a task does
 * a synchronous read() or write() using underlying asynchronous hardware,
 * the task goes inactive until the operation completes.
 *
 * It an active task has a lower priority of another active task, it stays
 * "active" but is not executed until all higher priority tasks go inactive.
 *
 * If a lower priority task is executing and a higher priority task goes "active" (for example,
 * after sleep() or usleep() timer is expired), the lower priority task is pre-empted.
 */
void scheduler(){

    //This interval needs to be long enough to allow for flash writes
    if( (sos_board_config.o_sys_flags & SYS_FLAG_IS_WDT_DISABLED) == 0 ){
        mcu_wdt_init(WDT_MODE_INTERRUPT|WDT_MODE_CLK_SRC_MAIN, SCHED_RR_DURATION * 10 * sos_board_config.task_total + 5);
    }

    if ( scheduler_prepare() ){  //this starts memory protection
        mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_FATAL, (void*)"sprep");
    }

    mcu_debug_log_info(MCU_DEBUG_SCHEDULER, "Start first thread");
    if ( start_first_thread() < 0 ){
        mcu_debug_log_info(MCU_DEBUG_SCHEDULER, "Start first thread failed");
        mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_FATAL, (void*)"strt1t");
    }

    mcu_debug_log_info(MCU_DEBUG_SCHEDULER, "Run scheduler");

    while(1){
        cortexm_svcall(mcu_wdt_root_reset, NULL);
        check_faults(); //check to see if a fault needs to be logged

        //Sleep when nothing else is going on
        if ( task_get_exec_count() == 0 ){
			  //the BSP can set SYS_FLAG_IS_ACTIVE_ON_IDLE and then sleep during this event if a simple sleep won't work
			  mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_SCHEDULER_IDLE, 0);
			  if( (sos_board_config.o_sys_flags & SYS_FLAG_IS_ACTIVE_ON_IDLE) == 0 ){
				  mcu_core_user_sleep(CORE_SLEEP);
			  }
        } else {
            //Otherwise switch to the active task
            sched_yield();
        }
    }
}

void root_fault_logged(void * args){
    m_scheduler_fault.fault.num = 0;
}

int check_faults(){
    if ( m_scheduler_fault.fault.num != 0 ){
        char buffer[256];
        //Trace the fault -- and output on debug
        scheduler_fault_build_trace_string(buffer);
        sos_trace_event_addr_tid(
                    POSIX_TRACE_FATAL,
                    buffer, strlen(buffer),
                    (u32)m_scheduler_fault.fault.pc + 1,
                    m_scheduler_fault.tid);

        usleep(2000);
        mcu_debug_log_error(MCU_DEBUG_SYS, "%s", buffer);

        char hex_buffer[9];
        strcpy(buffer, "ADDR 0x");
        htoa(hex_buffer, (u32)m_scheduler_fault.fault.addr);
        strcat(buffer, hex_buffer);
        sos_trace_event_addr_tid(
                    POSIX_TRACE_MESSAGE,
                    buffer, strlen(buffer),
                    (u32)m_scheduler_fault.fault.pc + 1,
                    m_scheduler_fault.tid);
        mcu_debug_log_error(MCU_DEBUG_SYS, "ADDR 0x%lX %ld", (u32)m_scheduler_fault.fault.pc + 1, m_scheduler_fault.tid);
        usleep(2000);


        strcpy(buffer, "Caller");
        sos_trace_event_addr_tid(
                    POSIX_TRACE_MESSAGE,
                    buffer, strlen(buffer),
                    (u32)m_scheduler_fault.fault.caller,
                    m_scheduler_fault.tid);
        mcu_debug_log_error(MCU_DEBUG_SYS, "Caller 0x%lX %ld", (u32)m_scheduler_fault.fault.caller, m_scheduler_fault.tid);
        usleep(2000);


        strcpy(buffer, "ISR PC");
        sos_trace_event_addr_tid(
                    POSIX_TRACE_MESSAGE,
                    buffer, strlen(buffer),
                    (u32)m_scheduler_fault.fault.handler_pc + 1,
                    m_scheduler_fault.tid);
        mcu_debug_log_error(MCU_DEBUG_SYS, "ISR PC 0x%lX %ld", (u32)m_scheduler_fault.fault.handler_pc+1, m_scheduler_fault.tid);
        usleep(2000);


        strcpy(buffer, "ISR Caller");
        sos_trace_event_addr_tid(
                    POSIX_TRACE_MESSAGE,
                    buffer, strlen(buffer),
                    (u32)m_scheduler_fault.fault.handler_caller,
                    m_scheduler_fault.tid);
        mcu_debug_log_error(MCU_DEBUG_SYS, "ISR Caller 0x%lX %ld", (u32)m_scheduler_fault.fault.handler_caller, m_scheduler_fault.tid);
        usleep(2000);

        cortexm_svcall(root_fault_logged, NULL);

    }

    return 0;
}

//Called when the task stops or drops in priority (e.g., releases a mutex)
void scheduler_root_update_on_stopped(){
    int i;
    s8 next_priority;

    //Issue #130
    cortexm_disable_interrupts();
    next_priority = SCHED_LOWEST_PRIORITY;
    for(i=1; i < task_get_total(); i++){
        //Find the highest priority of all active tasks
        if( task_enabled_active_not_stopped(i) &&
                (task_get_priority(i) > next_priority) ){
            next_priority = task_get_priority(i);
        }
    }
    task_root_set_current_priority(next_priority);
    cortexm_enable_interrupts();

    //this will cause an interrupt to execute but at a lower IRQ priority
    task_root_switch_context();
}


void scheduler_root_update_on_sleep(){
    scheduler_root_deassert_active( task_get_current() );
    scheduler_root_update_on_stopped();
}

//called when a task wakes up
void scheduler_root_update_on_wake(int id, int new_priority){
    //Issue #130
    if( new_priority < task_get_current_priority() ){
        return; //no action needed if the waking task is of lower priority than the currently executing priority
    }

    if( (id > 0) && task_stopped_asserted(id) ){
        return; //this task is stopped on a signal and shouldn't be considered
    }

    //elevate the priority (changes only if new_priority is higher than current
    task_root_elevate_current_priority(new_priority);

    //execute the context switcher but at a lower priority
    task_root_switch_context();
}


//this is called from user space?
int scheduler_get_highest_priority_blocked(void * block_object){
    int priority;
    int i;
    int new_thread;
    int current_task = task_get_current();

    //check to see if another task is waiting for the mutex
    priority = SCHED_LOWEST_PRIORITY - 1;
    new_thread = -1;

    //Issue #139 - blocked mutexes should be awarded in a round robin fashion started with the current task
    if( current_task == 0 ){ current_task++; }
    i = current_task+1;
	 if( i == task_get_total() ){
		 i = 1;
	 }
    do {
        if ( task_enabled(i) ){
            if ( (sos_sched_table[i].block_object == block_object) && ( !task_active_asserted(i) ) ){
                //it's waiting for the block -- give the block to the highest priority and waiting longest
                if( !task_stopped_asserted(i) && (sos_sched_table[i].attr.schedparam.sched_priority > priority) ){
                    //! \todo Find the task that has been waiting the longest time
                    new_thread = i;
                    priority = sos_sched_table[i].attr.schedparam.sched_priority;
                }
            }
        }
        //Issue #139
        i++;
        if( i == task_get_total() ){
            i = 1;
        }
    } while( i != current_task );

    return new_thread;
}
//This is only called from SVcall so it is always synchronous -- no re-entrancy issues with it
int scheduler_root_unblock_all(void * block_object, int unblock_type){
    int i;
    int priority;
    priority = SCHED_LOWEST_PRIORITY - 1;
    for(i=1; i < task_get_total(); i++){
        if ( task_enabled(i) ){
            if ( (sos_sched_table[i].block_object == block_object) && ( !task_active_asserted(i) ) ){
                //it's waiting for the semaphore -- give the semaphore to the highest priority and waiting longest
                scheduler_root_assert_active(i, unblock_type);
                if( !task_stopped_asserted(i) && (sos_sched_table[i].attr.schedparam.sched_priority > priority)  ){
                    priority = sos_sched_table[i].attr.schedparam.sched_priority;
                }
            }
        }
    }
    return priority;
}

int start_first_thread(){
    void * (*init)(void*);
    pthread_attr_t attr;
    int err;

    init = sos_sched_table[0].init;

    attr.stacksize = sos_board_config.start_stack_size;
    attr.stackaddr = malloc(attr.stacksize);
    if ( attr.stackaddr == NULL ){
        mcu_debug_log_error(MCU_DEBUG_SCHEDULER, "No memory\n");
        errno = ENOMEM;
        return -1;
    }
    PTHREAD_ATTR_SET_IS_INITIALIZED((&attr), 1);
    PTHREAD_ATTR_SET_CONTENTION_SCOPE((&attr), PTHREAD_SCOPE_SYSTEM);
    PTHREAD_ATTR_SET_GUARDSIZE((&attr), SCHED_DEFAULT_STACKGUARD_SIZE);
    PTHREAD_ATTR_SET_INHERIT_SCHED((&attr), PTHREAD_EXPLICIT_SCHED);
    PTHREAD_ATTR_SET_DETACH_STATE((&attr), PTHREAD_CREATE_DETACHED);
    PTHREAD_ATTR_SET_SCHED_POLICY((&attr), SCHED_RR);
    attr.schedparam.sched_priority = 21; //not the default priority

    err = scheduler_create_thread(init,
                                  sos_board_config.start_args,
                                  attr.stackaddr,
                                  attr.stacksize,
                                  &attr);

    if ( !err ){
        mcu_debug_log_error(MCU_DEBUG_SCHEDULER, "Failed to create thread\n");
        return -1;
    }

    return 0;
}




/*! @} */


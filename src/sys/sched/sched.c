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
 * \ingroup POSIX
 *
 */

#include "config.h"

#include <sched.h>
#include <errno.h>
#include <sys/scheduler/scheduler_local.h>
#include "mcu/mcu.h"


typedef struct {
    int tid;
    int policy;
    const struct sched_param * param;
} root_set_scheduling_param_t;
static void root_set_scheduling_param(void * args) MCU_ROOT_EXEC_CODE;
static void root_yield(void * args) MCU_ROOT_EXEC_CODE;

/*! \file */
static int get_pid_task(pid_t pid){
    int i;
    for(i=0; i < task_get_total(); i++){
        if ( (task_get_pid(i) == pid) && !task_thread_asserted(i) ){
            return i;
        }
    }
    return -1;
}

/*! \details This function gets the maximum priority for \a policy.
 * \return Zero on success or -1 with errno (see \ref ERRNO) set to:
 * - EINVAL:  \a policy is not SCHED_RR, SCHED_FIFO, or SCHED_OTHER
 *
 */
int sched_get_priority_max(int policy){
    switch(policy){
    case SCHED_RR:
    case SCHED_FIFO:
        return SCHED_HIGHEST_PRIORITY;
    case SCHED_OTHER:
        return 0;
    default:
        errno = EINVAL;
        return -1;
    }
}

/*! \details This function gets the minimum priority for \a policy.
 * \return Zero on success or -1 with errno (see \ref ERRNO) set to:
 * - EINVAL:  \a policy is not SCHED_RR, SCHED_FIFO, or SCHED_OTHER
 *
 */
int sched_get_priority_min(int policy){
    switch(policy){
    case SCHED_RR:
    case SCHED_FIFO:
        return 1;
    case SCHED_OTHER:
        return 0;
    default:
        errno = EINVAL;
        return -1;
    }
}

/*! \details This function gets the scheduling parameter (priority) for \a pid.
 * \return Zero on success or -1 with errno (see \ref ERRNO) set to:
 * - EINVAL:  \a policy is not SCHED_RR, SCHED_FIFO, or SCHED_OTHER
 * - ESRCH:  \a pid is not a valid process
 *
 */
int sched_getparam(pid_t pid, struct sched_param * param){
    int tid;
    //get task for pid thread 0
    tid = get_pid_task(pid);
    if ( tid < 0 ){
        errno = ESRCH;
        return -1;
    }

    //! \todo check the permissions for EPERM

    //copy from sched_table to param
    memcpy(param, (void*)&sos_sched_table[tid].attr.schedparam, sizeof(struct sched_param));
    return 0;
}

/*! \details This function gets the scheduling policy.
 * \return The scheduling policy on success or -1 with errno (see \ref ERRNO) set to:
 * - ESRCH:  \a pid is not a valid process
 *
 */
int sched_getscheduler(pid_t pid){
    int tid;
    //get task for pid thread 0
    tid = get_pid_task(pid);
    if ( tid < 0 ){
        errno = ESRCH;
        return -1;
    }

    //return the scheduling policy
    return PTHREAD_ATTR_GET_SCHED_POLICY( (&(sos_sched_table[tid].attr)) );
}

/*! \details This function gets the round robin interval for \a pid.
 * \return Zero on success or -1 with errno (see \ref ERRNO) set to:
 * - EINVAL:  \a ts is NULL
 * - ESRCH:  \a pid is not a valid process
 *
 */
int sched_rr_get_interval(pid_t pid, struct timespec * ts){

    if ( get_pid_task(pid) < 0 ){
        errno = ESRCH;
        return -1;
    }

    ts->tv_sec = 0;
    ts->tv_nsec = SCHED_RR_DURATION*1000*1000;
    return 0;
}

/*! \details This function sets the process's scheduling paramater (priority).
 * \return Zero on success or -1 with errno (see \ref ERRNO) set to:
 * - ESRCH:  \a pid is not a valid process
 * - EPERM:  the calling process does not have permission to change the scheduling parameters
 *
 */
int sched_setparam(pid_t pid, const struct sched_param * param){
    root_set_scheduling_param_t args;
    //get task for pid thread 0
    args.tid = get_pid_task(pid);
    if ( args.tid < 0 ){
        errno = ESRCH;
        return -1;
    }

    args.policy = PTHREAD_ATTR_GET_SCHED_POLICY( (&(sos_sched_table[args.tid].attr)) );

    if ( (param->sched_priority > sched_get_priority_max(args.policy)) ||
         (param->sched_priority < sched_get_priority_min(args.policy)) ){
        errno = EINVAL;
        return -1;
    }

    args.param = param;
    cortexm_svcall(root_set_scheduling_param, &args);
    return 0;
}

/*! \details This function sets the scheduler policy and parameter (priority) for the process.
 * \return Zero on success or -1 with errno (see \ref ERRNO) set to:
 * - EINVAL:  \a policy is not SCHED_RR, SCHED_FIFO, or SCHED_OTHER
 * - EPERM:  the calling process does not have permission to change the scheduling parameters
 * - ESRCH:  \a pid is not a valid process
 *
 */
int sched_setscheduler(pid_t pid, int policy, const struct sched_param * param){
    root_set_scheduling_param_t args;
    //get task for pid thread 0
    args.tid = get_pid_task(pid);
    if ( args.tid < 0 ){
        errno = ESRCH;
        return -1;
    }

    switch(policy){
    case SCHED_RR:
    case SCHED_FIFO:
    case SCHED_OTHER:
        //! \todo Set the policy
        if ( (param->sched_priority > sched_get_priority_max(policy)) ||
             (param->sched_priority < sched_get_priority_min(policy)) ){
            errno = EINVAL;
            return -1;
        }

        args.policy = policy;
        args.param = param;
        cortexm_svcall(root_set_scheduling_param, &args);
        return 0;
    default:
        errno = EINVAL;
        return -1;
    }

}

/*! \details This function causes the calling thread to yield the processor.  The context
 * is switched to the next active task.  If no tasks are active, the CPU idles.
 * \return Zero
 */
int sched_yield(){
    cortexm_svcall(root_yield, 0);
    return 0;
}


void root_yield(void * args){
    task_assert_yield( task_get_current() );
    task_root_switch_context();
}

void root_set_scheduling_param(void * args){
    root_set_scheduling_param_t * p = (root_set_scheduling_param_t*)args;
    int id;
    id = p->tid;

    if( task_enabled(id) ){

        PTHREAD_ATTR_SET_SCHED_POLICY( (&(sos_sched_table[id].attr)), p->policy);

        memcpy((void*)&sos_sched_table[id].attr.schedparam, p->param, sizeof(struct sched_param));

        //Issue #161 -- need to set the effective priority -- not just the prio ceiling
        task_set_priority(id, sos_sched_table[id].attr.schedparam.sched_priority);

		  //this won't become effective until the next time the task is run because the RR timer is currently active
        if ( p->policy == SCHED_FIFO ){
            task_assert_fifo(id);
        } else {
            task_deassert_fifo(id);
        }

        if( task_enabled_active_not_stopped(id) ){
            scheduler_root_update_on_stopped();
        }
    }

}

/*! @} */



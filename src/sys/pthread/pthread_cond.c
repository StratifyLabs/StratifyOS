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


/*! \addtogroup pthread
 * @{
 *
 *
 */

/*! \file */

#include "config.h"

#include <pthread.h>
#include <errno.h>

#include "../scheduler/scheduler_local.h"

/*! \cond */
#define PSHARED_FLAG 31
#define INIT_FLAG 30
#define PID_MASK 0x3FFFFFFF

static void svcall_cond_signal(void * args) MCU_ROOT_EXEC_CODE;

typedef struct {
	pthread_cond_t *cond;
	pthread_mutex_t *mutex;
	int new_thread;
	struct mcu_timeval interval;
	int result;
} svcall_cond_wait_t;
static void svcall_cond_wait(void  * args) MCU_ROOT_EXEC_CODE;
static void svcall_cond_broadcast(void * args) MCU_ROOT_EXEC_CODE;

/*! \endcond */

/*! \details This function initializes a pthread block condition.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: cond or attr is NULL
 */
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr){
	if ( (cond == NULL) || (attr == NULL) ){
		errno = EINVAL;
		return -1;
	}

	*cond = getpid() | (1<<INIT_FLAG);
	if ( attr->process_shared != 0 ){
		*cond |= (1<<PSHARED_FLAG);
	}
	return 0;
}

/*! \details This function destroys a pthread block condition.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: cond is NULL
 */
int pthread_cond_destroy(pthread_cond_t *cond){
	if ( cond == NULL ){
		errno = EINVAL;
		return -1;
	}

	if ( (*cond & (1<<INIT_FLAG)) == 0 ){
		return EINVAL;
		return -1;
	}

	*cond = 0;
	return 0;
}

void svcall_cond_broadcast(void * args){
	CORTEXM_SVCALL_ENTER();
	int prio;
	prio = scheduler_root_unblock_all(args, SCHEDULER_UNBLOCK_COND);
	scheduler_root_update_on_wake(-1, prio);
}

/*! \details This function wakes all threads that are blocked on \a cond.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: cond is NULL or not initialized
 */
int pthread_cond_broadcast(pthread_cond_t *cond){
	if ( cond == NULL ){
		errno = EINVAL;
		return -1;
	}

	if ( (*cond & (1<<INIT_FLAG)) == 0 ){
		errno = EINVAL;
		return -1;
	}

	//wake all tasks blocking on cond
	cortexm_svcall(svcall_cond_broadcast, cond);
	return 0;
}

void svcall_cond_signal(void * args){
	CORTEXM_SVCALL_ENTER();
	int id = *((int*)args);
	scheduler_root_assert_active(id, SCHEDULER_UNBLOCK_COND);
	scheduler_root_update_on_wake(id, task_get_priority(id));
}

/*! \details This function wakes the highest priority thread
 * that is blocked on \a cond.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: cond is NULL or not initialized
 */
int pthread_cond_signal(pthread_cond_t *cond){
	int new_thread;

	if ( cond == NULL ){
		errno = EINVAL;
		return -1;
	}

	if ( (*cond & (1<<INIT_FLAG)) == 0 ){
		return EINVAL;
		return -1;
	}

	new_thread = scheduler_get_highest_priority_blocked(cond);

	if ( new_thread != -1 ){
		cortexm_svcall(svcall_cond_signal, &new_thread);
	}

	return 0;
}


/*! \details This function causes the calling thread to block
 * on \a cond. When called, \a mutex must be locked by the caller.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL:  cond is NULL or not initialized
 * - EACCES:  cond is from a different process and not shared
 * - EPERM:  the caller does not have a lock on \a mutex
 */
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex){
	int pid;
	svcall_cond_wait_t args;

	if ( cond == NULL ){
		errno = EINVAL;
		return -1;
	}

	if ( (*cond & (1<<INIT_FLAG)) == 0 ){
		return EINVAL;
		return -1;
	}

	pid = *cond & PID_MASK;

	if ( (*cond & (1<<PSHARED_FLAG)) == 0 ){
		if ( pid != getpid() ){ //This is a different process with a not pshared cond
			errno = EACCES;
			return -1;
		}
	}

	args.cond = cond;
	args.mutex = mutex;
	args.interval.tv_sec = SCHEDULER_TIMEVAL_SEC_INVALID;
	args.interval.tv_usec = 0;

	//release the mutex and block on the cond
	args.new_thread = scheduler_get_highest_priority_blocked(mutex);
	cortexm_svcall(svcall_cond_wait, &args);

	if ( args.result == -1 ){
		errno = EPERM;
		return -1;
	}

	return 0;
}

/*! \details This function causes the calling thread to block
 * on \a cond. When called, \a mutex must be locked by the caller.  If \a cond does
 * not wake the process by \a abstime, the thread resumes.
 *
 * Example:
 * \code
 * struct timespec abstime;
 * clock_gettime(CLOCK_REALTIME, &abstime);
 * abstime.tv_sec += 5; //time out after five seconds
 * if ( pthread_cond_timedwait(cond, mutex, &abstime) == -1 ){
 * 	if ( errno == ETIMEDOUT ){
 * 		//Timedout
 * 	} else {
 * 		//Failed
 * 	}
 * }
 * \endcode
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL:  cond is NULL or not initialized
 * - EACCES:  cond is from a different process and not shared
 * - EPERM:  the caller does not have a lock on \a mutex
 * - ETIMEDOUT:  \a abstime passed before \a cond arrived
 *
 */
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime){
	int pid;
	svcall_cond_wait_t args;

	if ( cond == NULL ){
		errno = EINVAL;
		return -1;
	}

	if ( (*cond & (1<<INIT_FLAG)) == 0 ){
		return EINVAL;
		return -1;
	}

	pid = *cond & PID_MASK;

	if ( (*cond & (1<<PSHARED_FLAG)) == 0 ){
		if ( pid != getpid() ){ //This is a different process with a not pshared cond
			errno = EACCES;
			return -1;
		}
	}

	args.cond = cond;
	args.mutex = mutex;
	scheduler_timing_convert_timespec(&args.interval, abstime);


	//release the mutex and block on the cond
	args.new_thread = scheduler_get_highest_priority_blocked(mutex);
	cortexm_svcall(svcall_cond_wait, &args);

	if ( args.result == -1 ){
		errno = EPERM;
		return -1;
	}

	if ( scheduler_unblock_type( task_get_current() ) == SCHEDULER_UNBLOCK_SLEEP ){
		errno = ETIMEDOUT;
		return -1;
	}

	return 0;
}

/*! \cond */
void svcall_cond_wait(void  * args){
	CORTEXM_SVCALL_ENTER();
	svcall_cond_wait_t * argsp = (svcall_cond_wait_t*)args;
	int new_thread = argsp->new_thread;


	if ( argsp->mutex->pthread == task_get_current() ){
		//First unlock the mutex
		//Restore the priority to the task that is unlocking the mutex
		task_set_priority(task_get_current(), sos_sched_table[task_get_current()].attr.schedparam.sched_priority);

		if ( new_thread != -1 ){
			argsp->mutex->pthread = new_thread;
			argsp->mutex->pid = task_get_pid(new_thread);
			argsp->mutex->lock = 1;
			task_set_priority(new_thread, argsp->mutex->prio_ceiling);
			scheduler_root_assert_active(new_thread, SCHEDULER_UNBLOCK_MUTEX);
		} else {
			argsp->mutex->lock = 0;
			argsp->mutex->pthread = -1; //The mutex is up for grabs
		}

		scheduler_timing_root_timedblock(argsp->cond, &argsp->interval);
		argsp->result = 0;
	} else {
		argsp->result = -1;
	}

}
/*! \endcond */

/*! @} */


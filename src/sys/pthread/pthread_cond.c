// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup pthread
 * @{
 *
 *
 */

/*! \file */

#include "config.h"

#include <errno.h>
#include <pthread.h>

#include "../scheduler/scheduler_timing.h"
#include "../scheduler/scheduler_root.h"
#include "pthread_mutex_local.h"

/*! \cond */
#define PSHARED_FLAG 31
#define INIT_FLAG 30
#define PID_MASK 0x3FFFFFFF

static void svcall_cond_signal(void *args) MCU_ROOT_EXEC_CODE;

typedef struct {
  pthread_cond_t *cond;
  pthread_mutex_t *mutex;
  struct mcu_timeval interval;
} svcall_cond_wait_t;
static void svcall_cond_wait(void *args) MCU_ROOT_EXEC_CODE;
static void svcall_cond_broadcast(void *args) MCU_ROOT_EXEC_CODE;

/*! \endcond */

/*! \details This function initializes a pthread block condition.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: cond or attr is NULL
 */
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) {
  if ((cond == NULL) || (attr == NULL)) {
    errno = EINVAL;
    return -1;
  }

  *cond = getpid() | (1 << INIT_FLAG);
  if (attr->process_shared != 0) {
    *cond |= (1 << PSHARED_FLAG); // cppcheck-suppress[shiftTooManyBitsSigned]
  }
  return 0;
}

/*! \details This function destroys a pthread block condition.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: cond is NULL
 */
int pthread_cond_destroy(pthread_cond_t *cond) {
  if (cond == NULL) {
    errno = EINVAL;
    return -1;
  }

  if ((*cond & (1 << INIT_FLAG)) == 0) {
    errno = EINVAL;
    return -1;
  }

  *cond = 0;
  return 0;
}

void svcall_cond_broadcast(void *args) {
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
int pthread_cond_broadcast(pthread_cond_t *cond) {
  if (cond == NULL) {
    errno = EINVAL;
    return -1;
  }

  if ((*cond & (1 << INIT_FLAG)) == 0) {
    errno = EINVAL;
    return -1;
  }

  // wake all tasks blocking on cond
  cortexm_svcall(svcall_cond_broadcast, cond);
  return 0;
}

void svcall_cond_signal(void *args) {
  CORTEXM_SVCALL_ENTER();
  int id = *((int *)args);
  scheduler_root_assert_active(id, SCHEDULER_UNBLOCK_COND);
  scheduler_root_update_on_wake(id, task_get_priority(id));
}

/*! \details This function wakes the highest priority thread
 * that is blocked on \a cond.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: cond is NULL or not initialized
 */
int pthread_cond_signal(pthread_cond_t *cond) {
  int new_thread;

  if (cond == NULL) {
    errno = EINVAL;
    return -1;
  }

  if ((*cond & (1 << INIT_FLAG)) == 0) {
    errno = EINVAL;
    return -1;
  }

  new_thread = scheduler_get_highest_priority_blocked(cond);

  if (new_thread != -1) {
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
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
  return pthread_cond_timedwait(cond, mutex, NULL);
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
int pthread_cond_timedwait(
  pthread_cond_t *cond,
  pthread_mutex_t *mutex,
  const struct timespec *abstime) {
  SOS_DEBUG_ENTER_TIMER_SCOPE(pthread_cond_timedwait);
  scheduler_check_cancellation();
  int pid;
  svcall_cond_wait_t args;

  if (cond == NULL) {
    errno = EINVAL;
    return -1;
  }

  if ((*cond & (1 << INIT_FLAG)) == 0) {
    errno = EINVAL;
    return -1;
  }

  pid = *cond & PID_MASK;

  if ((*cond & (1 << PSHARED_FLAG)) == 0) { // cppcheck-suppress[shiftTooManyBitsSigned]
    if (pid != getpid()) { // This is a different process with a not pshared cond
      errno = EACCES;
      return -1;
    }
  }

  // does caller have a lock on the mutex?
  if (mutex->pthread != task_get_current()) {
    errno = EACCES;
    return -1;
  }

  args.cond = cond;
  args.mutex = mutex;
  if (abstime) {
    scheduler_timing_convert_timespec(&args.interval, abstime);
  } else {
    args.interval.tv_sec = SCHEDULER_TIMEVAL_SEC_INVALID;
    args.interval.tv_usec = 0;
  }

  // release the mutex and block on the cond
  cortexm_svcall(svcall_cond_wait, &args);
  pthread_mutex_lock(mutex);

  if (scheduler_unblock_type(task_get_current()) == SCHEDULER_UNBLOCK_SLEEP) {
    errno = ETIMEDOUT;
    return -1;
  }

#if POSIX_SPECIFIES_NO_RETURNING_EINTR || 1
  //signal will cause a spurious wakeup
  if (scheduler_unblock_type(task_get_current()) == SCHEDULER_UNBLOCK_SIGNAL) {
    errno = EINTR;
    return -1;
  }
#endif

  SOS_DEBUG_EXIT_TIMER_SCOPE(SOS_DEBUG_PTHREAD, pthread_cond_timedwait);
  return 0;
}

/*! \cond */
void svcall_cond_wait(void *args) {
  CORTEXM_SVCALL_ENTER();

  svcall_cond_wait_t *argsp = (svcall_cond_wait_t *)args;
  pthread_mutex_root_unlock_t unlock_args;
  unlock_args.id = task_get_current();
  unlock_args.mutex = argsp->mutex;
  pthread_mutex_root_unlock(&unlock_args);

  scheduler_timing_root_timedblock(argsp->cond, &argsp->interval);
}
/*! \endcond */

/*! @} */

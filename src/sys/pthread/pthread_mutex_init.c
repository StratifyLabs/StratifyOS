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

#include "cortexm/task.h"

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) MCU_WEAK;

/*! \details This function initializes \a mutex with \a attr.
 * \return Zero on success or -1 with \a errno (see \ref errno) set to:
 * - EINVAL: mutex is NULL
 */
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
  if (mutex == NULL) {
    errno = EINVAL;
    return -1;
  }

  mutex->flags = PTHREAD_MUTEX_FLAGS_INITIALIZED;

  if (attr == NULL) {
    mutex->prio_ceiling = PTHREAD_MUTEX_PRIO_CEILING;
    if (task_get_current() == 0) {
      mutex->pid = 0;
    } else {
      mutex->pid = task_get_pid(task_get_current());
    }
    mutex->lock = 0;
    mutex->pthread = -1;
    return 0;
  }

  if (attr->process_shared != 0) {
    // Enter priv mode to modify a shared object
    mutex->flags |= (PTHREAD_MUTEX_FLAGS_PSHARED);
  }

  if (attr->recursive) {
    mutex->flags |= (PTHREAD_MUTEX_FLAGS_RECURSIVE);
  }
  mutex->prio_ceiling = attr->prio_ceiling;
  mutex->pid = task_get_pid(task_get_current());
  mutex->lock = 0;
  mutex->pthread = -1;
  return 0;
}

/*! @} */

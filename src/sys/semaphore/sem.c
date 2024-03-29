// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup semaphore
 * @{
 *
 */

/*! \file */

#include "sos/fs/sysfs.h"
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../scheduler/scheduler_root.h"
#include "../scheduler/scheduler_timing.h"
#include "semaphore.h"

#include "sos/debug.h"

#define SEM_FILE_HDR_SIGNATURE 0x1285ABC8
#define SEM_FILE_HDR_NOT_SIGNATURE (~SEM_FILE_HDR_SIGNATURE)

/*! \cond */
typedef struct {
  u32 signature;
  u32 not_signature;
  sem_t *sem;
} sem_file_hdr_t;

typedef struct {
  sem_t sem;
  void *next;
} sem_list_t;

static void svcall_sem_wait(void *args) MCU_ROOT_EXEC_CODE;
static void svcall_sem_post(void *args) MCU_ROOT_EXEC_CODE;
static void svcall_sem_trywait(void *args) MCU_ROOT_EXEC_CODE;
static void svcall_sem_timedwait(void *args) MCU_ROOT_EXEC_CODE;

static int check_initialized(sem_t *sem);

typedef struct {
  sem_t *sem;
  int id;
  int new_thread;
  int result;
  struct mcu_timeval interval;
} root_sem_args_t;

static sem_list_t *sem_first = 0;

static sem_t *sem_find_named(const char *name) {
  sem_list_t *entry;
  for (entry = sem_first; entry != 0; entry = entry->next) {
    if (entry->sem.is_initialized != 0) {
      if (strncmp(entry->sem.name, name, sizeof(entry->sem.name) - 1) == 0) {
        return &entry->sem;
      }
    }
  }
  return SEM_FAILED;
}

static sem_t *sem_find_free() {
  sem_list_t *entry;
  sem_list_t *new_entry;
  sem_list_t *last_entry;
  last_entry = 0;
  for (entry = sem_first; entry != 0; entry = entry->next) {
    last_entry = entry;
    if (entry->sem.is_initialized == 0) {
      return &entry->sem;
    }
  }

  // no free semaphores
  new_entry = _malloc_r(sos_task_table[0].global_reent, sizeof(sem_list_t));
  if (new_entry == 0) {
    return SEM_FAILED;
  }
  if (last_entry == 0) {
    sem_first = new_entry;
  } else {
    last_entry->next = new_entry;
  }
  new_entry->next = 0;
  return &new_entry->sem;
}
/*! \endcond */

/*! \details Initializes \a sem as an unnamed semaphore with
 * \a pshared and \a value.
 *
 * @param sem A pointer to the semaphore to initialize
 * @param pshared Non-zero to share the semaphore between processes
 * @param value The initial value of the semaphore
 *
 * The semaphore value will be decremented on sem_wait() and incremented
 * on sem_post().
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EINVAL:  sem is NULL
 *
 */
int sem_init(sem_t *sem, int pshared, unsigned int value) {
  if (sem == NULL) {
    errno = EINVAL;
    return -1;
  }
  sem->is_initialized = 1;
  sem->value = value;
  sem->pshared = pshared;
  sem->pid = getpid();
  memset(sem->name, 0, sizeof(sem->name)); // This is an unnamed semaphore
  return 0;
}

/*! \details This function destroys \a sem--an unnamed semaphore.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EINVAL:  sem is NULL
 *
 */
int sem_destroy(sem_t *sem) {
  if (check_initialized(sem) < 0) {
    return -1;
  }

  sem->is_initialized = 0;
  return 0;
}

/*! \details This function gets the value of the semaphore.  If the semaphore is locked,
 * the value is zero.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EINVAL:  sem is NULL
 *
 */
int sem_getvalue(sem_t *sem, int *sval) {
  if (check_initialized(sem) < 0) {
    return -1;
  }

  *sval = sem->value;
  return 0;
}

/*! \details This function opens or creates a named semaphore.
 *
 * \param name The name of the message queue
 * \param oflag The flags to use when opening (O_CREAT, O_EXCL, O_RDWR)
 *
 * When using O_CREAT, the third argument is the mode and the fourth is
 * the initial value of the semaphore:
 * \code
 * sem_t * sem_open(const char * name, int oflag, int mode, int value){
 * \endcode
 *
 * \return Zero on success or SEM_FAILED with errno (see \ref errno) set to:
 * - ENAMETOOLONG:  name length is greater than NAME_MAX
 * - EEXIST:  O_CREAT and O_EXCL are set in \a oflag but the semaphore already exists
 * - ENOENT:  O_CREAT is not set in \a oflag and the semaphore does not exist
 * - ENOMEM:  not enough memory for the semaphore
 *
 */
sem_t *sem_open(const char *name, int oflag, ...) {
  sem_t *new_sem;
  mode_t mode;
  unsigned value;
  int action;
  va_list ap;

  if (strnlen(name, SEM_NAME_MAX) == SEM_NAME_MAX) {
    errno = ENAMETOOLONG;
    return SEM_FAILED;
  }

  // Check to see if the  semaphore exists
  new_sem = sem_find_named(name);

  // Check the flags to determine the appropriate action
  if (oflag & O_CREAT) {
    if (oflag & O_EXCL) {
      if (new_sem == 0) {
        // Create the new semaphore
        action = 0;
      } else {
        errno = EEXIST;
        return SEM_FAILED;
      }
    } else {
      if (new_sem == 0) {
        // Create a new semaphore
        action = 0;
      } else {
        // Read the existing semaphore
        action = 1;
      }
    }
  } else {
    if (new_sem == 0) {
      errno = ENOENT;
      return SEM_FAILED;
    } else {
      // Read the existing semaphore
      action = 1;
    }
  }

  switch (action) {
  case 0:
    // Create the new semaphore
    new_sem = sem_find_free();
    if (new_sem == NULL) {
      // errno is set by malloc
      return SEM_FAILED;
    }

    va_start(ap, oflag);
    mode = va_arg(ap, mode_t);
    value = va_arg(ap, unsigned);
    va_end(ap);

    new_sem->is_initialized = 1;
    new_sem->value = value;
    new_sem->references = 1;
    new_sem->mode = mode;
    new_sem->pshared = 1;
    strncpy(new_sem->name, name, sizeof(new_sem->name) - 1);
    break;

  case 1:
    // use the existing semaphore
    new_sem->references++;
    break;
  }

  return new_sem;
}

/*! \details This function closes a semaphore.  The semaphore
 * must be deleted using sem_unlink() in order to free the system resources
 * associated with the semaphore.
 *
 * \return Zero on success or SEM_FAILED with errno (see \ref errno) set to:
 * - EINVAL:  sem is NULL
 *
 */
int sem_close(sem_t *sem) {
  if (check_initialized(sem) < 0) {
    return -1;
  }

  if (sem->references > 0) {
    sem->references--;
  }

  if (sem->references == 0) {
    if (sem->is_initialized == 2) {
      // Close and delete
      sem->is_initialized = 0;
      return 0;
    }
  }
  return 0;
}

/*! \details Unlocks (increments) the value of the semaphore.
 *
 * \return Zero on success or SEM_FAILED with errno (see \ref errno) set to:
 * - EINVAL:  sem is NULL
 * - EACCES:  process cannot access semaphore
 *
 * If other threads or processes have called sem_wait() or sem_timedwait(),
 * the semaphore will be available and one of the threads will lock the semaphore.
 *
 */
int sem_post(sem_t *sem) {
  int new_thread;

  if (check_initialized(sem) < 0) {
    return -1;
  }

  // unlock the semaphore -- increment the semaphore
  sem->value++;

  // see if any tasks are blocked on this semaphore
  new_thread = scheduler_get_highest_priority_blocked(sem);

  if (new_thread != -1) {
    cortexm_svcall(svcall_sem_post, &new_thread);
  }

  return 0;
}

/*! \details Locks (decrements) the semaphore. If the semaphore cannot
 * be locked (it is already zero), this function will block until the
 * value of \a CLOCK_REALTIME exceeds the value of \a abs_timeout.
 *
 * @param sem A pointer to the semaphore
 * @param abs_timeout Absolute timeout value
 * @return Zero on success or SEM_FAILED with errno (see \ref errno) set to:
 * - EINVAL:  sem is NULL
 * - EACCES:  process cannot access semaphore
 * - ETIMEDOUT:  timeout expired without locking the semaphore
 *
 *
 *
 */
int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout) {
  // timed wait
  scheduler_check_cancellation();
  root_sem_args_t args;

  if (check_initialized(sem) < 0) {
    return -1;
  }

  args.sem = sem;
  scheduler_timing_convert_timespec(&args.interval, abs_timeout);

  cortexm_svcall(svcall_sem_timedwait, &args);

  if (args.result < 0) {
    if (scheduler_unblock_type(task_get_current()) == SCHEDULER_UNBLOCK_SLEEP) {
      // The timeout expired
      errno = ETIMEDOUT;
      return -1;
    } else {
      // waited on semaphore -- need to see if it is still available
      return sem_trywait(sem);
    }
  }

  return 0;
}

/*! \details This function locks (decrements) the semaphore if it can be locked
 * immediately.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EINVAL:  sem is NULL
 * - EACCES:  process cannot access semaphore
 * - EAGAIN:  \a sem could not be immediately locked.
 *
 */
int sem_trywait(sem_t *sem) {
  root_sem_args_t args;

  if (check_initialized(sem) < 0) {
    return -1;
  }

  args.sem = sem;
  args.result = 0;

  cortexm_svcall(svcall_sem_trywait, &args);

  if (args.result < 0) {
    errno = EAGAIN;
  }

  return args.result;
}

/*! \details This function unlinks a named semaphore.  It
 * also releases any system resources associated with the semaphore.
 *
 * \return Zero on success or SEM_FAILED with errno (see \ref errno) set to:
 * - ENOENT:  the semaphore with \a name could not be found
 * - ENAMETOOLONG:  the length of \a name exceeds \a NAME_MAX
 *
 */
int sem_unlink(const char *name) {
  sem_t *sem;

  if (strnlen(name, SEM_NAME_MAX) == SEM_NAME_MAX) {
    errno = ENAMETOOLONG;
    return -1;
  }

  sem = sem_find_named(name);
  if (sem == SEM_FAILED) {
    errno = ENOENT;
    return -1;
  }

  if (check_initialized(sem) < 0) {
    return -1;
  }

  if (sem->references == 0) {
    // Close and delete
    sem->is_initialized = 0;
    return 0;
  } else {
    // Close but don't delete until all references are gone
    sem->is_initialized = 2;
    return 0;
  }

  return -1;
}

/*! \details This function locks (decrements) the semaphore.  If
 * the semaphore is already zero (and therefore cannot be locked),
 * the calling thread blocks until the semaphore becomes available
 * by another thread calling sem_post().
 *
 * \return Zero on success or SEM_FAILED with errno (see \ref errno) set to:
 * - EINVAL:  \a sem is NULL
 * - EACCES:  \a sem is not pshared and was created in another process
 *
 */
int sem_wait(sem_t *sem) {
  scheduler_check_cancellation();
  root_sem_args_t args;
  if (check_initialized(sem) < 0) {
    return -1;
  }

  args.sem = sem;
  args.result = 0;

  do {
    cortexm_svcall(svcall_sem_wait, &args);
  } while (args.result < 0);

  return 0;
}

/*! \cond */

void svcall_sem_post(void *args) {
  CORTEXM_SVCALL_ENTER();
  int id = *((int *)args);
  sos_sched_table[id].block_object = NULL;
  scheduler_root_assert_active(id, SCHEDULER_UNBLOCK_SEMAPHORE);
  scheduler_root_update_on_wake(id, task_get_priority(id));
}

void svcall_sem_timedwait(void *args) {
  CORTEXM_SVCALL_ENTER();
  root_sem_args_t *p = (root_sem_args_t *)args;

  if (p->sem->value <= 0) {
    scheduler_timing_root_timedblock(p->sem, &p->interval);
    p->result = -1;
  } else {
    p->result = 0;
    p->sem->value--;
  }
}

void svcall_sem_trywait(void *args) {
  CORTEXM_SVCALL_ENTER();
  root_sem_args_t *p = (root_sem_args_t *)args;

  if (p->sem->value > 0) {
    p->sem->value--;
    p->result = 0;
  } else {
    p->result = -1;
  }
}

void svcall_sem_wait(void *args) {
  CORTEXM_SVCALL_ENTER();
  root_sem_args_t *p = args;

  sos_sched_table[task_get_current()].block_object = p->sem;

  if (p->sem->value <= 0) {
    // task must be blocked until the semaphore is available
    scheduler_root_update_on_sleep();
    p->result = -1; // didn't get the semaphore
  } else {
    // got the semaphore
    p->sem->value--;
    p->result = 0;
  }
}

int check_initialized(sem_t *sem) {
  if (sem == NULL) {
    errno = EINVAL;
    return -1;
  }

  if (sem->is_initialized == 0) {
    errno = EINVAL;
    return -1;
  }

  if ((sem->pshared == 0) && (task_get_pid(task_get_current())) != sem->pid) {
    errno = EACCES;
    return -1;
  }

  return 0;
}
/*! \endcond */

/*! @} */

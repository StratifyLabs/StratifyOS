// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "config.h"

#include <errno.h>
#include <pthread.h>

#include "../scheduler/scheduler_root.h"

static void svcall_join_thread(void *args) MCU_ROOT_EXEC_CODE;

/*! \details This function creates a new thread.
 * \return Zero on success or -1 with \a errno (see \ref errno) set to:
 * - ENOMEM: error allocating memory for the thread
 * - EAGAIN: insufficient system resources to create a new thread
 *
 *
 */
int pthread_create(
  pthread_t *thread /*! If not null, the thread id is written here */,
  const pthread_attr_t
    *attr /*! Sets the thread attributes (defaults are used if this is NULL) */,
  void *(*start_routine)(void *)/*! A pointer to the start routine */,
  void *arg /*! A pointer to the start routine's single argument */
) {

  int id;
  pthread_attr_t attrs;
  void *stack_addr;

  if (attr == NULL) {
    sos_debug_log_info(SOS_DEBUG_PTHREAD, "Init new attrs");
    if (pthread_attr_init(&attrs) < 0) {
      // errno is set by pthread_attr_init()
      return -1;
    }
  } else {
    attrs = *attr;
  }

  u32 mem_size = attrs.stacksize + sizeof(struct _reent) + CONFIG_TASK_DEFAULT_STACKGUARD_SIZE;
  stack_addr = malloc(mem_size);
  if (stack_addr == NULL) {
    sos_debug_log_error(SOS_DEBUG_PTHREAD, "Failed to alloc stack memory:%d", mem_size);
    errno = ENOMEM;
    return -1;
  }

  attrs.stackaddr = stack_addr;
  memset(stack_addr, 0, mem_size); // nullify memory space

  const int inherit_sched = PTHREAD_ATTR_GET_INHERIT_SCHED((&attrs));
  if( inherit_sched == PTHREAD_INHERIT_SCHED ){
    //set the policy and priority based on the calling thread
    int policy;
    struct sched_param param;
    pthread_getschedparam(task_get_current(), &policy, &param);
    pthread_attr_setschedpolicy(&attrs, policy);
    pthread_attr_setschedparam(&attrs, &param);
  }

  id = scheduler_create_thread(start_routine, arg, stack_addr, attrs.stacksize, &attrs);

  if (id) {
    if (thread) {
      *thread = id;
    }
    return 0;
  } else {
    free(stack_addr);
    if (attr == NULL) {
      sos_debug_log_info(SOS_DEBUG_PTHREAD, "Destroy attrs");
      pthread_attr_destroy(&attrs);
    }
    errno = EAGAIN;
    return -1;
  }
}

/*! \details This function blocks the calling thread until \a thread terminates.
 * \return Zero on success or -1 with \a errno (see \ref errno) set to:
 * - ESRCH: \a thread does not exist
 * - EDEADLK: a deadlock has been detected or \a thread refers to the calling thread
 * - EINVAL: \a thread does not refer to a joinable thread.
 */
int pthread_join(pthread_t thread, void **value_ptr) {

  if ((thread < task_get_total()) && (thread >= 0)) {
    if (task_enabled(thread)) {
      // now see if the thread is joinable
      if (
        PTHREAD_ATTR_GET_DETACH_STATE((&(sos_sched_table[thread].attr)))
        != PTHREAD_CREATE_JOINABLE) {
        errno = EINVAL;
        return -1;
      }

      // See if the thread is joined to this thread
      if (
        (sos_sched_table[thread].block_object
         == (void *)&sos_sched_table[task_get_current()])
        || ((int)thread == task_get_current())) {
        errno = EDEADLK;
        return -1;
      }

      do {
        cortexm_svcall(svcall_join_thread, &thread);
        if (thread < 0) {
          errno = ESRCH;
          return -1;
        }
      } while (scheduler_unblock_type(task_get_current())
               != SCHEDULER_UNBLOCK_PTHREAD_JOINED_THREAD_COMPLETE);

      if (value_ptr != NULL) {
        // When the thread terminates, it puts the exit value in this threads scheduler
        // table entry
        *value_ptr = (void *)(sos_sched_table[task_get_current()].exit_status);
      }

      return 0;
    }
  }
  errno = ESRCH;
  return -1;
}

void pthread_exit(void *value_ptr) { scheduler_thread_cleanup(value_ptr); }

/*! \cond */
void svcall_join_thread(void *args) {
  CORTEXM_SVCALL_ENTER();
  int *p = (int *)args;
  int id = *p;

  if (task_enabled(id)) {
    sos_sched_table[task_get_current()].block_object =
      (void *)&sos_sched_table[id]; // block on the thread to be joined
    // If the thread is waiting to be joined, it needs to be activated
    if (sos_sched_table[id].block_object == (void *)&sos_sched_table[id].block_object) {
      scheduler_root_assert_active(id, SCHEDULER_UNBLOCK_PTHREAD_JOINED);
    }
    scheduler_root_update_on_sleep();
  } else {
    *p = -1;
  }
}
/*! \endcond */

/*! @} */

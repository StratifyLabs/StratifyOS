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
#include <stdbool.h>

/*! \cond */
static int check_initialized(const pthread_condattr_t *attr) {
  if (attr == NULL) {
    errno = EINVAL;
    return -1;
  }
  if (attr->is_initialized == false) {
    errno = EINVAL;
    return -1;
  }
  return 0;
}
/*! \endcond */

/*! \details This function initializes \a attr with the default values.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL:  attr is NULL
 */
int pthread_condattr_init(pthread_condattr_t *attr) {
  attr->is_initialized = true;
  attr->process_shared = false;
  return 0;
}

/*! \details This function destroys \a attr.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL:  \a attr is not an initialized condition attribute
 */
int pthread_condattr_destroy(pthread_condattr_t *attr) {
  if (check_initialized(attr)) {
    return -1;
  }
  attr->is_initialized = false;
  return 0;
}

/*! \details This function gets the pshared value for \a attr and stores
 * it in \a pshared.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL:  \a attr is not an initialized condition attribute
 */
int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *pshared) {
  if (check_initialized(attr)) {
    return -1;
  }

  *pshared = attr->process_shared;
  return 0;
}

/*! \details This function sets the pshared value in \a attr to \a pshared.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL:  \a attr is not an initialized condition attribute
 * - EINVAL: \a pshared is not PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE
 */
int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared) {
  if (check_initialized(attr)) {
    return -1;
  }
  if( (pshared == PTHREAD_PROCESS_SHARED) || (pshared == PTHREAD_PROCESS_PRIVATE) ){
    attr->process_shared = pshared;
    return 0;
  }

  errno = EINVAL;
  return -1;
}

/*! \details This function gets the clock associated
 * with pthread_cond_timedwait() operations.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL:  \a attr is not an initialized condition attribute
 */
int pthread_condattr_getclock(const pthread_condattr_t *attr, clockid_t *clock_id) {
  if (check_initialized(attr)) {
    return -1;
  }
  *clock_id = CLOCK_REALTIME;
  return 0;
}

/*! \details pthread_cond_timedwait() operations
 * always use CLOCK_REALTIME.  This value
 * cannot be changed.
 *
 * \return -1 with errno set to:
 * - ENOTSUP:  this function is not supported
 */
int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock_id) {
  MCU_UNUSED_ARGUMENT(attr);
  MCU_UNUSED_ARGUMENT(clock_id);
  errno = ENOTSUP;
  return -1;
}

/*! @} */

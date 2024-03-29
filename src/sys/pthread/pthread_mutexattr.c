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

/*! \cond */
extern int pthread_mutexattr_check_initialized(const pthread_mutexattr_t *attr);
/*! \endcond */

/*! \details This function gets the priority ceiling from \a attr and stores it
 * in \a prioceiling.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EINVAL: \a attr does not refer to an initialized mutex attribute object
 */
int pthread_mutexattr_getprioceiling(
  const pthread_mutexattr_t *attr /*! Mutex Thread Attribute Object */,
  int *prioceiling /*! Destination pointer (cannot be NULL) */) {
  if (pthread_mutexattr_check_initialized(attr) < 0) {
    return -1;
  }

  *prioceiling = attr->prio_ceiling;
  return 0;
}

/*! \details This function sets the priority ceiling in \a attr to \a prio_ceiling.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EINVAL: \a attr does not refer to an initialized mutex attribute object
 */
int pthread_mutexattr_setprioceiling(
  pthread_mutexattr_t *attr /*! Mutex Thread Attribute Object */,
  int prio_ceiling /*! The priority ceiling to set */
) {
  if (pthread_mutexattr_check_initialized(attr) < 0) {
    return -1;
  }

  if( prio_ceiling < CONFIG_SCHED_LOWEST_PRIORITY ){
    errno = EINVAL;
    return -1;
  }

  if( prio_ceiling > CONFIG_SCHED_HIGHEST_PRIORITY ){
    errno = EINVAL;
    return -1;
  }

  attr->prio_ceiling = prio_ceiling;
  return 0;
}

/*! \details This function gets the protocol from \a attr and stores it
 * in \a protocol.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EINVAL: \a attr does not refer to an initialized mutex attribute object
 */
int pthread_mutexattr_getprotocol(
  const pthread_mutexattr_t *attr /*! Mutex Thread Attribute Object */,
  int *protocol /*! Pointer to the destination protocol */) {
  if (pthread_mutexattr_check_initialized(attr) < 0) {
    return -1;
  }

  *protocol = attr->protocol;
  return 0;
}

/*! \details This function sets protocol in \a attr to \a protocol.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EINVAL: \a attr does not refer to an initialized mutex attribute object
 */
int pthread_mutexattr_setprotocol(
  pthread_mutexattr_t *attr /*! Mutex Thread Attribute Object */,
  int protocol /*! The new protocol */) {
  if (pthread_mutexattr_check_initialized(attr) < 0) {
    return -1;
  }

  switch (protocol) {
  case PTHREAD_PRIO_NONE:
  case PTHREAD_PRIO_INHERIT:
  case PTHREAD_PRIO_PROTECT:
    break;
  default:
    errno = EINVAL;
    return -1;
  }

  attr->protocol = protocol;
  return 0;
}

/*! \details This function gets the process shared value from \a attr and stores it
 * in \a pshared.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EINVAL: \a attr does not refer to an initialized mutex attribute object
 */
int pthread_mutexattr_getpshared(
  const pthread_mutexattr_t *attr /*! Mutex Thread Attribute Object */,
  int *pshared /*! The pointer to the destination pshared value */) {
  if (pthread_mutexattr_check_initialized(attr) < 0) {
    return -1;
  }

  *pshared = attr->process_shared;
  return 0;
}

/*! \details This function sets the process shared value in \a attr to \a pshared.
 * A non-zero \a pshared means the mutex is shared.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EINVAL: \a attr does not refer to an initialized mutex attribute object
 * - EINVAL: \a pshared is not PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE
 */
int pthread_mutexattr_setpshared(
  pthread_mutexattr_t *attr /*! Mutex Thread Attribute Object */,
  int pshared /*! The new pshared value (non-zero to share with other processes) */) {
  if (pthread_mutexattr_check_initialized(attr) < 0) {
    return -1;
  }

  if( (pshared == PTHREAD_PROCESS_SHARED) || (pshared == PTHREAD_PROCESS_PRIVATE) ){
    attr->process_shared = pshared;
    return 0;
  }

  errno = EINVAL;
  return -1;
}

/*! \details This function gets the type from \a attr and stores it
 * in \a type.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EINVAL: \a attr does not refer to an initialized mutex attribute object
 */
int pthread_mutexattr_gettype(
  const pthread_mutexattr_t *attr /*! Mutex Thread Attribute Object */,
  int *type /*! The destination pointer for the type */) {
  if (pthread_mutexattr_check_initialized(attr) < 0) {
    return -1;
  }

  if (attr->recursive != 0) {
    *type = PTHREAD_MUTEX_RECURSIVE;
  } else {
    *type = PTHREAD_MUTEX_NORMAL;
  }
  return 0;
}

/*! \details This function sets the type in \a attr to \a type.
 * The \a type value should be on of:
 * - PTHREAD_MUTEX_NORMAL
 * - PTHREAD_MUTEX_RECURSIVE
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EINVAL: \a attr does not refer to an initialized mutex attribute object
 */
int pthread_mutexattr_settype(
  pthread_mutexattr_t *attr /*! Mutex Thread Attribute Object */,
  int type /*! The new type either PTHREAD_MUTEX_NORMAL or PTHREAD_MUTEX_RECURSIVE*/) {
  if (pthread_mutexattr_check_initialized(attr) < 0) {
    return -1;
  }

  switch (type) {
  case PTHREAD_MUTEX_NORMAL:
    attr->recursive = 0;
    break;
  case PTHREAD_MUTEX_RECURSIVE:
    attr->recursive = 1;
    break;
  case PTHREAD_MUTEX_ERRORCHECK:
  default:
    errno = EINVAL;
    return -1;
  }

  return 0;
}

/*! @} */

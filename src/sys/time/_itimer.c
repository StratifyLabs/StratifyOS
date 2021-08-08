// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup time
 * @{
 */

/*! \file */

#include <errno.h>
#include <sys/time.h>
#include <time.h>

// struct itimerval itimers[SYSCALLS_NUM_ITIMERS];

int _getitimer(int which, struct itimerval *value) {
  // Update the current value (time of day minus the start time)
  errno = ENOTSUP;
  return -1;
}

int _setitimer(int which, const struct itimerval *value, struct itimerval *ovalue) {
  errno = ENOTSUP;
  return -1;
}

/*! @} */

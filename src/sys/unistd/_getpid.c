// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include <sys/types.h>

#include "config.h"
#include "cortexm/task.h"

/*! \details This function returns the process ID of the calling process.
 * \return The process ID of the caller.
 */
pid_t getpid();

/*! \cond */
pid_t _getpid() { return (pid_t)task_get_pid(task_get_current()); }
/*! \endcond */

/*! @} */

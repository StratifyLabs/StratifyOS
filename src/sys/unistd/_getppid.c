// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include <sys/types.h>
#include "mcu/mcu.h"
#include "cortexm/task.h"

/*! \details This function returns the process ID of the parent process.
 * \return The process ID of the caller's parent process.
 */
pid_t getppid(){
	return (pid_t)task_get_pid( task_get_parent( task_get_current() ) );
}

/*! @} */


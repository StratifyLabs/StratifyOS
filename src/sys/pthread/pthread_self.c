// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup pthread
 * @{
 */

/*! \file */



#include <pthread.h>
#include "mcu/mcu.h"
#include "cortexm/task.h"

/*! \details This function returns the thread ID of the calling process.
 * \return The thread ID of the caller.
 */
pthread_t pthread_self(){
	return (pthread_t)task_get_current();
}

/*! @} */


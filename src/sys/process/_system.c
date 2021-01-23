// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup SYSCALLS_PROCESS
 * @{
 */

/*! \file */

#include "config.h"

#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

#include "process_start.h"
#include "../scheduler/scheduler_local.h"


/*! \details This function creates a new process.
 * The calling process blocks until the new process completes.
 *
 * \return The return value of the child process.
 */
int _system(const char * s){
	int err;
	int status;

	//Check for a NULL argument
	if ( s == NULL ){
		return 1; //command processor is available
	}

	//Start the new process
	err = process_start(s, NULL);
	if ( err < 0 ){
		//Process failed to start
		status = -1;
	} else {
		//Wait for the process to complete before returning
		if ( waitpid(err, &status, 0) < 0 ){
			status = -1;
		}
	}

	//free the argument memory

	return status;
}


/*! @} */





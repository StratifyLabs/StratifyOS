/* Copyright 2011-2018 Tyler Gilbert; 
 * This file is part of Stratify OS.
 *
 * Stratify OS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stratify OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * 
 */

/*! \addtogroup SYSCALLS_PROCESS
 * @{
 */

/*! \file */

#include "config.h"

#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

#include "process_start.h"
#include "mcu/mcu.h"
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
	err = process_start(s, NULL, 0);
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





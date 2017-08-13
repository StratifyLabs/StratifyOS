/* Copyright 2011-2016 Tyler Gilbert; 
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

/*! \addtogroup SIGNAL
 * @{
 */

/*! \file */

#include <signal.h>
#include <errno.h>

#include "sig_local.h"
#include "../sched/sched_local.h"

/*! \details This function sends the signal \a signo to the process \a pid.
 *
 * \return Zero or -1 with errno (see \ref ERRNO) set to:
 * - EINVAL: \a signo is not a valid signal number
 * - ESRCH: \a pid is not a valid process id
 *
 */
int kill(pid_t pid, int signo);

int _kill(pid_t pid, int signo){
	int tid;

	for(tid = 1; tid < task_get_total(); tid++){
		if ( pid == task_get_pid(tid) ){
			break;
		}
	}

	//! \todo Add permission error checking
	return signal_send(tid, signo, SI_USER, 0);
}


/*! @} */

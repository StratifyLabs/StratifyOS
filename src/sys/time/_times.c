/* Copyright 2011-2019 Tyler Gilbert;
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

/*! \addtogroup time
 * @{
 */

/*! \file */

#include <stdlib.h>
#include <sys/times.h>
#include <errno.h>

#include "mcu/arch.h"
#include "cortexm/task.h"


/*! \details This function populates \a buffer with the:
 * - process user time
 * - system time on behalf of the process
 * - children process user time
 * - children system time on behalf of the process
 *
 * \return Zero or -1 with errno (see \ref errno) set to:
 * - EINVAL: buffer is NULL
 *
 */
clock_t times(struct tms *buffer);

/*! \cond */
clock_t _times(struct tms *buffer) {
	int i;
	clock_t value;
	int pid;

	value = 0;
	pid = task_get_pid( task_get_current() );
	for(i=0; i < task_get_total(); i++){
		if ( task_get_pid(i) == pid ){
			value += task_gettime(i);
		}
	}

	buffer->tms_utime = value; //user time
	buffer->tms_stime = 0; //system time on behalf of process

	value = 0;
	for(i=0; i < task_get_total(); i++){
		if ( task_get_pid( task_get_parent(i) ) == pid ){
			value += task_gettime(i);
		}
	}

	buffer->tms_cutime = value; //children user time
	buffer->tms_cstime = 0; //children system time on behalf of process
	return buffer->tms_cutime + buffer->tms_utime;
}
/*! \endcond */

/*! @} */

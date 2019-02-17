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

/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include "config.h"
#include <sys/types.h>
#include "mcu/mcu.h"
#include "cortexm/task.h"

/*! \details This function returns the process ID of the calling process.
 * \return The process ID of the caller.
 */
pid_t getpid();

/*! \cond */
pid_t _getpid(){
	return (pid_t)task_get_pid( task_get_current() );
}
/*! \endcond */

/*! @} */


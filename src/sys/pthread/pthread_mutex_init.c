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

/*! \addtogroup pthread
 * @{
 *
 *
 */

/*! \file */


#include "mcu/arch.h"
#include "config.h"
#include <pthread.h>
#include <errno.h>
#include "cortexm/task.h"

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) MCU_WEAK;

/*! \details This function initializes \a mutex with \a attr.
 * \return Zero on success or -1 with \a errno (see \ref errno) set to:
 * - EINVAL: mutex is NULL
 */
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr){
	if ( mutex == NULL ){
		errno = EINVAL;
		return -1;
	}

	mutex->flags = PTHREAD_MUTEX_FLAGS_INITIALIZED;

	if ( attr == NULL ){
		mutex->prio_ceiling = PTHREAD_MUTEX_PRIO_CEILING;
		if ( task_get_current() == 0 ){
			mutex->pid = 0;
		} else {
			mutex->pid = task_get_pid( task_get_current() );
		}
		mutex->lock = 0;
		mutex->pthread = -1;
		return 0;
	}

	if ( attr->process_shared != 0 ){
		//Enter priv mode to modify a shared object
		mutex->flags |= (PTHREAD_MUTEX_FLAGS_PSHARED);
	}

	if ( attr->recursive ){
		mutex->flags |= (PTHREAD_MUTEX_FLAGS_RECURSIVE);
	}
	mutex->prio_ceiling = attr->prio_ceiling;
	mutex->pid = task_get_pid( task_get_current() );
	mutex->lock = 0;
	mutex->pthread = -1;
	return 0;
}

/*! @} */


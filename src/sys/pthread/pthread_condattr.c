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
#include "config.h"

#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

/*! \cond */
static int check_initialized(const pthread_condattr_t *attr){
	if ( attr == NULL ){
		errno = EINVAL;
		return -1;
	}
	if ( attr->is_initialized == false ){
		errno = EINVAL;
		return -1;
	}
	return 0;
}
/*! \endcond */

/*! \details This function initializes \a attr with the default values.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL:  attr is NULL
 */
int pthread_condattr_init(pthread_condattr_t *attr){
	attr->is_initialized = true;
	attr->process_shared = false;
	return 0;
}

/*! \details This function destroys \a attr.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL:  \a attr is not an initialized condition attribute
 */
int pthread_condattr_destroy(pthread_condattr_t *attr){
	if( check_initialized(attr) ){
		return -1;
	}
	attr->is_initialized = false;
	return 0;
}

/*! \details This function gets the pshared value for \a attr and stores
 * it in \a pshared.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL:  \a attr is not an initialized condition attribute
 */
int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *pshared){
	if( check_initialized(attr) ){
		return -1;
	}
	*pshared = attr->process_shared;
	return 0;
}

/*! \details This function sets the pshared value in \a attr to \a pshared.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL:  \a attr is not an initialized condition attribute
 */
int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared){
	if( check_initialized(attr) ){
		return -1;
	}
	attr->process_shared = pshared;
	return 0;
}

/*! \details This function gets the clock associated
 * with pthread_cond_timedwait() operations.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL:  \a attr is not an initialized condition attribute
 */
int pthread_condattr_getclock(const pthread_condattr_t * attr, clockid_t * clock_id){
	if( check_initialized(attr) ){
		return -1;
	}
	*clock_id = CLOCK_REALTIME;
	return 0;
}

/*! \details pthread_cond_timedwait() operations
 * always use CLOCK_REALTIME.  This value
 * cannot be changed.
 *
 * \return Zero on success or -1 with errno set to:
 * - ENOTSUP:  this function is not supported
 */
int pthread_condattr_setclock(pthread_condattr_t * attr, clockid_t clock_id){
	errno = ENOTSUP;
	return -1;
}

/*! @} */


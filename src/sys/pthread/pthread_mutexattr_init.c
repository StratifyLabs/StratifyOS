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

/*! \cond */
int pthread_mutexattr_check_initialized(const pthread_mutexattr_t * attr);
/*! \endcond */


/*! \details This function initializes \a attr with default values.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL:  \a attr is NULL
 *
 */
int pthread_mutexattr_init(pthread_mutexattr_t *attr){
#if PTHREAD_SINGLE_MODE == 1
	return 0;
#else
	attr->is_initialized = 1;
	attr->process_shared = 0;
	attr->prio_ceiling = 0;  //! \todo This should be a compile time define
	attr->protocol = 0;
	attr->recursive = 0;
	return 0;
#endif
}

/*! \details This function destroys \a attr.
 *
 * \return Zero on success or -1 with errno set to:
 * - EINVAL:  \a attr is not an initialized mutex attribute object
 *
 */
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr){
#if PTHREAD_SINGLE_MODE == 1
	return 0;
#else
	if ( pthread_mutexattr_check_initialized(attr) ){
		return -1;
	}
	attr->is_initialized = 0;
	return 0;
#endif
}

/*! \cond */
int pthread_mutexattr_check_initialized(const pthread_mutexattr_t * attr){
	if ( attr == NULL ){
		return EINVAL;
	}

	if ( attr->is_initialized != 1 ){
		errno = EINVAL;
		return -1;
	}
	return 0;
}
/*! \endcond */

/*! @} */


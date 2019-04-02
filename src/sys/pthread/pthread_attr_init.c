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
 */

/*! \file */

#include "config.h"
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>


/*! \details This function initializes \a attr to the
 * default values.
 * \return 0 on success
 */
int pthread_attr_init(pthread_attr_t * attr /*! a pointer to the attributes structure */){
	attr->stacksize = PTHREAD_DEFAULT_STACK_SIZE;
    attr->stackaddr = 0;

	//for good measure zero out stack
    PTHREAD_ATTR_SET_IS_INITIALIZED((attr), 1);
	PTHREAD_ATTR_SET_CONTENTION_SCOPE((attr), PTHREAD_SCOPE_SYSTEM);
	PTHREAD_ATTR_SET_GUARDSIZE((attr), SCHED_DEFAULT_STACKGUARD_SIZE);
	PTHREAD_ATTR_SET_INHERIT_SCHED((attr), PTHREAD_EXPLICIT_SCHED);
	PTHREAD_ATTR_SET_DETACH_STATE((attr), PTHREAD_CREATE_JOINABLE);
	PTHREAD_ATTR_SET_SCHED_POLICY((attr), SCHED_OTHER);

	attr->schedparam.sched_priority = SCHED_DEFAULT_PRIORITY;
	return 0;
}

/*! \details Destroys the pthead attributes.
 *
 * @param attr A pointer to the attributes to destroy
 *
 * This function frees the stack associated with the thread. The attributes
 * should not be destroyed until the thread is done executing.
 *
 * \return 0 on success or -1 and errno set to:
 *  - EINVAL:  \a attr is NULL or uninitialized
 */
int pthread_attr_destroy(pthread_attr_t * attr /*! a pointer to the attributes structure */){
    if ( attr == NULL ){ return EINVAL; }

	if ( PTHREAD_ATTR_GET_IS_INITIALIZED(attr) != 1 ){
		errno = EINVAL;
		return -1;
	}

	PTHREAD_ATTR_SET_IS_INITIALIZED((attr), 0);
	return 0;
}

/*! @} */


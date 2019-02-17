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

#include <sys/features.h>

#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sched.h>

/*! \cond */
static int check_initialized(const pthread_attr_t * attr);
/*! \endcond */

/*! \details This function gets the detach state from \a attr and
 * stores it in \a detachstate.
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: \a attr does not refer to an initialized thread attribute object
 */
int pthread_attr_getdetachstate(const pthread_attr_t * attr  /*! a pointer to the attributes structure */,
		int * detachstate /*! the destination for the detach state */){
	if ( check_initialized(attr) < 0 ){
		return -1;
	}

	*detachstate = PTHREAD_ATTR_GET_DETACH_STATE(attr);
	return 0;
}

/*! \details This function sets the detach state in \a attr with \a detachstate.
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: \a attr does not refer to an initialized thread attribute object
 * - EINVAL: \a detachstate is not a valid
 */
int pthread_attr_setdetachstate(pthread_attr_t *attr /*! a pointer to the attributes structure */,
		int detachstate /*! the new detach state (PTHREAD_CREATE_DETACHED or PTHREAD_CREATE_JOINABLE) */){
	if ( check_initialized(attr) < 0 ){
		return -1;
	}

	if ( (detachstate != PTHREAD_CREATE_DETACHED) &&
			(detachstate != PTHREAD_CREATE_JOINABLE) ){
		errno = EINVAL;
		return -1;
	}

	PTHREAD_ATTR_SET_DETACH_STATE(attr, detachstate);
	return 0;
}

/*! \details This function gets the guard size from \a attr and
 * stores it in \a guardsize.
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: \a attr does not refer to an initialized thread attribute object
 */
int pthread_attr_getguardsize(const pthread_attr_t * attr /*! a pointer to the attributes structure */,
		size_t * guardsize /*! the destination for the new guard size */){
	if ( check_initialized(attr) < 0 ){
		return -1;
	}

	*guardsize = PTHREAD_ATTR_GET_GUARDSIZE(attr);
	return 0;
}

/*! \details This function is not supported.  The guard size is
 * a fixed value that cannot be set by the user.
 * \return -1 with errno (see \ref errno) set to ENOTSUP
 */
int pthread_attr_setguardsize(pthread_attr_t *attr,
		size_t guardsize){
	errno = ENOTSUP;
	return -1;
}

/*! \details This function gets the inherit sched value from \a attr and
 * stores it in \a inheritsched.
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: \a attr does not refer to an initialized thread attribute object
 */
int pthread_attr_getinheritsched(const pthread_attr_t * attr /*! a pointer to the attributes structure */,
		int * inheritsched /*! the destination for the inherit sched value */){
#ifndef _POSIX_THREAD_PRIORITY_SCHEDULING
	errno = ENOSYS;
	return -1;
#else
	if ( check_initialized(attr) < 0 ){
		return -1;
	}

	*inheritsched = PTHREAD_ATTR_GET_INHERIT_SCHED(attr);
	return 0;
#endif
}

/*! \details This function sets the inherit sched in \a attr with \a inheritsched.
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: \a attr does not refer to an initialized thread attribute object
 * - EINVAL: \a inheritsched is not a valid value
 */
int pthread_attr_setinheritsched(pthread_attr_t *attr /*! a pointer to the attributes structure */,
		int inheritsched /*! the new inherit sched value */){
#ifndef _POSIX_THREAD_PRIORITY_SCHEDULING
	errno = ENOSYS;
	return -1;
#else
	if ( check_initialized(attr) < 0 ){
		return -1;
	}

	if ( (inheritsched == PTHREAD_INHERIT_SCHED) ||
			(inheritsched == PTHREAD_EXPLICIT_SCHED)){

		PTHREAD_ATTR_SET_INHERIT_SCHED(attr, inheritsched);
		return 0;
	} else {
		errno = EINVAL;
		return -1;
	}

#endif
}

/*! \details This function gets the scheduling parameters from \a attr and
 * stores it in \a param.
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: \a attr does not refer to an initialized thread attribute object
 */
int pthread_attr_getschedparam(const pthread_attr_t *attr /*! a pointer to the attributes structure */,
		struct sched_param *param /*! the destination for the sched param value */){
	if ( check_initialized(attr) < 0 ){
		return -1;
	}

	memcpy(param, &(attr->schedparam), sizeof(struct sched_param));
	return 0;
}

/*! \details This function sets the scheduling parameters in \a attr with \a param.
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: \a attr does not refer to an initialized thread attribute object
 */
int pthread_attr_setschedparam(pthread_attr_t *attr /*! a pointer to the attributes structure */,
		const struct sched_param *param /*! the source for the sched param value */){
	int policy;

	if ( check_initialized(attr) < 0 ){
		return -1;
	}

	policy = PTHREAD_ATTR_GET_SCHED_POLICY(attr);

	if ( param->sched_priority < sched_get_priority_min(policy) ){
		errno = EINVAL;
		return -1;
	}

	if ( param->sched_priority > sched_get_priority_max(policy) ){
		errno = EINVAL;
		return -1;
	}

	memcpy(&(attr->schedparam), param, sizeof(struct sched_param));
	return 0;
}

/*! \details This function gets the scheduling policy from \a attr and
 * stores it in \a policy.
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: \a attr does not refer to an initialized thread attribute object
 */
int pthread_attr_getschedpolicy(const pthread_attr_t *attr /*! a pointer to the attributes structure */,
		int *policy /*! the destination for the schedule policy value */){
	if ( check_initialized(attr) < 0 ){
		return -1;
	}

	*policy = PTHREAD_ATTR_GET_SCHED_POLICY(attr);
	return 0;
}

/*! \details This function sets the scheduling policy in \a attr with \a policy.
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: \a attr does not refer to an initialized thread attribute object
 * - EINVAL: \a policy does not refer to a valid policy.
 */
int pthread_attr_setschedpolicy(pthread_attr_t *attr /*! a pointer to the attributes structure */,
		int policy /*! the new policy value (SCHED_FIFO, SCHED_RR, or SCHED_OTHER) */){
	if ( check_initialized(attr) < 0 ){
		return -1;
	}

	switch(policy){
	case SCHED_FIFO:
	case SCHED_RR:
	case SCHED_OTHER:
		PTHREAD_ATTR_SET_SCHED_POLICY(attr, policy);
		return 0;
	default:
		errno = EINVAL;
		return -1;
	}

}

/*! \details This function gets the contention scope from \a attr and
 * stores it in \a contentionscope.
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: \a attr does not refer to an initialized thread attribute object
 */
int pthread_attr_getscope(const pthread_attr_t *attr /*! a pointer to the attributes structure */,
		int *contentionscope /*! the destination for the contention scope value */){
#ifndef _POSIX_THREAD_PRIORITY_SCHEDULING
	errno = ENOTSUP;
	return -1;
#else
	if ( check_initialized(attr) < 0 ){
		return -1;
	}

	*contentionscope = PTHREAD_ATTR_GET_CONTENTION_SCOPE(attr);
	return 0;
#endif
}

/*! \details This function sets the contention scope in \a attr with \a contentionscope.
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: \a attr does not refer to an initialized thread attribute object
 * - ENOTSUP:  contentionscope is not PTHREAD_SCOPE_SYSTEM or PTHREAD_SCOPE_PROCESS
 */
int pthread_attr_setscope(pthread_attr_t *attr /*! a pointer to the attributes structure */,
		int contentionscope /*! the new contention scope value */){
#ifndef _POSIX_THREAD_PRIORITY_SCHEDULING
	errno = ENOSYS;
	return -1;
#else
	if ( check_initialized(attr) < 0 ){
		return -1;
	}

	switch(contentionscope){
	case PTHREAD_SCOPE_SYSTEM:
		PTHREAD_ATTR_SET_CONTENTION_SCOPE(attr, contentionscope);
		return 0;
	case PTHREAD_SCOPE_PROCESS:
	default:
		errno = EINVAL;
		return -1;
	}
#endif
}

/*! \details This functions gets the stack size from \a attr and
 * stores it in \a stacksize.
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: \a attr does not refer to an initialized thread attribute object
 */
int pthread_attr_getstacksize(const pthread_attr_t *attr /*! a pointer to the attributes structure */,
		size_t *stacksize /*! the destination for the new stack size */){
	if ( check_initialized(attr) < 0 ){
		return -1;
	}

	*stacksize = attr->stacksize;
	return 0;
}

/*! \details This function sets the stack size in \a attr with \a stacksize.
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: \a attr does not refer to an initialized thread attribute object
 * - EINVAL: \a stacksize is too low of a value
 * - ENOMEM:  not enough memory
 */
int pthread_attr_setstacksize(pthread_attr_t *attr /*! a pointer to the attributes structure */,
		size_t stacksize /*! the new stack size value */){
	if ( check_initialized(attr) < 0 ){
		return -1;
	}

    if( stacksize >= PTHREAD_STACK_MIN ){
        attr->stacksize = stacksize;
        return 0;
    }

    errno = EINVAL;
    return -1;
}

/*! \details This functions gets the stack address from \a attr and
 * stores it in \a stackaddr.
 * \return Zero on success or -1 with errno set to:
 * - EINVAL: \a attr does not refer to an initialized thread attribute object
 */
int pthread_attr_getstackaddr(const pthread_attr_t *attr /*! a pointer to the attributes structure */,
		void **stackaddr /*! the destination for the stack address */){
    errno = ENOTSUP;
    return -1;
}

/*! \details This function is not supported.
 * \return -1 with errno equal to ENOTSUP
 */
int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr){
	errno = ENOTSUP;
	return -1;
}

/*! \cond */
int check_initialized(const pthread_attr_t * attr){
	if ( attr == NULL ){
		return EINVAL;
	}

	if ( PTHREAD_ATTR_GET_IS_INITIALIZED(attr) != 1 ){
		errno = EINVAL;
		return -1;
	}
	return 0;
}
/*! \endcond */


/*! @} */



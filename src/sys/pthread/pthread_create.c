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

/*! \addtogroup PTHREAD
 * @{
 *
 * \details This is the interface for POSIX threads.  Here is an example of creating a new thread:
 *
 * \code
 * #include <pthread.h>
 *
 * void * thread_function(void * args);
 *
 * pthread_t t;
 * pthread_attr_t attr;
 *
 * if ( pthread_attr_init(&attr) < 0 ){
 * 	perror("failed to initialize attr");
 * 	return -1;
 * }
 *
 * if ( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) < 0 ){
 * 	perror("failed to set detach state");
 * 	return -1;
 * }
 *
 * if ( pthread_attr_setstacksize(&attr, 4096) < 0 ){
 * 	perror("failed to set stack size");
 * 	return -1;
 * }
 *
 * if ( pthread_create(&t, &attr, thread_function, NULL) ){
 * 	perror("failed to create thread");
 * 	return -1;
 * }
 *
 * \endcode
 *
 */

/*! \file */

#include "config.h"

#include <pthread.h>
#include <errno.h>


#include "../scheduler/scheduler_local.h"


static void priv_join_thread(void * args) MCU_ROOT_EXEC_CODE;

/*! \details This function creates a new thread.
 * \return Zero on success or -1 with \a errno (see \ref ERRNO) set to:
 * - ENOMEM: error allocating memory for the thread
 * - EAGAIN: insufficient system resources to create a new thread
 *
 *
 */
int pthread_create(pthread_t * thread /*! If not null, the thread id is written here */,
		const pthread_attr_t * attr /*! Sets the thread attributes (defaults are used if this is NULL) */,
		void *(*start_routine)(void *) /*! A pointer to the start routine */,
		void *arg /*! A pointer to the start routine's single argument */){

	int id;
	pthread_attr_t attrs;

	if ( attr == NULL ){
		if ( pthread_attr_init(&attrs) < 0 ){
			//Errno is set by pthread_attr_init()
			return -1;
		}
	} else {
		memcpy(&attrs, attr, sizeof(pthread_attr_t));
	}

	id = scheduler_create_thread(start_routine,
			arg,
			attrs.stackaddr,
			attrs.stacksize,
			&attrs);

	if ( id ){
		if ( thread ){
			*thread = id;
		}
		return 0;
	} else {
		if ( attr == NULL ){
			pthread_attr_destroy(&attrs);
		}
		errno = EAGAIN;
		return -1;
	}
}


/*! \details This function blocks the calling thread until \a thread terminates.
 * \return Zero on success or -1 with \a errno (see \ref ERRNO) set to:
 * - ESRCH: \a thread does not exist
 * - EDEADLK: a deadlock has been detected or \a thread refers to the calling thread
 * - EINVAL: \a thread does not refer to a joinable thread.
 */
int pthread_join(pthread_t thread, void ** value_ptr){

	if( (thread < task_get_total()) && (thread >= 0) ){
		if ( task_enabled(thread) ){
			//now see if the thread is joinable
			if ( PTHREAD_ATTR_GET_DETACH_STATE( (&(sos_sched_table[thread].attr))) != PTHREAD_CREATE_JOINABLE ){
				errno = EINVAL;
				return -1;
			}


			//See if the thread is joined to this thread
			if ( (sos_sched_table[thread].block_object == (void*)&sos_sched_table[task_get_current()]) ||
					(thread == task_get_current()) ){
				errno = EDEADLK;
				return -1;
			}


			do {
				cortexm_svcall(priv_join_thread, &thread);
				if ( thread < 0 ){
					errno = ESRCH;
					return -1;
				}
			} while( scheduler_unblock_type(task_get_current()) != SCHEDULER_UNBLOCK_PTHREAD_JOINED_THREAD_COMPLETE);

			if ( value_ptr != NULL ){
				//When the thread terminates, it puts the exit value in this threads scheduler table entry
				*value_ptr = (void*)(sos_sched_table[ task_get_current() ].exit_status);
			}

			return 0;
		}
	}
	errno = ESRCH;
	return -1;
}

void priv_join_thread(void * args){
	int * p = (int*)args;
	int id = *p;

	if ( task_enabled(id) ){
		sos_sched_table[task_get_current()].block_object = (void*)&sos_sched_table[id]; //block on the thread to be joined
		//If the thread is waiting to be joined, it needs to be activated
		if ( sos_sched_table[id].block_object == (void*)&sos_sched_table[id].block_object ){
			scheduler_root_assert_active(id, SCHEDULER_UNBLOCK_PTHREAD_JOINED);
		}
		scheduler_root_update_on_sleep();
	} else {
		*p = -1;
	}
}



/*! @} */

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

/*! \addtogroup SEMAPHORE
 * @{
 *
 */

/*! \file */

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "sos/fs/sysfs.h"

#include "semaphore.h"
#include "../sched/sched_flags.h"

#include "mcu/debug.h"

#define SEM_FILE_HDR_SIGNATURE 0x1285ABC8
#define SEM_FILE_HDR_NOT_SIGNATURE (~SEM_FILE_HDR_SIGNATURE)

typedef struct {
	uint32_t signature;
	uint32_t not_signature;
	sem_t * sem;
} sem_file_hdr_t;

typedef struct {
	sem_t sem;
	void * next;
} sem_list_t;

static void priv_sem_wait(void * args) MCU_PRIV_EXEC_CODE;
static void priv_sem_post(void * args) MCU_PRIV_EXEC_CODE;
static void priv_sem_trywait(void * args) MCU_PRIV_EXEC_CODE;
static void priv_sem_timedwait(void * args) MCU_PRIV_EXEC_CODE;
static int check_initialized(sem_t * sem);

typedef struct {
	sem_t * sem;
	int id;
	int new_thread;
	int ret;
} priv_sem_t;

static sem_list_t * sem_first = 0;

static sem_t * sem_find_named(const char * name){
	sem_list_t * entry;
	for(entry = sem_first; entry != 0; entry = entry->next){
		if( entry->sem.is_initialized != 0 ){
			if( strncmp(entry->sem.name, name, NAME_MAX) == 0 ){
				return &entry->sem;
			}
		}
	}
	return SEM_FAILED;
}

static sem_t * sem_find_free(){
	sem_list_t * entry;
	sem_list_t * new_entry;
	sem_list_t * last_entry;
	last_entry = 0;
	for(entry = sem_first; entry != 0; entry = entry->next){
		last_entry = entry;
		if( entry->sem.is_initialized == 0 ){
			return &entry->sem;
		}
	}

	//no free semaphores
	new_entry = _malloc_r(task_table[0].global_reent, sizeof(sem_list_t));
	if( new_entry == 0 ){
		return SEM_FAILED;
	}
	if( last_entry == 0 ){
		sem_first = new_entry;
	} else {
		last_entry->next = new_entry;
	}
	new_entry->next = 0;
	return &new_entry->sem;
}

typedef struct {
	sem_t * sem;
	struct sched_timeval interval;
} priv_sem_timedwait_t;

/*! \details This function initializes \a sem as an unnamed semaphore with
 * \a pshared and \a value.
 *
 * \return Zero on success or -1 with errno (see \ref ERRNO) set to:
 * - EINVAL:  sem is NULL
 *
 */
int sem_init(sem_t *sem, int pshared, unsigned int value){
	if ( sem == NULL ){
		errno = EINVAL;
		return -1;
	}
	sem->is_initialized = 1;
	sem->value = value;
	sem->pshared = pshared;
	sem->pid = getpid();
	memset(sem->name, 0, NAME_MAX); //This is an unnamed semaphore
	return 0;
}

/*! \details This function destroys \a sem--an unnamed semaphore.
 *
 * \return Zero on success or -1 with errno (see \ref ERRNO) set to:
 * - EINVAL:  sem is NULL
 *
 */
int sem_destroy(sem_t *sem){
	if ( check_initialized(sem) < 0 ){
		return -1;
	}

	sem->is_initialized = 0;
	return 0;
}


/*! \details This function gets the value of the semaphore.  If the semaphore is locked,
 * the value is zero.
 *
 * \return Zero on success or -1 with errno (see \ref ERRNO) set to:
 * - EINVAL:  sem is NULL
 *
 */
int sem_getvalue(sem_t *sem, int *sval){
	if ( check_initialized(sem) < 0 ){
		return -1;
	}

	*sval = sem->value;
	return 0;
}

/*! \details This function opens or creates a named semaphore.
 *
 * \param name The name of the message queue
 * \param oflag The flags to use when opening (O_CREAT, O_EXCL, O_RDWR)
 *
 * When using O_CREAT, the third argument is the mode and the fourth is
 * the initial value of the semaphore:
 * \code
 * sem_t * sem_open(const char * name, int oflag, int mode, int value){
 * \endcode
 *
 * \return Zero on success or SEM_FAILED with errno (see \ref ERRNO) set to:
 * - ENAMETOOLONG:  name length is greater than NAME_MAX
 * - EEXIST:  O_CREAT and O_EXCL are set in \a oflag but the semaphore already exists
 * - ENOENT:  O_CREAT is not set in \a oflag and the semaphore does not exist
 * - ENOMEM:  not enough memory for the semaphore
 *
 */
sem_t * sem_open(const char * name, int oflag, ...){
	sem_t * new_sem;
	mode_t mode;
	unsigned value;
	int action;
	va_list ap;

	if ( strnlen(name, NAME_MAX) == NAME_MAX ){
		errno = ENAMETOOLONG;
		return SEM_FAILED;
	}

	//Check to see if the  semaphore exists
	new_sem = sem_find_named(name);

	//Check the flags to determine the appropriate action
	if ( oflag & O_CREAT ){
		if ( oflag & O_EXCL ){
			if ( new_sem == 0 ){
				//Create the new semaphore
				action = 0;
			} else {
				errno = EEXIST;
				return SEM_FAILED;
			}
		} else {
			if ( new_sem == 0 ){
				//Create a new semaphore
				action = 0;
			} else {
				//Read the existing semaphore
				action = 1;
			}
		}
	} else {
		if ( new_sem == 0 ){
			errno = ENOENT;
			return SEM_FAILED;
		} else {
			//Read the existing semaphore
			action = 1;
		}
	}

	switch(action){
	case 0:
		//Create the new semaphore
		new_sem = sem_find_free();
		if ( new_sem == NULL ){
			//errno is set by malloc
			return SEM_FAILED;
		}

		va_start(ap, oflag);
		mode = va_arg(ap, mode_t);
		value = va_arg(ap, unsigned);
		va_end(ap);

		new_sem->is_initialized = 1;
		new_sem->value = value;
		new_sem->references = 1;
		new_sem->mode = mode;
		new_sem->pshared = 1;
		strncpy(new_sem->name, name, NAME_MAX-1);
		break;

	case 1:
		//use the existing semaphore
		new_sem->references++;
		break;
	}

	return new_sem;
}

/*! \details This function closes a semaphore.  The semaphore
 * must be deleted using sem_unlink() in order to free the system resources
 * associated with the semaphore.
 *
 * \return Zero on success or SEM_FAILED with errno (see \ref ERRNO) set to:
 * - EINVAL:  sem is NULL
 *
 */
int sem_close(sem_t *sem){
	if ( check_initialized(sem) < 0 ){
		return -1;
	}

	if( sem->references > 0 ){
		sem->references--;
	}

	if ( sem->references == 0 ){
		if( sem->is_initialized == 2 ){
			//Close and delete
			sem->is_initialized = 0;
			return 0;
		}
	}
	return 0;
}

void priv_sem_post(void * args){
	int id = *((int*)args);
	stratify_sched_table[id].block_object = NULL;
	sched_priv_assert_active(id, SCHED_UNBLOCK_SEMAPHORE);
	if( !sched_stopped_asserted(id) ){
		sched_priv_update_on_wake( stratify_sched_table[id].priority );
	}
}

/*! \details This function unlocks (increments) the value of
 * the semaphore.
 *
 *
 * \return Zero on success or SEM_FAILED with errno (see \ref ERRNO) set to:
 * - EINVAL:  sem is NULL
 * - EACCES:  process cannot access semaphore
 *
 */
int sem_post(sem_t *sem){
	int new_thread;

	if ( check_initialized(sem) < 0 ){
		return -1;
	}

	//unlock the semaphore -- increment the semaphore
	sem->value++;

	//see if any tasks are blocked on this semaphore
	new_thread = sched_get_highest_priority_blocked(sem);

	if ( new_thread != -1 ){
		mcu_core_privcall(priv_sem_post, &new_thread);
	}

	return 0;
}

void priv_sem_timedwait(void * args){
	priv_sem_timedwait_t * argsp = (priv_sem_timedwait_t*)args;

	if ( argsp->sem->value <= 0 ){
		sched_priv_timedblock(argsp->sem, &argsp->interval);
	}

	argsp->sem->value--;
}


/*! \details This function waits to lock (decrement) the semaphore until the
 * value of \a CLOCK_REALTIME exceeds the value of \a abs_timeout.
 *
 * \return Zero on success or SEM_FAILED with errno (see \ref ERRNO) set to:
 * - EINVAL:  sem is NULL
 * - EACCES:  process cannot access semaphore
 * - ETIMEDOUT:  timeout expired without locking the semaphore
 *
 */
int sem_timedwait(sem_t * sem, const struct timespec * abs_timeout){
	//timed wait
	priv_sem_timedwait_t args;

	if ( check_initialized(sem) < 0 ){
		return -1;
	}

	args.sem = sem;
	sched_convert_timespec(&args.interval, abs_timeout);

	mcu_core_privcall(priv_sem_timedwait, &args);

	//Check for a timeout or a lock
	if ( sched_get_unblock_type(task_get_current()) == SCHED_UNBLOCK_SLEEP){
		//The timeout expired
		sem->value++; //lock is aborted
		errno = ETIMEDOUT;
		return -1;
	}

	return 0;
}

void priv_sem_trywait(void * args){
	priv_sem_t * p = (priv_sem_t*)args;

	if ( p->sem->value > 0 ){
		p->sem->value--;
		p->ret = 0;
	} else {
		errno = EAGAIN;
		p->ret = -1;
	}
}

/*! \details This function locks (decrements) the semaphore if it can be locked
 * immediately.
 *
 * \return Zero on success or -1 with errno (see \ref ERRNO) set to:
 * - EINVAL:  sem is NULL
 * - EACCES:  process cannot access semaphore
 * - EAGAIN:  \a sem could not be immediately locked.
 *
 */
int sem_trywait(sem_t *sem){
	priv_sem_t args;

	if ( check_initialized(sem) < 0 ){
		return -1;
	}

	args.sem = sem;

	mcu_core_privcall(priv_sem_trywait, &args);

	return args.ret;
}


/*! \details This function unlinks a named semaphore.  It
 * also releases any system resources associated with the semaphore.
 *
 * \return Zero on success or SEM_FAILED with errno (see \ref ERRNO) set to:
 * - ENOENT:  the semaphore with \a name could not be found
 * - ENAMETOOLONG:  the length of \a name exceeds \a NAME_MAX
 *
 */
int sem_unlink(const char *name){
	sem_t * sem;

	if ( strnlen(name, NAME_MAX) == NAME_MAX ){
		errno = ENAMETOOLONG;
		return -1;
	}


	sem = sem_find_named(name);
	if( sem == SEM_FAILED ){
		errno = ENOENT;
		return -1;
	}


	if ( check_initialized(sem) < 0 ){
		return -1;
	}

	if ( sem->references == 0 ){
		//Close and delete
		sem->is_initialized = 0;
		return 0;
	} else {
		//Close but don't delete until all references are gone
		sem->is_initialized = 2;
		return 0;
	}


	return -1;
}

void priv_sem_wait(void * args){
	sem_t * sem = (sem_t*)args;

	stratify_sched_table[ task_get_current() ].block_object = args;

	if ( sem->value <= 0){
		//task must be blocked until the semaphore is available
		sched_priv_update_on_sleep();
	}

	sem->value--;
}

/*! \details This function locks (decrements) the semaphore.  If
 * the semaphore is already zero (and therefore cannot be locked),
 * the calling thread blocks until the semaphore becomes available.
 *
 * \return Zero on success or SEM_FAILED with errno (see \ref ERRNO) set to:
 * - EINVAL:  \a sem is NULL
 * - EACCES:  \a sem is not pshared and was created in another process
 *
 */
int sem_wait(sem_t *sem){
	if ( check_initialized(sem) < 0 ){
		return -1;
	}

	mcu_core_privcall(priv_sem_wait, sem);
	return 0;
}

int check_initialized(sem_t * sem){
	if( sem == NULL ){
		errno = EINVAL;
		return -1;
	}

	if ( sem->is_initialized == 0 ){
		errno = EINVAL;
		return -1;
	}

	if ( (sem->pshared == 0) && (task_get_pid(task_get_current())) != sem->pid ){
		errno = EACCES;
		return -1;
	}

	return 0;
}


/*! @} */

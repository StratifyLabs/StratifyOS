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


/*! \addtogroup signal
 *
 * @{
 *
 */

/*! \file */

#include "mcu/debug.h"
#include <signal.h>
#include <errno.h>
#include "../scheduler/scheduler_local.h"
#include "sig_local.h"

/*! \cond */
static int alloc_sigactions();
static int alloc_sigaction(int sig);
/*! \endcond */

/*! \details This function sets the handler for \a sig to
 * \a func.
 *
 *
 * \return Zero or SIG_ERR with errno (see \ref errno) set to:
 * - EINVAL: \a sig is not a valid signal or is a signal that cannot be caught.
 *
 */
_sig_func_ptr signal(int sig, _sig_func_ptr func){
	if ( sig < SCHEDULER_NUM_SIGNALS ){

		//check for signals that cannot be caught or ignored
		if ( (sig == SIGKILL) || (sig == SIGSTOP) ){
			errno = EINVAL;
			return SIG_ERR;
		}

		if ( alloc_sigactions() < 0 ){
			return SIG_ERR;
		}

		if ( alloc_sigaction(sig) < 0 ){
			return SIG_ERR;
		}

		if( func == SIG_IGN ){
			//clear the pending flag for the signal
			THREAD_SIGPENDING &= ~(1<<sig);
		}

		GLOBAL_SIGACTION(sig)->sa_handler = func;
		GLOBAL_SIGACTION(sig)->sa_flags = 0;

	} else {
		errno = EINVAL;
		return SIG_ERR;
	}

	return func;
}


/*! \details This function sets the action for \a sig to \a act.  The previous
 * action is written is \a oact if \a oact is not NULL.  Using this function will
 * override any handler previously set with \ref signal().
 *
 *
 * \return Zero or SIG_ERR with errno (see \ref errno) set to:
 * - EINVAL: \a sig is not a valid signal or is a signal that cannot be caught.
 * - EINTR: a signal, not in \a set, was caught
 *
 */
int sigaction(int sig, const struct sigaction * act, struct sigaction * oact){

	if ( sig < SCHEDULER_NUM_SIGNALS ){

		//check for signals that cannot be caught or ignored
		if ( (sig == SIGKILL) || (sig == SIGSTOP) ){
			errno = EINVAL;
			return -1;
		}

		if ( alloc_sigactions() < 0 ){
			return -1;
		}

		if ( alloc_sigaction(sig) < 0 ){
			return -1;
		}

		if ( oact != NULL ){
			//copy the existing action to oact
			memcpy(oact, GLOBAL_SIGACTION(sig), sizeof(struct sigaction));
		}

		memcpy(GLOBAL_SIGACTION(sig), act, sizeof(struct sigaction));
		//GLOBAL_SIGACTION(sig)->sa_flags |= (1<<SA_SIGINFO);

	} else {
		errno = EINVAL;
		return -1;
	}

	return 0;
}

/*! \details This function sets the signal mask for the current thread.
 *
 *
 * \return Zero or SIG_ERR with errno (see \ref errno) set to:
 * - EINVAL: \a how is not one of SIG_BLOCK, SIG_SETMASK, or SIG_UNBLOCK
 *
 */
int pthread_sigmask(int how, const sigset_t * set, sigset_t * oset){
	sigset_t mask;

	mask = THREAD_SIGMASK;

	switch(how){
	case SIG_BLOCK: //union of current set and input set (or)
		mask |= *set;
		break;
	case SIG_SETMASK: //the input set
		mask = *set;
		break;
	case SIG_UNBLOCK: //intersection of current set and input set (and)
		//check to see if the signal is currently pending -- if it is, mark unpending and execute handler
		mask &= ~(*set);
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	//these cannot be blocked -- ignore attempts to do so
	sigdelset(&mask, SIGKILL);
	sigdelset(&mask, SIGSTOP);

	if( oset != NULL ){
		//store the current set in oset
		*oset = THREAD_SIGMASK;
	}

	THREAD_SIGMASK = mask;

	return 0;
}

/*! \details This function sets the signal mask for the current thread.
 * This function should only be called in single threaded applications.
 *
 * \return Zero or SIG_ERR with errno (see \ref errno) set to:
 * - EINVAL: \a how is not one of SIG_BLOCK, SIG_SETMASK, or SIG_UNBLOCK
 */
int sigprocmask(int how, const sigset_t * set, sigset_t * oset){
	//! \todo the calling thread must be a process (thread 0 of the process)
	return pthread_sigmask(how, set, oset);
}


/*! \details This function writes the calling thread's current pending set to
 * \a set.  It then waits for a signal to arrive.  If the action for the signal is
 * to terminate, then this function never returns.  If the action is to
 * execute a signal catching function, the signal catching function is executed and
 * the signal mask for the thread is restored before returning.
 *
 * \return -1 with errno set to EINTR otherwise never returns
 */
int sigpending(sigset_t * set){
	//grab the block and pending signals
	*set = THREAD_SIGMASK & THREAD_SIGPENDING;
	return 0;
}

int sigsuspend(const sigset_t * sigmask){
	sigset_t mask;

	mask = THREAD_SIGMASK;
	THREAD_SIGMASK = *sigmask;

	//Suspend this thread until delivery of a signal
	cortexm_svcall(signal_svcall_wait, NULL);

	errno = EINTR;
	THREAD_SIGMASK = mask;
	return -1;
}

/*

//These are XSI extensions
int sighold(int sig){
	//Add to signal mask of calling process
	return 0;
}

int sigignore(int sig){
	//set disposition of signal to SIG_IGN
	return 0;
}

int sigpause(int sig){
	//remove sig from signal mask and suspend until a signal is received

	//restore signal mask before returning
	return 0;
}

int sigrelse(int sig){
	//remove sig from the signal mask
	return 0;
}
 */

/*! \cond */
int alloc_sigactions(){
	int tmp;
	void * mem;
	if( GLOBAL_SIGACTIONS == NULL ){
		tmp = sizeof(struct sigaction*) * SCHEDULER_NUM_SIGNALS;
		mem = malloc(tmp);
		if( mem == NULL ){
			//memory allocation failure
			return -1;
		}
		GLOBAL_SIGACTIONS = mem;
		memset(GLOBAL_SIGACTIONS, 0, tmp);
	}
	return 0;
}

int alloc_sigaction(int sig){
	int tmp;
	void * mem;
	if ( GLOBAL_SIGACTION(sig) == NULL ){
		tmp = sizeof(struct sigaction);
		mem = malloc(tmp);
		if ( mem == NULL ) {
			//memory allocation failure
			return -1;
		}
		memset(mem, 0, tmp);
		GLOBAL_SIGACTION(sig) = mem;
	}
	return 0;
}
/*! \endcond */

/*! @} */


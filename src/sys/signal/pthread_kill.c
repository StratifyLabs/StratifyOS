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
 * @{
 */

/*! \file */

#include <pthread.h>

#include "config.h"
#include "mcu/mcu.h"
#include "cortexm/task.h"
#include "mcu/debug.h"
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include "sos/sos.h"
#include "../scheduler/scheduler_local.h"
#include "sig_local.h"

#include "mcu/debug.h"

/*! \cond */
static int signal_root_forward(int send_tid,
										 int tid,
										 int si_signo,
										 int si_sigcode,
										 int sig_value) MCU_ROOT_CODE;
static void signal_forward_handler(int send_tid, int signo, int sigcode, int sigvalue);

//this checks to see if sending a signal will cause a stack/heap collision in the target thread
static void svcall_check_signal_stack(void * args) MCU_ROOT_EXEC_CODE;
static void root_check_signal_stack(void * args) MCU_ROOT_CODE;

void svcall_check_signal_stack(void * args){
	CORTEXM_SVCALL_ENTER();
	root_check_signal_stack(args);
}

void root_check_signal_stack(void * args){
	int * result = (int*)args;
	int tid = *result;
	u32 sp;

	*result = 0;

	//Check to see if stacking an interrupt handler will cause a stack heap collision
	if( tid != task_get_current() ){
		//check the target stack pointer
		sp = (u32)sos_task_table[tid].sp;
	} else {
		//read the current stack pointer
		cortexm_get_thread_stack_ptr((void**)&sp);
	}

	//stackguard * 2 gives the handler a little bit of memory
	if( (sp - task_interrupt_stacksize() -
		  (2*SCHED_DEFAULT_STACKGUARD_SIZE)) <
		 (u32)(sos_task_table[tid].mem.stackguard.address) ){
		*result = -1;
	}
}

int devfs_signal_callback(void * context, const mcu_event_t * data){
	//This only works if the other parts of the interrupt handler have not modified the stack
	devfs_signal_callback_t * args = (devfs_signal_callback_t*)context;
	if( signal_root_send(0, args->tid, args->si_signo, args->si_sigcode, args->sig_value, 1) < 0){
		//return 0; //this will dis-regard the callback so additional events stop sending signals
	}
	return args->keep; //non-zero return means to leave callback in place
}

void signal_forward_handler(int send_tid, int signo, int sigcode, int sigvalue){
	//This is called in a non-privileged context on task 0
	signal_send(send_tid, signo, sigcode, sigvalue);
}

int signal_root_forward(int send_tid, int tid, int si_signo, int si_sigcode, int sig_value){
	task_interrupt_t intr;
	int check_stack;

	//make sure the task id is valid
	if ( (u32)tid < task_get_total() ){
		if ( si_signo != 0 ){
			if ( si_signo < SCHEDULER_NUM_SIGNALS ){

				check_stack = tid;
				root_check_signal_stack(&check_stack);
				if( check_stack < 0 ){
					errno = ENOMEM;
					return -1;
				}

				intr.tid = tid;
				intr.handler = (task_interrupt_handler_t)signal_forward_handler;
				intr.sync_callback = (cortexm_svcall_t)signal_root_activate;
				intr.sync_callback_arg = &tid;
				intr.arg[0] = send_tid;
				intr.arg[1] = si_signo;
				intr.arg[2] = si_sigcode;
				intr.arg[3] = sig_value;
				task_svcall_interrupt(&intr);
			} else {
				return -1;
			}
		}
	} else {
		return -1;
	}
	return 0;
}


int signal_root_send(int send_tid, int tid, int si_signo, int si_sigcode, int sig_value, int forward){
	task_interrupt_t intr;
	int check_stack;

	if( ( (tid == task_get_current()) ) && (forward != 0) ){
		//If the receiving tid is currently executing, sending the signal directly will corrupt the stack
		//So we stack a signal on task 0 and have it send the signal
		//This only happens when root signals are sent because the stack is in an unknown state
		return signal_root_forward(tid, 0, si_signo, si_sigcode, sig_value);
	}

	//make sure the task id is valid
	if ( (u32)tid < task_get_total() ){
		if ( si_signo != 0 ){
			if ( si_signo < SCHEDULER_NUM_SIGNALS ){

				check_stack = tid;
				svcall_check_signal_stack(&check_stack);
				if( check_stack < 0 ){
					return SYSFS_SET_RETURN(ENOMEM);
				}


				intr.tid = tid;
				intr.handler =
						(task_interrupt_handler_t)signal_handler;
				intr.sync_callback =
						(cortexm_svcall_t)signal_root_activate;
				intr.sync_callback_arg = &tid;
				intr.arg[0] = send_tid;
				intr.arg[1] = si_signo;
				intr.arg[2] = si_sigcode;
				intr.arg[3] = sig_value;
				task_svcall_interrupt(&intr);
			} else {
				return SYSFS_SET_RETURN(EINVAL);
			}
		} else {
			return SYSFS_SET_RETURN(EINVAL);
		}
	} else {
		return SYSFS_SET_RETURN(ESRCH);
	}
	return 0;
}


int signal_send(int tid, int si_signo, int si_sigcode, int sig_value){
	task_interrupt_t intr;
	pthread_mutex_t * delay_mutex;
	int check_stack;
	struct timespec abstime;

	//make sure the task id is valid
	if ( scheduler_check_tid(tid) ){
		errno = ESRCH;
		return -1;
	}

	if ( si_signo != 0 ){

		delay_mutex = sos_sched_table[tid].signal_delay_mutex;
		//check for the signal delay mutex
		if( delay_mutex != NULL ){
			//this means the target task is doing critical work (like modifying the filesystem)
			clock_gettime(CLOCK_REALTIME, &abstime);
			abstime.tv_sec++; //give the mutex one second -- then send the signal
			if ( pthread_mutex_timedlock(delay_mutex, &abstime) == 0 ){ //wait until the task releases the mutex
				pthread_mutex_unlock(delay_mutex);  //unlock the mutex then continue
			}
		}

		if ( si_signo < SCHEDULER_NUM_SIGNALS ){
			check_stack = tid;
			cortexm_svcall(svcall_check_signal_stack, &check_stack);
			if( check_stack < 0 ){
				return SYSFS_SET_RETURN(ENOMEM);
			}

			intr.tid = tid;
			intr.handler = (task_interrupt_handler_t)signal_handler;
			intr.sync_callback = (cortexm_svcall_t)signal_root_activate;
			intr.sync_callback_arg = &tid;
			intr.arg[0] = task_get_current();
			intr.arg[1] = si_signo;
			intr.arg[2] = si_sigcode;
			intr.arg[3] = sig_value;
			task_interrupt(&intr);
		} else {
			errno = EINVAL;
			return -1;
		}
	}
	return 0;
}

void signal_root_activate(int * thread){
	int id = *thread;
	scheduler_root_deassert_stopped(id);
	scheduler_root_assert_active(id, SCHEDULER_UNBLOCK_SIGNAL);
	scheduler_root_update_on_wake(id, scheduler_priority(id));
}
/*! \endcond */

/*! \details This function sends the signal \a signo to \a thread.
 * The handler is executed in the context of \a thread, but the
 * signal effects the entire process.  For example,
 * \code
 * pthread_kill(8, SIGKILL);
 * \endcode
 * will kill the process that holds thread 8.  The exit handler
 * will be executed on thread 8's stack.
 *
 *
 * \return Zero or -1 with errno (see \ref errno) set to:
 * - EINVAL: \a signo is not a valid signal number
 * - ESRCH: \a pid is not a valid process id
 *
 */
int pthread_kill(pthread_t thread, int signo){
	return signal_send(thread, signo, SI_USER, 0);
}





/*! @} */

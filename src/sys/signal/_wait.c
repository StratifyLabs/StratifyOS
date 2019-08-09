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

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include "../scheduler/scheduler_local.h"
#include "sig_local.h"

/*! \cond */
static void svcall_wait_child(void * args) MCU_ROOT_EXEC_CODE;

typedef struct {
	int pid;
	int status;
	int tid;
} svcall_check_for_zombie_child_t;

static void svcall_check_for_zombie_child(void * args) MCU_ROOT_EXEC_CODE;
/*! \endcond */

pid_t waitpid(pid_t pid, int *stat_loc, int options){
	svcall_check_for_zombie_child_t args;

	if ( (pid < -1) || (pid == 0) ){
		errno = ENOTSUP;
		return -1;
	}


	do {
		args.tid = 0;
		args.pid = pid;
		if ( !(options & WNOHANG) ){
			cortexm_svcall(svcall_wait_child, &args);
			//sleep here and wait for the signal to arrive
		}


		//process has awoken
		if( args.tid == 0 ){
			if( SIGCHLD_ASSERTED() ){
				//signal has arrived -- check again for the zombie
				cortexm_svcall(svcall_check_for_zombie_child, &args);
			} else if ( !(options & WNOHANG) ){
				errno = EINTR;
				return -1;
			}
		}

		if( options & WNOHANG ){
			break;
		}

		//we loop here in case a process sends SIGCHLD without being a zombie
	} while( args.tid == 0 );

	//for WNOHANG
	if( args.tid == 0 ){
		return 0;
	}

	//no more children
	if ( args.tid < 0 ){
		errno = ECHILD;
		return -1;
	}

	//Read the status information from the child
	if ( stat_loc != NULL ){
		*stat_loc = args.status;
	}

	//return the pid of the child process
	return task_get_pid( args.tid );
}

pid_t wait(int *stat_loc);

/*! \cond */
pid_t _wait(int *stat_loc){
	return waitpid(-1, stat_loc, 0);
}

void svcall_check_for_zombie_child(void * args){
	CORTEXM_SVCALL_ENTER();
	int num_children;
	svcall_check_for_zombie_child_t * p;
	p = (svcall_check_for_zombie_child_t*)args;
	int i;
	int num_zombies;
	int current_pid;

	num_zombies = 0;
	num_children = 0;

	current_pid = task_get_pid( task_get_current() );

	p->tid = -1;

	for(i=1; i < task_get_total(); i++){
		if( task_enabled(i) ){
			//must check to see if the child is orphaned as well -- don't wait for orphaned children

			if( task_get_pid( task_get_parent(i) ) == current_pid ){ //is the task a child
				num_children++;
				if ( scheduler_zombie_asserted(i) ){
					//this zombie process is ready
					if ( (task_get_pid(i) == p->pid) || (p->pid == -1) ){ //matching pid or any pid?
						if( SIGCHLD_ASSERTED() ){
							if( num_zombies == 0 ){
								p->tid = i;
								p->status = sos_sched_table[i].exit_status;
								sos_sched_table[i].flags = 0;
								task_root_delete(i);
							}
							num_zombies++;
						}
					}
				}
			}
		}
	}

	if( num_zombies <= 1 ){
		SIGCHLD_DEASSERT();  //deassert sigcaught if there are no more zombies
	}

	if( p->tid > 0 ){
		return;
	}

	if ( num_children > 0 ){
		p->tid = 0; //this means there are still children waiting
	} else {
		p->tid = -1;
	}
}

void svcall_wait_child(void * args){
	CORTEXM_SVCALL_ENTER();
	//see if SIGCHLD is blocked and the status is available now
	svcall_check_for_zombie_child_t * p;
	p = (svcall_check_for_zombie_child_t*)args;

	svcall_check_for_zombie_child(args);
	if( p->tid == 0 ){
		scheulder_root_assert_stopped(task_get_current());
		scheduler_root_update_on_stopped(); //causes the currently executing thread to sleep and wait for a signal (from a child
		//scheduler_root_update_on_sleep(); //Sleep the current thread
	} //otherwise -- tid is < 0 and there are no children
}
/*! \endcond */

/*! @} */

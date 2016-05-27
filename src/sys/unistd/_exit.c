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

/*! \addtogroup UNISTD
 * @{
 */

/*! \file */

#include "config.h"
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/reent.h>
#include <errno.h>
#include <stratify/sysfs.h>

#include "mcu/mpu.h"
#include "iface/link.h"
#include "../sched/sched_flags.h"
#include "../signal/sig_local.h"

static void priv_stop_threads(int * send_signal) MCU_PRIV_EXEC_CODE;
static void priv_zombie_process(int * signal_sent) MCU_PRIV_EXEC_CODE;

int exec_options();

/*! \details This function causes the calling process
 * to exit with the specified exit code.
 *
 * \note In this version, named semaphores are not closed in this function.
 *
 * \return This function never returns
 */
void _exit(int __status){
#if SINGLE_PROCESS == 0
	int send_signal;
	int tmp;
	int i;

	// todo close named semaphores


	// todo cancel any async IO

	//Make sure all open file descriptors are closed
	for(i=0; i < OPEN_MAX; i++){
		close(i); //make sure all file descriptors are closed
	}

	//unlock any locks on the filesystems
	sysfs_unlock();

	tmp = (__status >> 8) & 0xff; //the signal number if terminated by a signal
	send_signal = __status & 0xff; //this is the 8-bit exit code
	__status = (send_signal << 8) | tmp;
	send_signal = __status;

	mcu_core_privcall((core_privcall_t)priv_stop_threads, &send_signal);

	while(1){ //if the process turns in to a zombie it should resume zombie mode if it receives a signal
		//This process will be a zombie process until it is disabled by the parent
		mcu_core_privcall((core_privcall_t)priv_zombie_process, &send_signal);
	}
#else
	while(1);
#endif
}

#if SINGLE_PROCESS == 0

void priv_stop_threads(int * send_signal){
	int i;
	int tmp;
	struct _reent * parent_reent;
	int options = exec_options();

	//set the exit status
	stfy_sched_table[task_get_current()].exit_status = *send_signal;

	//Kill all other threads in process
	tmp = task_get_pid( task_get_current() );
	//Terminate the threads in this process
	for(i=1; i < task_get_total(); i++){
		if ( task_get_pid(i) == tmp ){
			if ( i != task_get_current() ){
				stfy_sched_table[i].flags = 0;
				task_priv_del(i);
			}
		}
	}

	//now check for SA_NOCLDWAIT in parent process
	tmp = task_get_parent( task_get_current() );
	parent_reent = (struct _reent *)task_table[tmp].global_reent;

	if ( task_get_pid(tmp) == 0 ){
		//process 0 never waits
		*send_signal = false;
	} else {
		*send_signal = true;
		if( parent_reent->procmem_base->sigactions != NULL ){
			if ( parent_reent->procmem_base->sigactions[SIGCHLD] != NULL ) {
				if ( (parent_reent->procmem_base->sigactions[SIGCHLD]->sa_flags & (1<<SA_NOCLDWAIT)) ||
						parent_reent->procmem_base->sigactions[SIGCHLD]->sa_handler == SIG_IGN){
					//do not send SIGCHLD -- do not be a zombie process
					*send_signal = false;
				}
			}
		}
	}

	//ORPHAN nevers sends a signal to parent and never becomes a zombie process
	if( options & LINK_APPFS_EXEC_OPTIONS_ORPHAN ){
		*send_signal = false;
	}

	if ( sched_zombie_asserted(task_get_current()) ){
		//This will be executed if a SIGTERM is sent to a ZOMBIE process
		*send_signal = false;
	} else if( *send_signal == true ){
		//assert the zombie flags
		stfy_sched_table[task_get_current()].flags |= (1<< SCHED_TASK_FLAGS_ZOMBIE);

		//send a signal to the parent process
		tmp = task_get_pid( task_get_parent( task_get_current() ) );
		options = 0;
		for(i=0; i < task_get_total(); i++){
			//send SIGCHLD to each thread of parent
			if ( (tmp == task_get_pid(i)) && (task_enabled(i)) ){
				options = 1;
				errno = 0;
				if( signal_priv_send(
						task_get_current(),
						i,
						SIGCHLD,
						SI_USER,
						stfy_sched_table[task_get_current()].exit_status,
						0) < 0 ){
					*send_signal = false;
					return;
				}
			}
		}

		if( options == 0 ){
			//signal was never sent -- no matching parent
			*send_signal = false;
		}
	}

}

int exec_options(){
	link_appfs_file_t * hdr;
	//check to see if the app should discard itself
	hdr = (link_appfs_file_t *)mpu_addr((uint32_t)task_table[task_get_current()].mem.code.addr);
	return hdr->exec.options;
}

void priv_zombie_process(int * signal_sent){
	if ( *signal_sent == false ){
		//discard this thread immediately
		stfy_sched_table[task_get_current()].flags = 0;
		task_priv_del(task_get_current());
	} else {
		//the parent is waiting -- set this thread to a zombie thread
		sched_priv_deassert_inuse(task_get_current());
	}

	sched_priv_update_on_sleep();
}

#endif


/*! @} */

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


#include <unistd.h>
#include <errno.h>

#include "sos/sos.h"
#include "cortexm/cortexm.h"
#include "cortexm/task.h"
#include "mcu/mcu.h"
#include "../scheduler/scheduler_flags.h"

static void svcall_update_task_root(void * args){
	MCU_UNUSED_ARGUMENT(args);
	CORTEXM_SVCALL_ENTER();
	if( args != 0 &&
		 //authenticated flag must be checked after CORTEXM_SVCALL_ENTER()
		 scheduler_authenticated_asserted( task_get_current() ) ){
		task_assert_root(
					task_get_current()
					);
	} else {
		task_deassert_root(
					task_get_current()
					);
	}
}

uid_t geteuid(){
	return task_root_asserted(task_get_current())
			? SYSFS_ROOT
			: SYSFS_USER;
}

uid_t getuid(){
	return scheduler_authenticated_asserted(task_get_current())
			? SYSFS_ROOT
			: SYSFS_USER;
}

int seteuid(uid_t uid){
	if( uid == SOS_USER ){
		//if root -- switch to user
		if( geteuid() == SOS_USER_ROOT ){
			//deassert task root
			cortexm_svcall(svcall_update_task_root, 0);
		}
		return 0;
	}

	if( uid == SOS_USER_ROOT ){
		if( geteuid() == SOS_USER ){
			if( scheduler_authenticated_asserted( task_get_current() ) ){
				//assert task root
				cortexm_svcall(svcall_update_task_root, (void*)1);
				return 0;
			}
		}
		errno = EPERM;
		return -1;
	}

	//only SOS_USER and SOS_USER_ROOT are valid user id values
	errno = EINVAL;
	return -1;
}

int setuid(uid_t uid){
	return seteuid(uid);
}

//group is always 0
gid_t getgid(){ return 0; }
gid_t getegid(){ return 0; }




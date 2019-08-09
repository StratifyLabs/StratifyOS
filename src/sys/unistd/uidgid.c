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
#include "cortexm/task.h"
#include "mcu/mcu.h"

uid_t geteuid(){
	return task_root_asserted(task_get_current()) ? SOS_USER_ROOT : SOS_USER;
}

uid_t getuid(){ return geteuid(); }

int seteuid(uid_t uid){
	MCU_UNUSED_ARGUMENT(uid);
	errno = ENOTSUP;
	return -1;
}

//group is always 0
gid_t getgid(){ return 0; }
gid_t getegid(){ return 0; }




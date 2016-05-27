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


#include <unistd.h>


#include "mcu/mcu.h"


static uid_t uid MCU_SYS_MEM;
static uid_t euid MCU_SYS_MEM;
static gid_t gid MCU_SYS_MEM;
static gid_t egid MCU_SYS_MEM;

uid_t geteuid(){
	return euid;
}

uid_t getuid(){
	return uid;
}

int seteuid(uid_t uid){
	//needs to be priv'd
	return 0;
}

gid_t getgid(){
	return gid;
}

gid_t getegid(){
	return egid;
}




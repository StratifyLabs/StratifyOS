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

/*! \addtogroup time
 * @{
 */

/*! \file */

#include <sys/time.h>
#include <time.h>
#include <errno.h>

//struct itimerval itimers[SYSCALLS_NUM_ITIMERS];

int _getitimer(int which, struct itimerval *value){
	//Update the current value (time of day minus the start time)
	errno = ENOTSUP;
	return -1;
}

int _setitimer(int which, const struct itimerval * value,
	       struct itimerval * ovalue) {
	errno = ENOTSUP;
	return -1;
}

/*! @} */

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

/*! \addtogroup pthread
 * @{
 *
 */

/*! \file */

#include "config.h"

#include <pthread.h>
#include <errno.h>


/*! \details This function is not supported.
 * \return -1 with errno equal to ENOTSUP
 */
int pthread_cancel(pthread_t thread){
	errno = ENOTSUP;
	return -1;
}

/*! \details This function is not supported.
 * \return -1 with errno equal to ENOTSUP
 */
int pthread_setcancelstate(int state, int *oldstate){
	//errno = ENOTSUP;
	//return -1;
	return 0;
}

/*! \details This function is not supported.
 * \return -1 with errno equal to ENOTSUP
 */
int pthread_setcanceltype(int type, int *oldtype){
	//errno = ENOTSUP;
	//return -1;
	return 0;
}

void pthread_testcancel(){
}

/*! @} */


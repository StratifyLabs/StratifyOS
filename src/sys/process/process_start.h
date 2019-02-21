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

/*! \addtogroup SYSCALLS_PROCESS
 * @{
 */

/*! \file */

#ifndef SYSCALLS_PROCESS_H_
#define SYSCALLS_PROCESS_H_

#include "config.h"

#include <stdint.h>
#include <unistd.h>

int process_start(const char *path, char *const envp[], int options);

#endif /* SYSCALLS_PROCESS_H_ */


/*! @} */

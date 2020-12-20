// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup SYSCALLS_PROCESS
 * @{
 */

/*! \file */

#ifndef SYSCALLS_PROCESS_H_
#define SYSCALLS_PROCESS_H_

#include "config.h"

#include <stdint.h>
#include <unistd.h>

int process_start(const char *path, char *const envp[]);

#endif /* SYSCALLS_PROCESS_H_ */


/*! @} */

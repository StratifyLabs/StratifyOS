// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup UNI_PROCESS
 * @{
 */

/*! \file */

#include <errno.h>
#include <sys/types.h>

/*! \details This function is not supported.
 *
 * \return Zero on success or -1 on error with errno (see \ref errno) set to:
 *  - ENOTSUP:  function is not supported
 *
 */
pid_t fork();

pid_t _fork(){
	//This function can not be supported unless the CPU has an MMU
	errno = ENOTSUP;
	return -1;
}

/*! \details This function is not supported.
 *
 * \return Zero on success or -1 on error with errno (see \ref errno) set to:
 *  - ENOTSUP:  function is not supported
 *
 */
pid_t vfork();

pid_t _vfork(){

	//create a new process copying the characteristic of the current process (has new process ID -- all else identical)

	//manipulate the stack of the current task so that the child process ID is immediately returned


	//Force a switch to the child process -- block the parent process execution


	//return 0 (as the child)


	//when exec() or _exit() is run -- the parent process needs to be woken up


	//Use the following label as the PC for the parent
	//parent_process_resume:

	errno = ENOTSUP;
	return -1;
}

/*! @} */


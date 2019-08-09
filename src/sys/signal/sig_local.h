/* Copyright 2011-2017 Tyler Gilbert;
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

#ifndef SIG_LOCAL_H_
#define SIG_LOCAL_H_


#include <stdint.h>
#include <sys/reent.h>

#include "mcu/types.h"


#define SCOPE_PROCESS 0
#define SCOPE_THREAD 1

void signal_handler(int tid, int si_signo, int si_sigcode_si_tid, union sigval sig_value);
int signal_send(int tid, int si_signo, int si_sigcode, int sig_value);
int signal_root_send(int send_tid,
		int tid,
		int si_signo,
		int si_sigcode,
		int sig_value,
		int forward //this must be non-zero unless si_signo == SIGKILL
		) MCU_WEAK MCU_ROOT_CODE;
void signal_root_activate(int * thread) MCU_ROOT_EXEC_CODE;
void signal_svcall_wait(void * args) MCU_ROOT_EXEC_CODE;


//this is set if the signal received executes a user defined function
#define _PROC_FLAG_SIGEXEC 2

#define SIGCAUGHT_ASSERTED() (_GLOBAL_REENT->procmem_base->flags & (1<<_PROC_FLAG_SIGCAUGHT))
#define SIGCAUGHT_ASSERT() (_GLOBAL_REENT->procmem_base->flags |= (1<<_PROC_FLAG_SIGCAUGHT))
#define SIGCAUGHT_DEASSERT() (_GLOBAL_REENT->procmem_base->flags &= ~(1<<_PROC_FLAG_SIGCAUGHT))

#define SIGCHLD_ASSERTED() (_GLOBAL_REENT->procmem_base->flags & (1<<_PROC_FLAG_SIGCHLD))
#define SIGCHLD_ASSERT() (_GLOBAL_REENT->procmem_base->flags |= (1<<_PROC_FLAG_SIGCHLD))
#define SIGCHLD_DEASSERT() (_GLOBAL_REENT->procmem_base->flags &= ~(1<<_PROC_FLAG_SIGCHLD))

#define SIGEXEC_ASSERTED() (_GLOBAL_REENT->procmem_base->flags & (1<<_PROC_FLAG_SIGCHLD))
#define SIGEXEC_ASSERT() (_GLOBAL_REENT->procmem_base->flags |= (1<<_PROC_FLAG_SIGCHLD))
#define SIGEXEC_DEASSERT() (_GLOBAL_REENT->procmem_base->flags &= ~(1<<_PROC_FLAG_SIGCHLD))

#define THREAD_SIGMASK (_REENT->sigmask)
#define THREAD_SIGPENDING (_REENT->sigpending)
#define GLOBAL_SIGINFOS (_GLOBAL_REENT->procmem_base->siginfos)
#define GLOBAL_SIGINFO(signo) (((siginfo_t*)(_GLOBAL_REENT->procmem_base->siginfos))[signo])
#define GLOBAL_SIGACTIONS (_GLOBAL_REENT->procmem_base->sigactions)
#define GLOBAL_SIGACTION(signo) (_GLOBAL_REENT->procmem_base->sigactions[signo])

#endif /* SIG_LOCAL_H_ */

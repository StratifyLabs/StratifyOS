// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#ifndef SIG_LOCAL_H_
#define SIG_LOCAL_H_


#include <stdint.h>
#include <sys/reent.h>

#include <sdk/types.h>


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
void signal_svcall_wait(void *args) MCU_ROOT_CODE;

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

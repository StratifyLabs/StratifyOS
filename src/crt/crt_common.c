// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "crt_common.h"

#include <pthread.h>
#include <reent.h>
#include <signal.h>
#include <sys/lock.h>
#include <sys/reent.h>

#include "cortexm/task.h"

extern void (*_ctors)();
extern int _ctors_size;
extern void (*_dtors)();
extern int _dtors_size;
extern int main(int argc, char *const argv[]);

static void constructors();
static void destructors();

static siginfo_t signal_info[32];

int crt_common(char *path_arg, int *ret, const char *name) {
  task_setstackguard(
    pthread_self(), &_ebss + sizeof(proc_mem_t), SCHED_DEFAULT_STACKGUARD_SIZE);
  int argc;
  char **argv;

  // Zero out the BSS section
  memset(
    &_bss, 0,
    (uint32_t)((char *)&_ebss - (char *)&_bss) // cppcheck-suppress[comparePointers]
  );

  _REENT->procmem_base = (proc_mem_t *)&_ebss;
  _REENT->procmem_base->proc_name = name;
  _REENT->procmem_base->size = 0;
  _REENT->procmem_base->sigactions = NULL;
  const open_file_t init_open_file = {0};
  for (int i = 0; i < OPEN_MAX; i++) {
    _REENT->procmem_base->open_file[i] = init_open_file;
  }

  // Initialize the global mutexes
  __lock_init_recursive_global(__malloc_lock_object);
  __lock_init_global(__tz_lock_object);
  __lock_init_recursive_global(__atexit_lock);
  __lock_init_recursive_global(__sfp_lock);
  __lock_init_recursive_global(__sinit_lock);
  __lock_init_recursive_global(__env_lock_object);

  // import argv in to the process memory
  argv = crt_import_argv(path_arg, &argc);

  _REENT->procmem_base->siginfos = &signal_info;

  // Initialize STDIO
  __sinit(_GLOBAL_REENT);
  write(stdout->_file, 0, 0); // forces stdin, stdout, and stderr to open

  // Execute main
  constructors();
  *ret = main(argc, argv);
  destructors();

  return 0;
}

void constructors() {
  int i;
  for (i = 0; i < (int)&_ctors_size; i++) {
    (&_ctors)[i]();
  }
}
void destructors() {
  int i;
  for (i = 0; i < (int)&_dtors_size; i++) {
    (&_dtors)[i]();
  }
}

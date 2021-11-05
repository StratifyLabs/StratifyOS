// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <ctype.h>
#include <sos/fs/sysfs.h>
#include <stdlib.h>
#include <string.h>

#include "cortexm/mpu.h"
#include "cortexm/task.h"
#include "sos/debug.h"

typedef struct {
  int code_size;
  int data_size;
} root_load_data_t;

static void svcall_load_data(void *args) MCU_ROOT_EXEC_CODE;
void svcall_load_data(void *args) {
  CORTEXM_SVCALL_ENTER();
  root_load_data_t *p = args;


  // sanity check the size
  u32 size = sos_task_table[task_get_current()].mem.data.size;
  if (p->data_size < size) { // p->data_size is the total amount of RAM available to the
                             // application
    size = p->data_size;
  }

  u32 code_size = p->code_size;
  if (code_size > sos_task_table[task_get_current()].mem.code.size) {
    // fatal error here?
    return;
  }

  void *dest_addr = sos_task_table[task_get_current()].mem.data.address;
  void *code_addr = sos_task_table[task_get_current()].mem.code.address;
  void *src_addr = (u8 *)code_addr + code_size;

  //TODO Validate the source and dest values

  memcpy(dest_addr, src_addr, size);
}

// global_reent but it can't be removed without spinning the application signature
void crt_load_data(void *global_reent, int code_size, int data_size) {
  root_load_data_t args;
  args.code_size = code_size;
  args.data_size = data_size;
  cortexm_svcall(svcall_load_data, &args);
}

char ** crt_import_argv(char *path_arg, int *argc) {
  *argc = 0;

  if (path_arg == 0) {
    return 0;
  }

  // this needs to be strnlen -- security
  const int len = strnlen(path_arg, PATH_MAX + ARG_MAX) + 1;

  int count = 0;
  {
    char *next = NULL;
    char *p = NULL;
    next = strtok_r(path_arg, sysfs_whitespace, &p);
    while (next) {
      next = strtok_r(0, sysfs_whitespace, &p);
      count++;
    }
  }

  u32 argv_size = sizeof(char *) * (count + 1);
  char **argv = malloc(argv_size + len);
  if (argv == NULL) {
    // since we couldn't allocate memory in the application, free the memory allocated on
    // global
    if (path_arg) {
      _free_r(sos_task_table[0].global_reent, path_arg);
    }
    return 0;
  }

  char *arg_buffer = ((char *)argv) + argv_size;
  memcpy(arg_buffer, path_arg, len);
  // len includes the zero terminator -- need to zero terminate just in case
  arg_buffer[len - 1] = 0;

  // free the path_arg passed from shared system memory
  _free_r(sos_task_table[0].global_reent, path_arg);

  count = 0;
  argv[count++] = arg_buffer;
  int is_next = 0;
  for (int i = 0; i < len - 1; i++) {
    if (isspace(arg_buffer[i])) {
      arg_buffer[i] = 0;
    }
    if (arg_buffer[i] == 0) {
      is_next = 1;
    } else if (is_next && arg_buffer[i]) {
      argv[count++] = arg_buffer + i;
      is_next = 0;
    }
  }

  *argc = count;

  sos_debug_log_directive(SOS_DEBUG_MALLOC, "hist:mallocPerf");
  sos_debug_log_directive(SOS_DEBUG_MALLOC, "hist:freePerf");

  return argv;
}

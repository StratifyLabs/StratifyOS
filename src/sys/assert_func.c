// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "cortexm/cortexm.h"
#include "cortexm/task.h"
#include "sos/debug.h"
#include "sos/sos_events.h"

void __assert_func(const char *file, int line, const char *func, const char *failedexpr) {

  if (cortexm_is_root_mode() || (task_get_pid(task_get_current()) == 0)) {
    sos_debug_printf(
      "assertion \"%s\" failed: file \"%s\", line %d%s%s\n", failedexpr, file, line,
      func ? ", function: " : "", func ? func : "");
    sos_handle_event(SOS_EVENT_ASSERT, NULL);

  } else {
    fiprintf(
      stderr, "assertion \"%s\" failed: file \"%s\", line %d%s%s\n", failedexpr, file,
      line, func ? ", function: " : "", func ? func : "");
    abort();
  }
}

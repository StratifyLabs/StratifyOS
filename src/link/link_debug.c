// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "link_local.h"

int link_debug_level = 0;

void link_set_debug(int debug_level) { link_debug_level = debug_level; }

int link_debug_printf(int x, const char *function, int line, const char *fmt, ...) {

  if (x <= link_debug_level) {

    switch (x) {
    case LINK_DEBUG_FATAL:
      printf("FATAL:");
      break;
    case LINK_DEBUG_CRITICAL:
      printf("CRITICAL:");
      break;
    case LINK_DEBUG_WARNING:
      printf("WARNING:");
      break;
    case LINK_DEBUG_INFO:
      printf("INFO:");
      break;
    case LINK_DEBUG_MESSAGE:
      printf("MESSAGE:");
      break;
    case LINK_DEBUG_DEBUG:
      printf("DEBUG:");
      break;
    }

    printf("%s():%d -> ", function, line);

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
    fflush(stdout);

    return 1;
  }

  return 0;
}

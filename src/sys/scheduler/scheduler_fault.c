// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "scheduler_local.h"
#include "sos/sos.h"
#include <string.h>

void scheduler_fault_build_memory_string(char *dest, const char *term) {
  char hex_buffer[9];
  strcpy(dest, "STACK:B");
  htoa(hex_buffer, (u32)sos_task_table[task_get_current()].sp);
  strcat(dest, hex_buffer);

  strcat(dest, ":C");
  htoa(hex_buffer, (u32)sos_sched_table[task_get_current()].attr.stackaddr);
  strcat(dest, hex_buffer);

  strcat(dest, ":S");
  htoa(hex_buffer, (u32)sos_sched_table[task_get_current()].attr.stacksize);
  strcat(dest, hex_buffer);
  if (term) {
    strcat(dest, term);
  }
}

void scheduler_fault_build_trace_string(char *dest) {
  char hex_buffer[9];
  strcpy(dest, "F");
  htoa(hex_buffer, m_scheduler_fault.fault.num);
  strcat(dest, hex_buffer);
  strcat(dest, ":");
  htoa(hex_buffer, (u32)m_scheduler_fault.fault.addr);
  strcat(dest, hex_buffer);
}

void scheduler_fault_build_string(char *dest, const char *term) {
  char hex_buffer[9];
  strcpy(dest, "I");
  htoa(hex_buffer, m_scheduler_fault.tid);
  strcat(dest, hex_buffer);

  strcat(dest, ":F");
  htoa(hex_buffer, m_scheduler_fault.fault.num);
  strcat(dest, hex_buffer);

  strcat(dest, ":A");
  htoa(hex_buffer, (u32)m_scheduler_fault.fault.addr);
  strcat(dest, hex_buffer);

  strcat(dest, ":PC");
  htoa(hex_buffer, (u32)m_scheduler_fault.fault.pc);
  strcat(dest, hex_buffer);

  strcat(dest, ":C");
  htoa(hex_buffer, (u32)m_scheduler_fault.fault.caller);
  strcat(dest, hex_buffer);

  strcat(dest, ":HA");
  htoa(hex_buffer, (u32)m_scheduler_fault.fault.handler_pc);
  strcat(dest, hex_buffer);

  strcat(dest, ":HC");
  htoa(hex_buffer, (u32)m_scheduler_fault.fault.handler_caller);
  strcat(dest, hex_buffer);

  if (term) {
    strcat(dest, term);
  }
}

void htoa(char *dest, int num) {
  int i;
  for (i = 0; i < 8; i++) {
    char nibble;
    nibble = num & 0xF;
    num >>= 4;
    dest[7 - i] = htoc(nibble);
  }
  dest[8] = 0;
}

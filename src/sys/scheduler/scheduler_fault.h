// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#ifndef SCHEDULER_SCHEDULER_FAULT_H_
#define SCHEDULER_SCHEDULER_FAULT_H_

#include "scheduler_local.h"

void scheduler_fault_build_memory_string(char * dest, const char * term);
void scheduler_fault_build_string(char * dest, const char * term);
void scheduler_fault_build_trace_string(char *dest);

int scheduler_root_fault_handler(void *context, const mcu_event_t *data);

#endif /* SCHEDULER_SCHEDULER_FAULT_H_ */

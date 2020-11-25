/* Copyright 2011-2018 Tyler Gilbert;
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

#include <errno.h>

#include "../scheduler/scheduler_timing.h"
#include "cortexm/cortexm.h"
#include "cortexm/mpu.h"
#include "cortexm/task.h"
#include "mcu/core.h"
#include "sos/link.h"
#include "sos/link/transport_usb.h"
#include "sos/sos.h"

#define PRINT_DEBUG 0

extern void task_restore();

static void sos_trace_event_addr(
  link_trace_event_id_t event_id,
  const void *data_ptr,
  size_t data_len,
  u32 addr);

static void sos_trace_build_event(
  link_trace_event_t *event,
  link_trace_event_id_t event_id,
  const void *data_ptr,
  size_t data_len,
  u32 addr,
  int tid,
  const struct timespec *spec);
static void svcall_trace_event(void *args);
static void svcall_get_stack_pointer(void *args);

static u16 *
scan_code_for_push(u32 link_address, u32 *stack_jump, int *link_register_offset);

static u32 decode_push_opcode(u16 *opcode);
static u32 decode_push_with_lr_opcode(u16 *opcode);
static u32 decode_store_on_stack_opcode(u16 *opcode);
static u32 decode_subtract_stack_opcode(u16 *opcode);

void sos_trace_root_trace_event(
  link_trace_event_id_t event_id,
  const void *data_ptr,
  size_t data_len) {
  register u32 lr asm("lr");
  link_trace_event_t event;
  if (sos_board_config.trace_event) {
    sos_trace_build_event(
      &event, event_id, data_ptr, data_len, lr, task_get_current(), 0);
    sos_board_config.trace_event(&event);
  }
}

void sos_trace_build_event(
  link_trace_event_t *event,
  link_trace_event_id_t event_id,
  const void *data_ptr,
  size_t data_len,
  u32 addr,
  int tid,
  const struct timespec *spec) {
  event->header.size = sizeof(link_trace_event_t);
  event->header.id = LINK_NOTIFY_ID_POSIX_TRACE_EVENT;
  event->posix_trace_event.posix_event_id = event_id;
  event->posix_trace_event.posix_pid = task_get_pid(tid);
  event->posix_trace_event.posix_prog_address = addr; // grab the value of the caller
  event->posix_trace_event.posix_thread_id = tid;

  if (data_len > LINK_POSIX_TRACE_DATA_SIZE - 1) {
    data_len = LINK_POSIX_TRACE_DATA_SIZE - 1;
    event->posix_trace_event.posix_truncation_status = 1;
  } else {
    event->posix_trace_event.posix_truncation_status = 0;
  }

  memset(event->posix_trace_event.data, 0, LINK_POSIX_TRACE_DATA_SIZE);
  memcpy(event->posix_trace_event.data, data_ptr, data_len);
  if (spec) {
    event->posix_trace_event.posix_timestamp_tv_sec = spec->tv_sec;
    event->posix_trace_event.posix_timestamp_tv_nsec = spec->tv_nsec;
  } else {
    event->posix_trace_event.posix_timestamp_tv_sec = 0;
    event->posix_trace_event.posix_timestamp_tv_nsec = 0;
  }
  cortexm_assign_zero_sum32(&event, CORTEXM_ZERO_SUM32_COUNT(event));
}

void sos_trace_event(
  link_trace_event_id_t event_id,
  const void *data_ptr,
  size_t data_len) {
  register u32 lr asm("lr");
  sos_trace_event_addr(event_id, data_ptr, data_len, lr);
}

void sos_trace_event_addr(
  link_trace_event_id_t event_id,
  const void *data_ptr,
  size_t data_len,
  u32 addr) {
  sos_trace_event_addr_tid(event_id, data_ptr, data_len, addr, task_get_current());
}

void svcall_trace_event(void *args) {
  CORTEXM_SVCALL_ENTER();
  sos_board_config.trace_event(args);
}

void sos_trace_event_addr_tid(
  link_trace_event_id_t event_id,
  const void *data_ptr,
  size_t data_len,
  u32 addr,
  int tid) {
  // record event id and in-calling processes trace stream

  if (sos_board_config.trace_event) {
    // convert the address using the task memory location
    // check if addr is part of kernel or app
    if (
      ((addr >= (u32)&_text) && (addr < (u32)&_etext))
      || ((addr > (uint32_t)&_tcim) && (addr < (uint32_t)&_etcim)) || (addr == 1)) {
      // kernel
      addr = addr - 1;
    } else {
      // app
      addr = addr - (u32)sos_task_table[tid].mem.code.address - 1 + 0xDE000000;
    }

    link_trace_event_t event;
    struct timespec spec;

    if (cortexm_is_root_mode()) {
      struct mcu_timeval sched_time = {0};
      scheduler_timing_svcall_get_realtime(&sched_time);
      spec.tv_sec =
        sched_time.tv_sec * SCHEDULER_TIMEVAL_SECONDS + sched_time.tv_usec / 1000000UL;
      spec.tv_nsec = (sched_time.tv_usec % 1000000UL) * 1000UL;
    } else {
      clock_gettime(CLOCK_REALTIME, &spec);
    }

    sos_trace_build_event(&event, event_id, data_ptr, data_len, addr, tid, &spec);

    if (cortexm_is_root_mode()) {
      svcall_trace_event(&event);
    } else {
      cortexm_svcall(svcall_trace_event, &event);
    }
  }
}

void svcall_get_stack_pointer(void *args) {
  CORTEXM_SVCALL_ENTER();
  cortexm_get_thread_stack_ptr(args);
}

u32 lookup_caller_address(u32 input) {

  if ((input & 0x01) == 0) {
    return 0; // don't trace non-callers
  }

  // check if input is a caller for the kernel
  if ((input >= (u32)&_text) && (input < (u32)&_etext)) {
    return input;
  }

  // check if input is a caller for the application
  const u8 tid = task_get_current();
  if (
    (input >= (u32)sos_task_table[tid].mem.code.address)
    && (input < (u32)sos_task_table[tid].mem.code.address + sos_task_table[tid].mem.code.size)) {
    return input;
  }
  return 0;
}

/*
 * -mpoke-function-name adds the function name to the text section -- prevents need to
 * parse elf file -fomit-frame-pointer is used and makes it more difficult to trace back
 * the stack -fno-omit-frame-pointer causes StratifyOS to crash, not sure why
 *
 * if there is a
 *
 * push {lr}
 * sub sp
 *
 * then the stack jump won't point at the
 *
 * gcc "throw" functions use no return and then destory the stack
 *
 * there may be a way to override the throw functions with just abort()
 * then the stack trace will zero in
 *
 * namespace std
 * {
 *
 * void __throw_out_of_range(char const*)
 * {
 * 	abort();
 * }
 *
 * }
 *
 * arm-none-eabi-objcopy can be used to remove symbols (search for gcc-hard for an
 * example) -W makes the symbols weak -- use -W _ZSt24__throw_out_of_range_fmtPKcz
 *
 */

int sos_trace_stack(u32 count) {

  const u32 *sp;
  cortexm_svcall(svcall_get_stack_pointer, &sp);

  /*
   * cortexm_svcall() reads the PSP in handler mode. So
   * the PSP has a hardware stack from on top of it when it
   * is calculated. This needs to be adjusted for.
   *
   * The hardware stack frame has 8 registers
   *
   */
  CORTEXM_DECLARE_LINK_REGISTER(first_link_register);

  u32 stack_top = (u32)(
    sos_task_table[task_get_current()].mem.data.address
    + sos_task_table[task_get_current()].mem.data.size);

  sos_trace_stack_with_pointer(
    (const u32 *)first_link_register, sp, (const u32 *)stack_top, count);
}

int sos_trace_stack_with_pointer(
  const u32 *first_link_register,
  const u32 *sp,
  const u32 *stack_top,
  u32 count) {

  char message[20] = {0};
  int len;
  strncpy(message, "stackTrace", 16);
  len = strnlen(message, 16);

  // adjust for HW stack frame pointer
  sp += 8;

  u32 next_link_register = ((u32)first_link_register) & ~0x01;

#if PRINT_DEBUG
  mcu_debug_printf("start trace from %p\n", first_link_register);
#endif

  u32 push_count = 0;
  u32 stack_jump = 0;
  int link_register_offset = 0;
  do {
    u16 *code_pointer =
      scan_code_for_push(next_link_register, &stack_jump, &link_register_offset);

    // code points near the entrance of the function

#if PRINT_DEBUG & 0
    int preview = stack_jump;
    if (preview < 8) {
      preview += 8;
    }
    for (int i = 0; i < preview; i++) {
      mcu_debug_printf("stack preview %d:%x -> %08x\n", i, sp + i, sp[i]);
    }
#endif

    sp += (stack_jump);
#if PRINT_DEBUG
    mcu_debug_printf(
      "Pushed %p < %p registers %d jump: %d\n", sp, stack_top, link_register_offset,
      stack_jump);
#endif

    next_link_register = sp[-1] & ~0x01;
#if PRINT_DEBUG
    mcu_debug_printf("Next link is %08x\n", next_link_register);
#endif
    if (next_link_register == (((u32)task_restore) & ~0x01)) {
#if PRINT_DEBUG
      mcu_debug_printf("task restore operation\n");
#endif
      for (int i = 0; i < 16; i++) {
#if PRINT_DEBUG
        mcu_debug_printf("--stack preview %d:%x -> %08x\n", i, sp + i, sp[i]);
#endif
      }
      // special treatment -- doesn't return HW stack is inserted
      sp += (16);
      next_link_register = sp[-3] & ~0x01;
    }

    if (push_count) { // first one is always sos_stack_trace()
      sos_trace_event_addr(
        LINK_POSIX_TRACE_MESSAGE, message, len, ((u32)code_pointer) + 1);
    }

    push_count++;

    if (lookup_caller_address(next_link_register + 1) == 0) {
#if PRINT_DEBUG
      mcu_debug_printf("failed to fully trace stack %p\n", next_link_register);
#endif
      sos_trace_event_addr(
        LINK_POSIX_TRACE_ERROR, "traceFailed", sizeof("traceFailed"), next_link_register);
      return push_count;
    }

  } while ((sp < stack_top) && (push_count < count));

#if PRINT_DEBUG
  mcu_debug_printf("is end %p < %p\n", sp, stack_top);
#endif

  return push_count;
}

u16 *scan_code_for_push(
  u32 link_address,
  u32 *stack_jump_result,
  int *link_register_offset) {
  u16 *code_pointer = (u16 *)link_address;

  /*
   * push is encoded as b5xx: xx is the register mask -- can be a 16-bit instruction
   * stmdb is encoded as e92d xxxx: xxxx is the register mask
   * subtract from sp as 0xb08x: x is subtraction / 4
   *
   *
   */

  u32 jump;
  int lr_jump = 0;
  u32 stack_jump = 0;
  u32 result = 0;
  do {

    jump = decode_subtract_stack_opcode(code_pointer);
    if (jump) {
      stack_jump += jump;
      lr_jump += jump;
      result = 0;
    }

    jump = decode_store_on_stack_opcode(code_pointer);
    if (jump) {
      stack_jump += jump;
      lr_jump += jump;
      result = 1;
    }

    jump = decode_push_with_lr_opcode(code_pointer);
    if (jump) {
      stack_jump += jump;
      lr_jump += jump;
      result = 1;
      jump = decode_push_opcode(code_pointer - 1);
      if (jump > 0) {
        code_pointer--;
        stack_jump += jump;
      }
    }

    if (result == 0) {
      // look to see if a 32-bit instruction is coming up -- if so, an extra decrement is
      // needed
      if ((*(code_pointer - 2) & 0xe000) == 0xe000) {
        code_pointer--;
      }
      code_pointer--;
    }

  } while ((result == 0) && (code_pointer != 0));

  *stack_jump_result = stack_jump;
  *link_register_offset = stack_jump - lr_jump;

  return code_pointer;
}

u32 decode_push_with_lr_opcode(u16 *opcode) {
  u32 count = 0;
  if ((*opcode & 0xff00) == 0xb500) {
    u8 register_mask = *opcode & 0xff;
    count++;
    for (u32 i = 0; i < 8; i++) {
      if ((1 << i) & register_mask) {
        count++;
      }
    }
  }
  return count;
}

u32 decode_push_opcode(u16 *opcode) {
  u32 count = 0;
  if ((*opcode & 0xff00) == 0xb400) {
    u8 register_mask = *opcode & 0xff;
    for (u32 i = 0; i < 8; i++) {
      if ((1 << i) & register_mask) {
        count++;
      }
    }
  }
  return count;
}

u32 decode_store_on_stack_opcode(u16 *opcode) {
  u32 count = 0;
  if (*opcode == 0xe92d) {
    u16 register_mask = *(opcode + 1);
    for (u32 i = 0; i < 16; i++) {
      if ((1 << i) & register_mask) {
        count++;
      }
    }
  }
  return count;
}

u32 decode_subtract_stack_opcode(u16 *opcode) {
  if ((*opcode & 0xff80) == 0xb080) {
    return *opcode & 0x007f;
  }
  return 0;
}

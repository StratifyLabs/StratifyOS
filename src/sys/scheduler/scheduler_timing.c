// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "config.h"

#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#include "../signal/sig_local.h"
#include "scheduler_local.h"

#include "sos/debug.h"
#include "mcu/rtc.h"
#include "mcu/tmr.h"

static volatile u32 sched_usecond_counter MCU_SYS_MEM;

static void svcall_allocate_timer(void *args) MCU_ROOT_EXEC_CODE;
static void root_allocate_timer(void *args) MCU_ROOT_EXEC_CODE;
static int root_handle_usecond_overflow_event(void *context, const mcu_event_t *data)
  MCU_ROOT_EXEC_CODE;
static int root_handle_usecond_match_event(void *context, const mcu_event_t *data)
  MCU_ROOT_EXEC_CODE;
static int root_handle_usecond_process_timer_match_event(
  void *context,
  const mcu_event_t *data) MCU_ROOT_EXEC_CODE;
static int send_and_reload_timer(volatile sos_process_timer_t *timer, u8 task_id, u32 now)
  MCU_ROOT_EXEC_CODE;
static inline u8 scheduler_timing_process_timer_task_id(timer_t timer_id) {
  return timer_id >> 8;
}
static inline u8 scheduler_timing_process_timer_id_offset(timer_t timer_id) {
  return timer_id & 0xFF;
}
static inline u8 scheduler_timing_process_timer_count() {
  return SOS_PROCESS_TIMER_COUNT;
}
static void update_tmr_for_process_timer_match(volatile sos_process_timer_t *timer)
  MCU_ROOT_EXEC_CODE;

u64 scheduler_timing_real64usec(struct mcu_timeval *tv) {
  return tv->tv_usec + (u64)tv->tv_sec * (u64)SOS_USECOND_PERIOD;
}

void scheduler_timing_init() {
  sos_config.clock.initialize(
    root_handle_usecond_match_event, root_handle_usecond_process_timer_match_event,
    root_handle_usecond_overflow_event);
}

volatile sos_process_timer_t *scheduler_timing_process_timer(timer_t timer_id) {
  u8 task_id = scheduler_timing_process_timer_task_id(timer_id);
  u8 id_offset = scheduler_timing_process_timer_id_offset(timer_id);

  if (
    (task_id < task_get_total())
    && (id_offset < scheduler_timing_process_timer_count())) {
    return sos_sched_table[task_id].timer + id_offset;
  }
  return 0;
}

typedef struct {
  timer_t timer_id;
  const struct sigevent *event;
  int result;
} svcall_allocate_timer_t;

void svcall_allocate_timer(void *args) {
  CORTEXM_SVCALL_ENTER();
  root_allocate_timer(args);
}

void root_allocate_timer(void *args) {
  svcall_allocate_timer_t *p = args;
  volatile sos_process_timer_t *timer = scheduler_timing_process_timer(p->timer_id);
  if (timer->o_flags == 0) {
    if (p->event) {
      memcpy((void *)&timer->sigevent, p->event, sizeof(struct sigevent));
    } else {
      memset((void *)&timer->sigevent, 0, sizeof(struct sigevent));
      timer->sigevent.sigev_notify = SIGEV_SIGNAL;
      timer->sigevent.sigev_signo = SIGALRM;
      timer->sigevent.sigev_value.sival_int = p->timer_id;
    }
    timer->value.tv_sec = SCHEDULER_TIMEVAL_SEC_INVALID;
    timer->value.tv_usec = 0;
    timer->interval.tv_sec = 0;
    timer->interval.tv_usec = 0;
    timer->o_flags = SCHEDULER_TIMING_PROCESS_TIMER_FLAG_IS_INITIALIZED;

    cortexm_assign_zero_sum32((void *)timer, sizeof(sos_process_timer_t) / sizeof(u32));
    p->result = 0;
  } else {
    p->result = -1;
  }
}

void scheduler_timing_root_process_timer_initialize(u16 task_id) {
  memset(
    (void *)sos_sched_table[task_id].timer, 0,
    sizeof(sos_process_timer_t) * SOS_PROCESS_TIMER_COUNT);

  // the first available timer slot is reserved for alarm/ualarm
  if (task_get_parent(task_id) == task_id) {
    svcall_allocate_timer_t args;
    args.timer_id = SCHEDULER_TIMING_PROCESS_TIMER(task_id, 0);
    args.event = 0;
    root_allocate_timer(&args);
  }
}

timer_t scheduler_timing_process_create_timer(const struct sigevent *evp) {
  s32 count = scheduler_timing_process_timer_count();
  int pid = task_get_pid(task_get_current());
  svcall_allocate_timer_t args;
  args.event = evp;
  args.result = -1;
  for (u8 task_id = 0; task_id < task_get_total(); task_id++) {
    if (task_get_pid(task_id) == pid) {
      for (u8 id_offset = 0; id_offset < count; id_offset++) {
        args.timer_id = SCHEDULER_TIMING_PROCESS_TIMER(task_id, id_offset);
        cortexm_svcall(svcall_allocate_timer, &args);
        if (args.result == 0) {
          return args.timer_id;
        }
      }
    }
  }
  return (timer_t)(-1);
}

typedef struct {
  timer_t timer_id;
  int result;
} svcall_delete_timer_t;

static void svcall_delete_timer(void *args) MCU_ROOT_EXEC_CODE;
void svcall_delete_timer(void *args) {
  CORTEXM_SVCALL_ENTER();
  svcall_delete_timer_t *p = args;
  volatile sos_process_timer_t *timer = scheduler_timing_process_timer(p->timer_id);
  if (timer == 0) {
    p->result = -1;
    return;
  }

  memset((void *)timer, 0, sizeof(sos_process_timer_t));
  cortexm_assign_zero_sum32((void *)timer, sizeof(sos_process_timer_t) / sizeof(u32));
  p->result = 0;
}

int scheduler_timing_process_delete_timer(timer_t timer_id) {
  svcall_delete_timer_t args;
  args.timer_id = timer_id;
  args.result = -202020;
  cortexm_svcall(svcall_delete_timer, &args);
  return args.result;
}

typedef struct {
  timer_t timer_id;
  int flags;
  const struct mcu_timeval *value;
  const struct mcu_timeval *interval;
  struct mcu_timeval *o_value;
  struct mcu_timeval *o_interval;
  int result;
} svcall_settime_t;

static void svcall_settime(void *args) MCU_ROOT_EXEC_CODE;
void svcall_settime(void *args) {
  CORTEXM_SVCALL_ENTER();
  svcall_settime_t *p = args;
  struct mcu_timeval abs_time;

  volatile sos_process_timer_t *timer = scheduler_timing_process_timer(p->timer_id);
  if (timer == 0) {
    errno = EINVAL;
    p->result = -1;
    return;
  }

  scheduler_timing_root_get_realtime(&abs_time);

  if (timer->value.tv_sec == SCHEDULER_TIMEVAL_SEC_INVALID) {
    p->o_value->tv_sec = 0;
    p->o_value->tv_usec = 0;
  } else {
    // ovalue is the amount of time before the timer would have expired timer->timeout -
    // current_time
    *p->o_value = scheduler_timing_subtract_mcu_timeval(
      (struct mcu_timeval *)&timer->value, &abs_time);
  }

  if ((p->flags & TIMER_ABSTIME) == 0) {
    // value is a relative time -- convert to absolute time
    timer->value = scheduler_timing_add_mcu_timeval(&abs_time, p->value);
  } else {
    timer->value = *p->value;
  }

  // update the interval output value
  *p->o_interval = timer->interval;
  timer->interval = *p->interval;
  if (
    (timer->interval.tv_sec == 0) && (timer->interval.tv_usec > 0)
    && (timer->interval.tv_usec < SCHED_USECOND_TMR_MINIMUM_PROCESS_TIMER_INTERVAL)) {
    timer->interval.tv_usec = SCHED_USECOND_TMR_MINIMUM_PROCESS_TIMER_INTERVAL;
  }

  // stop the timer -- see if event is in past, assign the values, start the timer
  update_tmr_for_process_timer_match(timer);

  cortexm_assign_zero_sum32((void *)timer, sizeof(sos_process_timer_t) / sizeof(u32));
  p->result = 0;
}

int scheduler_timing_process_set_timer(
  timer_t timerid,
  int flags,
  const struct mcu_timeval *value,
  const struct mcu_timeval *interval,
  struct mcu_timeval *o_value,
  struct mcu_timeval *o_interval) {
  svcall_settime_t args;
  args.timer_id = timerid;
  args.flags = flags;
  args.value = value;
  args.interval = interval;
  args.o_value = o_value;
  args.o_interval = o_interval;
  args.result = -202020;
  cortexm_svcall(svcall_settime, &args);
  return args.result;
}

typedef struct {
  timer_t timer_id;
  struct mcu_timeval *value;
  struct mcu_timeval *interval;
  struct mcu_timeval *now;
  int result;
} svcall_gettime_t;

static void svcall_gettime(void *args) MCU_ROOT_EXEC_CODE;
void svcall_gettime(void *args) {
  CORTEXM_SVCALL_ENTER();
  svcall_gettime_t *p = args;

  volatile sos_process_timer_t *timer = scheduler_timing_process_timer(p->timer_id);
  if (timer == 0) {
    p->result = -1;
    return;
  }

  scheduler_timing_root_get_realtime(p->now);
  if (timer->value.tv_sec == SCHEDULER_TIMEVAL_SEC_INVALID) {
    p->value->tv_sec = 0;
    p->value->tv_usec = 0;
  } else {
    // value is absolute time but gettime want's relative time
    *(p->value) =
      scheduler_timing_subtract_mcu_timeval((struct mcu_timeval *)&timer->value, p->now);
  }
  *p->interval = timer->interval;
  p->result = 0;
}

int scheduler_timing_process_get_timer(
  timer_t timerid,
  struct mcu_timeval *value,
  struct mcu_timeval *interval,
  struct mcu_timeval *now) {
  svcall_gettime_t args;
  args.timer_id = timerid;
  args.result = -202020;
  args.value = value;
  args.interval = interval;
  args.now = now;

  cortexm_svcall(svcall_gettime, &args);
  return args.result;
}

u32 scheduler_timing_useconds_to_clocks(int useconds) {
  return (u32)((sos_config.clock.frequency / 1000000UL) * useconds);
}

void update_tmr_for_process_timer_match(volatile sos_process_timer_t *timer) {
  mcu_channel_t chan_req;
  u32 now;

  // Initialization

  if (timer->value.tv_sec >= sched_usecond_counter) {
    int is_time_to_send = 1;

    if (timer->value.tv_sec == sched_usecond_counter) {

      now = sos_config.clock.disable();
      // See if abs_time is in the past

      // Read the current OC value to see if it needs to be updated
      chan_req.loc = SCHED_USECOND_TMR_SYSTEM_TIMER_OC;

      sos_config.clock.get_channel(&chan_req);
      if (timer->value.tv_usec < chan_req.value) {
        // this means the signal needs to happen sooner than currently set
        chan_req.value = timer->value.tv_usec;
      }

      // needs to be enough in the future to allow the
      // OC to be set before the timer passes it
      if ((timer->value.tv_usec > now) && (chan_req.value == timer->value.tv_usec)) {
        sos_config.clock.set_channel(&chan_req);
        is_time_to_send = 0;
      }
    }

    sos_config.clock.enable();

    if (is_time_to_send) {
      // send it now and reload if needed
      send_and_reload_timer(timer, task_get_current(), now);

      // if interval is non-zero -- this needs to be called again
      if (timer->interval.tv_sec + timer->interval.tv_usec) {
        update_tmr_for_process_timer_match(timer);
      }
    }
  }
}

typedef struct {
  int si_signo;
  int sig_value;
} svcall_unqueue_timer_t;

static void svcall_unqueue_timer(void *args) {
  CORTEXM_SVCALL_ENTER();
  svcall_unqueue_timer_t *p = args;
  timer_t timer_id;
  for (u8 j = 0; j < SOS_PROCESS_TIMER_COUNT; j++) {
    timer_id = SCHEDULER_TIMING_PROCESS_TIMER(task_get_current(), j);
    volatile sos_process_timer_t *timer = scheduler_timing_process_timer(timer_id);
    if (
      (timer->sigevent.sigev_signo == p->si_signo)
      && (timer->sigevent.sigev_value.sival_int == p->sig_value)) {
      // unqueue this timer
      timer->o_flags &= ~SCHEDULER_TIMING_PROCESS_TIMER_FLAG_IS_QUEUED;
    }
  }
}

void scheduler_timing_process_unqueue_timer(
  int tid,
  int si_signo,
  union sigval sig_value) {
  svcall_unqueue_timer_t args;
  args.si_signo = si_signo;
  args.sig_value = sig_value.sival_int;
  cortexm_svcall(svcall_unqueue_timer, &args);
}

int send_and_reload_timer(volatile sos_process_timer_t *timer, u8 task_id, u32 now) {

  // check to see if a signal has already been queued
  if (
    ((timer->o_flags & SCHEDULER_TIMING_PROCESS_TIMER_FLAG_IS_QUEUED) == 0)
    && (timer->sigevent.sigev_notify == SIGEV_SIGNAL)) {
    int result = signal_root_send(
      0, task_id, timer->sigevent.sigev_signo, SI_TIMER,
      timer->sigevent.sigev_value.sival_int, task_get_current() == task_id);
    if (result < 0) {
      sos_debug_log_error(
        SOS_DEBUG_SCHEDULER, "failed to fire %d", SYSFS_GET_RETURN(result));
    } else {
      timer->o_flags |= SCHEDULER_TIMING_PROCESS_TIMER_FLAG_IS_QUEUED;
    }
  }

  // reload the timer if interval is valid
  if (timer->interval.tv_sec + timer->interval.tv_usec) {
    struct mcu_timeval current;
    current.tv_sec = sched_usecond_counter;
    current.tv_usec = now;
    timer->value =
      scheduler_timing_add_mcu_timeval(&current, (struct mcu_timeval *)&timer->interval);
  } else {
    timer->value.tv_sec = SCHEDULER_TIMEVAL_SEC_INVALID;
  }
  return 0;
}

void scheduler_timing_root_timedblock(void *block_object, struct mcu_timeval *abs_time) {
  int id;
  mcu_channel_t chan_req;
  u32 now;
  int is_time_to_sleep;

  // Initialization
  id = task_get_current();
  sos_sched_table[id].block_object = block_object;
  is_time_to_sleep = 0;

  if (abs_time->tv_sec >= sched_usecond_counter) {

    sos_sched_table[id].wake.tv_sec = abs_time->tv_sec;
    sos_sched_table[id].wake.tv_usec = abs_time->tv_usec;

    if (abs_time->tv_sec == sched_usecond_counter) {

      now = sos_config.clock.disable();

      // Read the current OC value to see if it needs to be updated
      chan_req.loc = SCHED_USECOND_TMR_SLEEP_OC;

      sos_config.clock.get_channel(&chan_req);
      if (abs_time->tv_usec < chan_req.value) { // this means the interrupt needs to
                                                // happen sooner than currently set
        chan_req.value = abs_time->tv_usec;
      }

      // Is it necessary to look ahead since the timer is stopped? -- Issue #62
      if (abs_time->tv_usec > now) { // needs to be enough in the future to allow the OC
                                     // to be set before the timer passes it
        if (chan_req.value == abs_time->tv_usec) {
          sos_config.clock.set_channel(&chan_req);
        }
        is_time_to_sleep = 1;
      }

      sos_config.clock.enable();

    } else {
      is_time_to_sleep = 1;
    }
  }

  // only sleep if the time hasn't already passed
  if (is_time_to_sleep) {
    scheduler_root_update_on_sleep();
  }
}

void scheduler_timing_convert_timespec(
  struct mcu_timeval *tv,
  const struct timespec *ts) {
  if (ts == NULL) {
    tv->tv_sec = SCHEDULER_TIMEVAL_SEC_INVALID;
    tv->tv_usec = 0;
  } else {
    div_t d = div(ts->tv_sec, SOS_SCHEDULER_TIMEVAL_SECONDS);
    tv->tv_sec = d.quot;
    tv->tv_usec = d.rem * 1000000UL + (ts->tv_nsec + 999UL) / 1000UL;
  }
}

void scheduler_timing_convert_mcu_timeval(
  struct timespec *ts,
  const struct mcu_timeval *mcu_tv) {
  div_t d;
  d = div(mcu_tv->tv_usec, 1000000UL);
  ts->tv_sec = mcu_tv->tv_sec * SCHEDULER_TIMEVAL_SECONDS + d.quot;
  ts->tv_nsec = d.rem * 1000UL;
}

// never used
u32 scheduler_timing_get_realtime() {
  struct mcu_timeval tv;
  cortexm_svcall((cortexm_svcall_t)scheduler_timing_svcall_get_realtime, &tv);
  return tv.tv_usec;
}

struct mcu_timeval scheduler_timing_add_mcu_timeval(
  const struct mcu_timeval *a,
  const struct mcu_timeval *b) {
  struct mcu_timeval result;
  result.tv_sec = a->tv_sec + b->tv_sec;
  result.tv_usec = a->tv_usec + b->tv_usec;
  if (result.tv_usec > SOS_USECOND_PERIOD) {
    result.tv_sec++;
    result.tv_usec -= SOS_USECOND_PERIOD;
  }
  return result;
}

struct mcu_timeval scheduler_timing_subtract_mcu_timeval(
  const struct mcu_timeval *a,
  const struct mcu_timeval *b) {
  struct mcu_timeval result;
  result.tv_sec = 0;
  result.tv_usec = 0;
  if (a->tv_sec > b->tv_sec) {
    result.tv_sec = a->tv_sec - b->tv_sec;
    if (a->tv_usec >= b->tv_usec) {
      result.tv_usec = a->tv_usec - b->tv_usec;
    } else {
      result.tv_sec--;
      result.tv_usec = SOS_USECOND_PERIOD - (b->tv_usec - a->tv_usec);
    }
  } else if (a->tv_sec == b->tv_sec) {
    // tv_sec is already 0
    if (a->tv_usec >= b->tv_usec) {
      result.tv_usec = a->tv_usec - b->tv_usec;
    }
  }

  return result;
}

void scheduler_timing_svcall_get_realtime(void *args) {
  CORTEXM_SVCALL_ENTER();
  scheduler_timing_root_get_realtime(args);
}

void scheduler_timing_root_get_realtime(struct mcu_timeval *tv) {
  tv->tv_usec = sos_config.clock.disable();
  tv->tv_sec = sched_usecond_counter;
  sos_config.clock.enable();
}

int root_handle_usecond_overflow_event(void *context, const mcu_event_t *data) {
  sched_usecond_counter++;
  root_handle_usecond_match_event(0, 0);
  root_handle_usecond_process_timer_match_event(0, 0);
  sos_config.clock.enable();
  return 1; // do not clear callback
}

int root_handle_usecond_match_event(void *context, const mcu_event_t *data) {
  int i;
  u32 next;
  u32 tmp;
  int new_priority;
  mcu_channel_t chan_req;
  u32 now;

  // Initialize variables
  chan_req.loc = SCHED_USECOND_TMR_SLEEP_OC;
  chan_req.value = SOS_USECOND_PERIOD + 1;
  new_priority = SCHED_LOWEST_PRIORITY - 1;
  next = SOS_USECOND_PERIOD;

  now = sos_config.clock.disable();

  for (i = 1; i < task_get_total(); i++) {

    if (task_enabled_not_active(i)) {
      tmp = sos_sched_table[i].wake.tv_usec;

      // compare the current clock to the wake time
      if (
        (sos_sched_table[i].wake.tv_sec < sched_usecond_counter)
        || ((sos_sched_table[i].wake.tv_sec == sched_usecond_counter) && (tmp <= now))) {
        // wake this task
        scheduler_root_assert_active(i, SCHEDULER_UNBLOCK_SLEEP);
        if (!task_stopped_asserted(i) && (scheduler_priority(i) > new_priority)) {
          new_priority = scheduler_priority(i);
        }

      } else if (
        (sos_sched_table[i].wake.tv_sec == sched_usecond_counter) && (tmp < next)) {
        // see if this is the next event to wake up
        next = tmp;
      }
    }
  }
  if (next < SOS_USECOND_PERIOD) {
    chan_req.value = next;
  }
  sos_config.clock.set_channel(&chan_req);
  scheduler_root_update_on_wake(-1, new_priority);
  sos_config.clock.enable();

  return 1;
}

int root_handle_usecond_process_timer_match_event(
  void *context,
  const mcu_event_t *data) {
  // a system timer expired
  int i;
  int j;
  u32 next;
  u32 tmp;
  mcu_channel_t chan_req;
  u32 now;

  // Initialize variables
  chan_req.loc = SCHED_USECOND_TMR_SYSTEM_TIMER_OC;
  chan_req.value = SOS_USECOND_PERIOD + 1;
  next = SOS_USECOND_PERIOD;

  now = sos_config.clock.disable();

  for (i = 1; i < task_get_total(); i++) {
    // look for the next signal

    if (task_enabled(i)) {

      for (j = 0; j < SOS_PROCESS_TIMER_COUNT; j++) {
        volatile sos_process_timer_t *timer = sos_sched_table[i].timer + j;

        if (timer->o_flags & SCHEDULER_TIMING_PROCESS_TIMER_FLAG_IS_INITIALIZED) {
          tmp = timer->value.tv_usec;
          if (
            (timer->value.tv_sec < sched_usecond_counter)
            || ((timer->value.tv_sec == sched_usecond_counter) && (tmp <= now))) {

            // reload the timer if interval is valid
            send_and_reload_timer(timer, i, now);

            // if interval value is non-zero, need to check if this is less than next
            if (timer->interval.tv_sec + timer->interval.tv_usec) {
              tmp = timer->value.tv_usec;
              if ((timer->value.tv_sec == sched_usecond_counter) && (tmp < next)) {
                next = tmp;
              }
            }

          } else if ((timer->value.tv_sec == sched_usecond_counter) && (tmp < next)) {
            // see if this is the next event to wake up
            next = tmp;
          }
        }
      }
    }
  }

  if (next < SOS_USECOND_PERIOD) {
    chan_req.value = next;
  }

  sos_config.clock.set_channel(&chan_req);
  sos_config.clock.enable();

  return 1;
}

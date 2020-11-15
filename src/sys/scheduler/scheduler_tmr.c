
#include <sdk/types.h>

#include "mcu/tmr.h"

#include "sos/fs/devfs.h"
#include "sos/sos.h"

typedef struct {
  int (*init)();
  void (*enable)();
  void (*disable)();
  void (*set_channel)(mcu_channel_t channel);
  void (*microseconds)();
  void (*nanoseconds)();
} sos_scheduler_tmr_t;

static const devfs_handle_t m_tmr_handle = {.port = 0, .config = NULL, .state = NULL};

int scheduler_tmr_init(
  int (*handle_usecond_overflow)(void *context, const mcu_event_t *handle_match_event),
  int (*handle_usecond_match_event)(void *context, const mcu_event_t *handle_match_event),
  int (*handle_process_timer_match_event)(
    void *context,
    const mcu_event_t *handle_match_event)) {
  int err;
  tmr_attr_t attr;
  tmr_info_t info;
  mcu_action_t action;
  mcu_channel_t chan_req;

  // Open the microsecond timer
  err = mcu_tmr_open(&m_tmr_handle);
  if (err) {
    return err;
  }

  err = mcu_tmr_getinfo(&m_tmr_handle, &info);
  if (err) {
    return err;
  }

  memset(&attr, 0, sizeof(tmr_attr_t));
  attr.freq = 1000000UL;
  attr.o_flags = TMR_FLAG_SET_TIMER | TMR_FLAG_IS_SOURCE_CPU | TMR_FLAG_IS_AUTO_RELOAD;
  attr.period = SOS_USECOND_PERIOD; // only works if TMR_FLAG_IS_AUTO_RELOAD is supported
  memset(&attr.pin_assignment, 0xff, sizeof(tmr_pin_assignment_t));

  err = mcu_tmr_setattr(&m_tmr_handle, &attr);
  if (err) {
    return err;
  }

  // Initialize the value of the timer to zero
  err = mcu_tmr_set(&m_tmr_handle, (void *)0);
  if (err) {
    return err;
  }

  if ((info.o_flags & TMR_FLAG_IS_AUTO_RELOAD) == 0) {
    // The reset OC is only needed if TMR_FLAG_IS_AUTO_RELOAD is not supported
    // Set the reset output compare value to reset the clock every STFY_USECOND_PERIOD

    attr.channel.loc = SCHED_USECOND_TMR_RESET_OC;
    attr.channel.value = SOS_USECOND_PERIOD;
    attr.o_flags = TMR_FLAG_SET_CHANNEL | TMR_FLAG_IS_CHANNEL_RESET_ON_MATCH;
    err = mcu_tmr_setattr(&m_tmr_handle, &attr);
    if (err) {
      return err;
    }

    action.prio = 0;
    action.channel = SCHED_USECOND_TMR_RESET_OC;
    action.o_events = MCU_EVENT_FLAG_MATCH;
    action.handler.callback = handle_usecond_overflow;
    action.handler.context = 0;
    err = mcu_tmr_setaction(&m_tmr_handle, &action);
    if (err) {
      return -1;
    }

  } else {
    action.prio = 0;
    action.channel = 0; // doesn't matter
    action.o_events = MCU_EVENT_FLAG_OVERFLOW;
    action.handler.callback = handle_usecond_overflow;
    action.handler.context = 0;
    err = mcu_tmr_setaction(&m_tmr_handle, &action);
    if (err) {
      return -1;
    }
  }

  // Turn the timer on
  err = mcu_tmr_enable(&m_tmr_handle, 0);
  if (err) {
    return -1;
  }

  // This sets up the output compare unit used with the usleep() function
  chan_req.loc = SCHED_USECOND_TMR_SLEEP_OC;
  chan_req.value = SOS_USECOND_PERIOD + 1;
  err = mcu_tmr_setchannel(&m_tmr_handle, &chan_req);
  if (err) {
    return -1;
  }

  action.channel = SCHED_USECOND_TMR_SLEEP_OC;
  action.o_events = MCU_EVENT_FLAG_MATCH;
  action.handler.callback = handle_usecond_match_event;
  action.handler.context = 0;
  err = mcu_tmr_setaction(&m_tmr_handle, &action);
  if (err) {
    return -1;
  }

#if SOS_PROCESS_TIMER_COUNT > 0
  chan_req.loc = SCHED_USECOND_TMR_SYSTEM_TIMER_OC;
  err = mcu_tmr_setchannel(&m_tmr_handle, &chan_req);
  if (err) {
    return -1;
  }

  action.channel = SCHED_USECOND_TMR_SYSTEM_TIMER_OC;
  action.o_events = MCU_EVENT_FLAG_MATCH;
  action.handler.callback = handle_process_timer_match_event;
  action.handler.context = 0;
  err = mcu_tmr_setaction(&m_tmr_handle, &action);
  if (err) {
    return -1;
  }
#endif

  return 0;
}

void scheduler_tmr_enable() { mcu_tmr_enable(&m_tmr_handle, NULL); }

void scheduler_tmr_disable() { mcu_tmr_disable(&m_tmr_handle, NULL); }

void scheduler_tmr_set_channel(mcu_channel_t channel) {
  mcu_tmr_setchannel(&m_tmr_handle, &channel);
}

u32 scheduler_tmr_microseconds() { return mcu_tmr_get(&m_tmr_handle, NULL); }

u32 scheduler_tmr_nanoseconds() { return 0; }

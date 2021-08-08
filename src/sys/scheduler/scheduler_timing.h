// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#ifndef SCHEDULER_SCHEDULER_TIMING_H_
#define SCHEDULER_SCHEDULER_TIMING_H_

#include "scheduler_local.h"

void scheduler_timing_init();

u32 scheduler_timing_useconds_to_clocks(int useconds);
void scheduler_timing_root_timedblock(void * block_object, struct mcu_timeval * interval);

void scheduler_timing_convert_timespec(struct mcu_timeval * tv, const struct timespec * ts);
void scheduler_timing_convert_mcu_timeval(struct timespec * ts, const struct mcu_timeval * mcu_tv);
void scheduler_timing_svcall_get_realtime(void * args) MCU_ROOT_EXEC_CODE;
void scheduler_timing_root_get_realtime(struct mcu_timeval * tv) MCU_ROOT_CODE;

struct mcu_timeval scheduler_timing_add_mcu_timeval(const struct mcu_timeval * a, const struct mcu_timeval * b);
struct mcu_timeval scheduler_timing_subtract_mcu_timeval(const struct mcu_timeval * a, const struct mcu_timeval * b);

u32 scheduler_timing_get_realtime();
u64 scheduler_timing_real64usec(struct mcu_timeval *tv);

#if CONFIG_TASK_PROCESS_TIMER_COUNT > 0
//per process timers
#define SCHEDULER_TIMING_PROCESS_TIMER_FLAG_IS_INITIALIZED (1<<0)
#define SCHEDULER_TIMING_PROCESS_TIMER_FLAG_IS_QUEUED (1<<1)

#define SCHEDULER_TIMING_PROCESS_TIMER(t_id, id_off) (t_id << 8 | id_off)

void scheduler_timing_root_process_timer_initialize(u16 task_id) MCU_ROOT_EXEC_CODE;
volatile sos_process_timer_t * scheduler_timing_process_timer(timer_t timer_id);
timer_t scheduler_timing_process_create_timer(const struct sigevent *evp);
int scheduler_timing_process_delete_timer(timer_t timer_id);
void scheduler_timing_process_cancel_timer(timer_t timer_id);
int scheduler_timing_process_set_timer(timer_t timerid, int flags,
						const struct mcu_timeval * value,
						const struct mcu_timeval * interval,
						struct mcu_timeval * o_value,
						struct mcu_timeval * o_interval);
int scheduler_timing_process_get_timer(timer_t timerid, struct mcu_timeval * value, struct mcu_timeval * interval, struct mcu_timeval * now);

void scheduler_timing_process_unqueue_timer(int tid, int si_signo, union sigval sig_value);
#endif



#endif /* SCHEDULER_SCHEDULER_TIMING_H_ */

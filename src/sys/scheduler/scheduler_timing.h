/* Copyright 2011-2017 Tyler Gilbert;
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

#ifndef SCHEDULER_SCHEDULER_TIMING_H_
#define SCHEDULER_SCHEDULER_TIMING_H_

#include "mcu/types.h"
#include "sos/sos.h"

#define SCHEDULER_CLOCK_NSEC_DIV ((u32)((u64)1024 * 1000000000 / mcu_board_config.core_cpu_freq))
#define SCHEDULER_CLOCK_USEC_MULT ((u32)(mcu_board_config.core_cpu_freq / 1000000))

#define SCHEDULER_TIMEVAL_SEC_INVALID UINT_MAX
#define SCHEDULER_TIMEVAL_SECONDS STFY_SCHEDULER_TIMEVAL_SECONDS



int scheduler_timing_init();

u32 scheduler_timing_seconds_to_clocks(int seconds);
u32 scheduler_timing_useconds_to_clocks(int useconds);
void scheduler_timing_root_timedblock(void * block_object, struct mcu_timeval * interval);

void scheduler_timing_convert_timespec(struct mcu_timeval * tv, const struct timespec * ts);
void scheduler_timing_convert_mcu_timeval(struct timespec * ts, const struct mcu_timeval * mcu_tv);
void scheduler_timing_svcall_get_realtime(void * args) MCU_ROOT_EXEC_CODE;
void scheduler_timing_root_get_realtime(struct mcu_timeval * tv) MCU_ROOT_CODE;

struct mcu_timeval scheduler_timing_add_mcu_timeval(const struct mcu_timeval * a, const struct mcu_timeval * b);
struct mcu_timeval scheduler_timing_subtract_mcu_timeval(const struct mcu_timeval * a, const struct mcu_timeval * b);

u32 scheduler_timing_get_realtime();




//per process timers
#define SCHEDULER_TIMING_PROCESS_TIMER_FLAG_IS_INITIALIZED (1<<0)
#define SCHEDULER_TIMING_PROCESS_TIMER_FLAG_IS_QUEUED (1<<1)

#define SCHEDULER_TIMING_PROCESS_TIMER(t_id, id_off) (t_id << 8 | id_off)

void scheduler_timing_root_process_timer_initialize(u16 task_id);
volatile sos_process_timer_t * scheduler_timing_process_timer(timer_t timer_id);
timer_t scheduler_timing_process_create_timer(const struct sigevent *evp);
int scheduler_timing_process_delete_timer(timer_t timer_id);
int scheduler_timing_process_set_timer(timer_t timerid, int flags,
						const struct mcu_timeval * value,
						const struct mcu_timeval * interval,
						struct mcu_timeval * o_value,
						struct mcu_timeval * o_interval);
int scheduler_timing_process_get_timer(timer_t timerid, struct mcu_timeval * value, struct mcu_timeval * interval, struct mcu_timeval * now);

void scheduler_timing_process_unqueue_timer(int tid, int si_signo, union sigval sig_value);




#endif /* SCHEDULER_SCHEDULER_TIMING_H_ */

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


#define SCHEDULER_CLOCK_NSEC_DIV ((u32)((u64)1024 * 1000000000 / mcu_board_config.core_cpu_freq))
#define SCHEDULER_CLOCK_USEC_MULT ((u32)(mcu_board_config.core_cpu_freq / 1000000))

#define SCHEDULER_TIMEVAL_SEC_INVALID UINT_MAX
#define SCHEDULER_TIMEVAL_SECONDS STFY_SCHEDULER_TIMEVAL_SECONDS

int scheduler_timing_init();

u32 scheduler_timing_seconds_to_clocks(int seconds);
u32 scheduler_timing_useconds_to_clocks(int useconds);
u32 scheduler_timing_nanoseconds_to_clocks(int nanoseconds);
void scheduler_timing_root_timedblock(void * block_object, struct mcu_timeval * interval);

void scheduler_timing_convert_timespec(struct mcu_timeval * tv, const struct timespec * ts);
void scheduler_timing_root_get_realtime(struct mcu_timeval * tv) MCU_ROOT_EXEC_CODE;


#endif /* SCHEDULER_SCHEDULER_TIMING_H_ */

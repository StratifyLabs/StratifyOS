/* Copyright 2011-2016 Tyler Gilbert; 
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

/*! \addtogroup STFY
 * @{
 */

/*! \file */

#include "config.h"
#include <errno.h>
#include "iface/dev/sys.h"
#include "mcu/core.h"
#include "mcu/sys.h"
#include "mcu/debug.h"
#include "sched/sched_flags.h"
#include "signal/sig_local.h"
#include "device/sys.h"
#include "symbols.h"

static int read_task(sys_taskattr_t * task);

uint8_t sys_euid MCU_SYS_MEM;

int sys_open(const device_cfg_t * cfg){
	return 0;
}

int sys_ioctl(const device_cfg_t * cfg, int request, void * ctl){
	sys_attr_t * sys = ctl;
	sys_killattr_t * killattr = ctl;

	int i;
	switch(request){
	case  I_SYS_GETATTR:
		strncpy(sys->version, VERSION, 7);
		strncpy(sys->sys_version, stfy_board_config.sys_version, 7);
		sys->version[7] = 0;
		strncpy(sys->arch, ARCH, 7);
		sys->arch[7] = 0;
		sys->security = _mcu_sys_getsecurity();
		sys->signature = symbols_table[0];
		sys->cpu_freq = _mcu_core_getclock();
		sys->sys_mem_size = stfy_board_config.sys_memory_size;
		sys->flags = stfy_board_config.sys_flags;
		strncpy(sys->stdin_name, stfy_board_config.stdin_dev, NAME_MAX-1);
		strncpy(sys->stdout_name, stfy_board_config.stdout_dev, NAME_MAX-1);
		strncpy(sys->name, stfy_board_config.sys_name, NAME_MAX-1);
		_mcu_core_getserialno(&(sys->serial));
		return 0;
	case I_SYS_GETTASK:
		return read_task(ctl);
	case I_SYS_KILL:
		for(i = 1; i < task_get_total(); i++){
			if( (task_get_pid(i) == killattr->id) &&
					!task_isthread_asserted(i)
			){
				signal_priv_send(task_get_current(),
						i,
						killattr->si_signo,
						killattr->si_sigcode,
						killattr->si_sigvalue, 1);
				break;
			}
		}
		return 0;
	case I_SYS_PTHREADKILL:
		return signal_priv_send(task_get_current(),
				killattr->id,
				killattr->si_signo,
				killattr->si_sigcode,
				killattr->si_sigvalue, 1);
	}
	errno = EINVAL;
	return -1;
}

int sys_read(const device_cfg_t * cfg, device_transfer_t * rop){
	errno = ENOTSUP;
	return -1;
}

int sys_write(const device_cfg_t * cfg, device_transfer_t * wop){
	errno = ENOTSUP;
	return -1;
}

int sys_close(const device_cfg_t * cfg){
	return 0;
}

int read_task(sys_taskattr_t * task){
	int ret;
	if ( task->tid < task_get_total() ){
		if ( task_enabled( task->tid )){
			task->is_enabled = 1;
			task->pid = task_get_pid( task->tid );
			task->timer = task_priv_gettime(task->tid);
			task->mem_loc = (uint32_t)stfy_sched_table[task->tid].attr.stackaddr;
			task->mem_size = stfy_sched_table[task->tid].attr.stacksize;
			task->stack_ptr = (uint32_t)task_table[task->tid].sp;
			task->prio = stfy_sched_table[task->tid].priority;
			task->prio_ceiling = stfy_sched_table[task->tid].attr.schedparam.sched_priority;
			task->is_active = (sched_active_asserted(task->tid) != 0) | ((sched_stopped_asserted(task->tid != 0)<<1));
			task->is_thread = task_isthread_asserted( task->tid );

			strncpy(task->name, ((struct _reent*)task_table[ task->tid ].global_reent)->procmem_base->proc_name, NAME_MAX);

			if ( !task->is_thread && ( task_table[task->tid].reent != NULL) ){
				task->malloc_loc = (uint32_t)&(((struct _reent*)task_table[task->tid].reent)->procmem_base->base)
						+ ((struct _reent*)task_table[task->tid].reent)->procmem_base->size;
			} else {
				task->malloc_loc = 0;
			}


			ret = 1;

		} else {
			task->is_enabled = 0;
			ret = 0;
		}
		errno = 0;
	} else {
		errno = ESRCH;
		ret = -1;
	}

	return ret;
}



/*! @} */

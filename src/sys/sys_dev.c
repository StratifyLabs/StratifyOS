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
#include "sos/dev/sys.h"
#include "mcu/core.h"
#include "mcu/sys.h"
#include "mcu/debug.h"
#include "sched/sched_flags.h"
#include "signal/sig_local.h"
#include "mcu/sys.h"
#include "symbols.h"

extern void mcu_core_hardware_id();

static int read_task(sys_taskattr_t * task);

uint8_t sys_euid MCU_SYS_MEM;

int sys_open(const devfs_handle_t * cfg){
	return 0;
}

int sys_ioctl(const devfs_handle_t * cfg, int request, void * ctl){
	sys_id_t * id = ctl;
	sys_info_t * sys = ctl;
	sys_killattr_t * killattr = ctl;

	int i;
	switch(request){
	case  I_SYS_GETINFO:
		memset(sys, 0, sizeof(sys_info_t));
		strncpy(sys->kernel_version, VERSION, 7);
		strncpy(sys->sys_version, sos_board_config.sys_version, 7);
		strncpy(sys->arch, ARCH, 15);
		sys->security = 0;
		sys->signature = symbols_table[0];
		sys->cpu_freq = mcu_core_getclock();
		sys->sys_mem_size = sos_board_config.sys_memory_size;
		sys->o_flags = sos_board_config.o_sys_flags;
		strncpy(sys->id, sos_board_config.sys_id, PATH_MAX-1);
		strncpy(sys->stdin_name, sos_board_config.stdin_dev, NAME_MAX-1);
		strncpy(sys->stdout_name, sos_board_config.stdout_dev, NAME_MAX-1);
		strncpy(sys->name, sos_board_config.sys_name, NAME_MAX-1);
		mcu_core_getserialno(&(sys->serial));
		sys->hardware_id = (u32)mcu_core_hardware_id;
		return 0;
	case I_SYS_GETTASK:
		return read_task(ctl);

	case I_SYS_GETID:
		memcpy(id->id, sos_board_config.sys_id, PATH_MAX-1);
		return 0;
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
	case I_SYS_GETBOARDCONFIG:
		memcpy(ctl, &sos_board_config, sizeof(sos_board_config));
		return 0;
	default:
		break;
	}
	errno = EINVAL;
	return -1;
}

int sys_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	errno = ENOTSUP;
	return -1;
}

int sys_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	errno = ENOTSUP;
	return -1;
}

int sys_close(const devfs_handle_t * cfg){
	return 0;
}

int read_task(sys_taskattr_t * task){
	int ret;
	if ( task->tid < task_get_total() ){
		if ( task_enabled( task->tid )){
			task->is_enabled = 1;
			task->pid = task_get_pid( task->tid );
			task->timer = task_priv_gettime(task->tid);
			task->mem_loc = (uint32_t)sos_sched_table[task->tid].attr.stackaddr;
			task->mem_size = sos_sched_table[task->tid].attr.stacksize;
			task->stack_ptr = (uint32_t)task_table[task->tid].sp;
			task->prio = sos_sched_table[task->tid].priority;
			task->prio_ceiling = sos_sched_table[task->tid].attr.schedparam.sched_priority;
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

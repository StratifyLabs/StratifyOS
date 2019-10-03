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


#include "config.h"
#include <errno.h>
#include "sos/dev/sys.h"
#include "sos/dev/bootloader.h"
#include "mcu/core.h"
#include "device/sys.h"
#include "mcu/debug.h"
#include "mcu/mcu.h"
#include "scheduler/scheduler_local.h"
#include "cortexm/mpu.h"

#include "signal/sig_local.h"
#include "device/sys.h"
#include "symbols.h"

extern void mcu_core_hardware_id();

static int read_task(sys_taskattr_t * task);
static int sys_setattr(const devfs_handle_t * handle, void * ctl);


int sys_open(const devfs_handle_t * handle){
	return 0;
}

int sys_ioctl(const devfs_handle_t * handle, int request, void * ctl){
	sys_id_t * id = ctl;
	sys_info_t * info = ctl;
	sys_killattr_t * killattr = ctl;
	int i;

	switch(request){
		case  I_SYS_GETINFO:
			memset(info, 0, sizeof(sys_info_t));
			strncpy(info->kernel_version, VERSION, 7);
			strncpy(info->sys_version, sos_board_config.sys_version, 7);
			strncpy(info->arch, ARCH, 15);
			info->security = 0;
			info->signature = symbols_table[0];
			info->cpu_freq = mcu_core_getclock();
			info->sys_mem_size = sos_board_config.sys_memory_size;
			info->o_flags = sos_board_config.o_sys_flags;
			info->o_mcu_board_config_flags = mcu_board_config.o_flags;
			strncpy(info->id, sos_board_config.sys_id, PATH_MAX-1);
			strncpy(info->stdin_name, sos_board_config.stdin_dev, NAME_MAX-1);
			strncpy(info->stdout_name, sos_board_config.stdout_dev, NAME_MAX-1);
			strncpy(info->name, sos_board_config.sys_name, NAME_MAX-1);
			strncpy(info->trace_name, sos_board_config.trace_dev, NAME_MAX-1);
			if( sos_board_config.git_hash ){
				strncpy(info->bsp_git_hash, sos_board_config.git_hash, 15);
			}
			strncpy(info->sos_git_hash, SOS_GIT_HASH, 15);
			if( mcu_config.git_hash ){
				strncpy(info->mcu_git_hash, mcu_config.git_hash, 15);
			}
			mcu_core_getserialno(&(info->serial));
			info->hardware_id = *((u32*)(&_text + BOOTLOADER_HARDWARE_ID_OFFSET/sizeof(u32)));
			return 0;
		case I_SYS_GETTASK:
			return read_task(ctl);

		case I_SYS_GETID:
			memcpy(id->id, sos_board_config.sys_id, PATH_MAX-1);
			return 0;
		case I_SYS_KILL:
			for(i = 1; i < task_get_total(); i++){
				if( (task_get_pid(i) == killattr->id) && task_enabled(i) &&
					 !task_thread_asserted(i)
					 ){
					int result = signal_root_send(task_get_current(),
															i,
															killattr->si_signo,
															killattr->si_sigcode,
															killattr->si_sigvalue, 1);
					return result;
				}
			}
			return SYSFS_SET_RETURN(EINVAL);
		case I_SYS_PTHREADKILL:
			return signal_root_send(task_get_current(),
											killattr->id,
											killattr->si_signo,
											killattr->si_sigcode,
											killattr->si_sigvalue, 1);
		case I_SYS_GETBOARDCONFIG:

			memcpy(ctl, &sos_board_config, sizeof(sos_board_config));
			return 0;


		case I_SYS_GETMCUBOARDCONFIG:
			{
				mcu_board_config_t * config = ctl;
				memcpy(config, &mcu_board_config, sizeof(mcu_board_config));
				//dont' provide any security info to non-root callers
				if( scheduler_authenticated_asserted( task_get_current() ) == 0 ){
					config->secret_key_address = (void*)-1;
					config->secret_key_size = 0;
				}
				return 0;
			}


		case I_SYS_SETATTR:
			return sys_setattr(handle, ctl);



		default:
			break;
	}
	return SYSFS_SET_RETURN(EINVAL);
}

int sys_read(const devfs_handle_t * handle, devfs_async_t * rop){
	return SYSFS_SET_RETURN(ENOTSUP);
}

int sys_write(const devfs_handle_t * handle, devfs_async_t * wop){
	return SYSFS_SET_RETURN(ENOTSUP);
}

int sys_close(const devfs_handle_t * handle){
	return 0;
}

int read_task(sys_taskattr_t * task){
	int ret;
	if ( task->tid < task_get_total() ){
		if ( task_enabled( task->tid )){
			task->is_enabled = 1;
			task->pid = task_get_pid( task->tid );
			task->timer = task_root_gettime(task->tid);
			task->mem_loc = (u32)sos_sched_table[task->tid].attr.stackaddr;
			task->mem_size = sos_sched_table[task->tid].attr.stacksize;
			task->stack_ptr = (u32)sos_task_table[task->tid].sp;
			task->prio = task_get_priority(task->tid);
			task->prio_ceiling = sos_sched_table[task->tid].attr.schedparam.sched_priority;
			task->is_active = (task_active_asserted(task->tid) != 0) | ((task_stopped_asserted(task->tid != 0)<<1));
			task->is_thread = task_thread_asserted( task->tid );

			strncpy(task->name, ((struct _reent*)sos_task_table[ task->tid ].global_reent)->procmem_base->proc_name, NAME_MAX);

#if 1
			task->malloc_loc = scheduler_calculate_heap_end(task->tid);
#else
			if ( !task->is_thread &&
				  ( sos_task_table[task->tid].reent != NULL)
				  ){
				task->malloc_loc =
						(u32)&(((struct _reent*)sos_task_table[task->tid].reent)->procmem_base->base) +
						((struct _reent*)sos_task_table[task->tid].reent)->procmem_base->size;

			} else {
				task->malloc_loc = 0;
			}
#endif


			ret = 1;

		} else {
			task->is_enabled = 0;
			ret = 0;
		}
	} else {
		//Stratify Link freezes up if this doesn't return -1 -- needs to be fixed
		ret = SYSFS_SET_RETURN_WITH_VALUE(ESRCH, 1);
	}

	return ret;
}

int sys_setattr(const devfs_handle_t * handle, void * ctl){
	int result;
	const sys_attr_t * attr = ctl;

	if( attr == 0 ){ return SYSFS_SET_RETURN(EINVAL); }

	u32 o_flags = attr->o_flags;

	if( o_flags & SYS_FLAG_SET_MEMORY_REGION ){

		if( attr->region == TASK_APPLICATION_DATA_USER_REGION ){

			int type = MPU_MEMORY_SRAM;
			if( o_flags & SYS_FLAG_IS_FLASH ){
				type = MPU_MEMORY_FLASH;

			} else if( o_flags & SYS_FLAG_IS_EXTERNAL ){
				type = MPU_MEMORY_EXTERNAL_SRAM;
			}

			int access = MPU_ACCESS_PRW;
			if( (o_flags & SYS_FLAG_IS_READ_ALLOWED) && (o_flags & SYS_FLAG_IS_WRITE_ALLOWED) ){
				access = MPU_ACCESS_PRW_URW;
			} else if( o_flags & SYS_FLAG_IS_READ_ALLOWED ){
				access = MPU_ACCESS_PRW_UR;
			}


			result = mpu_enable_region(
						attr->region,
						(void*)attr->address,
						attr->size,
						access,
						type,
						0);

			if ( result < 0 ){
				mcu_debug_log_error(MCU_DEBUG_SYS, "Failed to enable memory region 0x%lX to 0x%lX (%d)", attr->address, attr->size, result);
				return result;
			}

		} else {
			return SYSFS_SET_RETURN(EINVAL);
		}
	}
	return 0;
}


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

#include "cortexm/cortexm.h"
#include "sos/sos.h"
#include "sos/link/transport_usb.h"
#include "cortexm/task.h"
#include "mcu/core.h"
#include "sos/link.h"
#include "cortexm/mpu.h"

#define OLD_WAY_STACK_TRACE 0

static void sos_trace_event_addr(
		link_trace_event_id_t event_id,
		const void * data_ptr,
		size_t data_len,
		u32 addr
		);
static void sos_trace_build_event(
		link_trace_event_t * event,
		link_trace_event_id_t event_id,
		const void * data_ptr,
		size_t data_len,
		u32 addr,
		int tid,
		const struct timespec * spec
		);
static void svcall_trace_event(void * args);
static void svcall_get_stack_pointer(void * args);

#if OLD_WAY_STACK_TRACE == 0
static u16 decode_stack_modifier_opcodes(u16 * machine_code, u32 * stack_jump);
static u16 * scan_code_for_push(u32 link_address, u32* stack_jump);
static u32 decode_push_opcode(u16* opcode);
static u32 decode_store_on_stack_opcode(u16* opcode);
static u32 decode_subtract_stack_opcode(u16* opcode);
#else
static u32 lookup_caller_adddress(u32 input);
#endif


void sos_trace_root_trace_event(
		link_trace_event_id_t event_id,
		const void * data_ptr,
		size_t data_len
		){
	register u32 lr asm("lr");
	link_trace_event_t event;
	if( sos_board_config.trace_event ){
		sos_trace_build_event(&event, event_id, data_ptr, data_len, lr, task_get_current(), 0);
		sos_board_config.trace_event(&event);
	}
}

void sos_trace_build_event(
		link_trace_event_t * event,
		link_trace_event_id_t event_id,
		const void * data_ptr,
		size_t data_len,
		u32 addr,
		int tid,
		const struct timespec * spec
		){
	event->header.size = sizeof(link_trace_event_t);
	event->header.id = LINK_NOTIFY_ID_POSIX_TRACE_EVENT;
	event->posix_trace_event.posix_event_id = event_id;
	event->posix_trace_event.posix_pid = task_get_pid( tid );
	event->posix_trace_event.posix_prog_address = addr; //grab the value of the caller
	event->posix_trace_event.posix_thread_id = tid;

	if( data_len > LINK_POSIX_TRACE_DATA_SIZE-1  ){
		data_len = LINK_POSIX_TRACE_DATA_SIZE-1;
		event->posix_trace_event.posix_truncation_status = 1;
	} else {
		event->posix_trace_event.posix_truncation_status = 0;
	}

	memset(event->posix_trace_event.data, 0, LINK_POSIX_TRACE_DATA_SIZE);
	memcpy(event->posix_trace_event.data, data_ptr, data_len);
	if (spec){
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
		const void * data_ptr,
		size_t data_len
		){
	register u32 lr asm("lr");
	sos_trace_event_addr(event_id, data_ptr, data_len, lr);
}

void sos_trace_event_addr(
		link_trace_event_id_t event_id,
		const void * data_ptr,
		size_t data_len,
		u32 addr
		){
	sos_trace_event_addr_tid(event_id, data_ptr, data_len, addr, task_get_current());
}

void svcall_trace_event(void * args){
	CORTEXM_SVCALL_ENTER();
	sos_board_config.trace_event(args);
}

void sos_trace_event_addr_tid(
		link_trace_event_id_t event_id,
		const void * data_ptr,
		size_t data_len,
		u32 addr,
		int tid
		){
	//record event id and in-calling processes trace stream
	struct timespec spec;
	link_trace_event_t event;

	if( sos_board_config.trace_event ){
		//convert the address using the task memory location
		//check if addr is part of kernel or app
		if( ((addr >= (u32)&_text) && (addr < (u32)&_etext)) || (addr == 1) ){
			//kernel
			addr = addr - 1;
		} else {
			//app
			addr = addr - (u32)sos_task_table[tid].mem.code.address - 1 + 0xDE000000;
		}

		clock_gettime(CLOCK_REALTIME, &spec);

		sos_trace_build_event(
					&event,
					event_id,
					data_ptr,
					data_len,
					addr,
					tid,
					&spec
					);

		cortexm_svcall(svcall_trace_event, &event);

	}

}

void svcall_get_stack_pointer(void * args){
	CORTEXM_SVCALL_ENTER();
	cortexm_get_thread_stack_ptr(args);
}


#if OLD_WAY_STACK_TRACE
u32 lookup_caller_adddress(u32 input){

	if( (input & 0x01) == 0 ){
		return 0; //don't trace non-callers
	}

	//check if input is a caller for the kernel
	if( (input >= (u32)&_text) &&
			(input < (u32)&_etext)
			){
		return input;
	}

	//check if input is a caller for the application
	const u8 tid = task_get_current();
	if( (input >= (u32)sos_task_table[tid].mem.code.address) &&
			(input <
			 (u32)sos_task_table[tid].mem.code.address +
			 sos_task_table[tid].mem.code.size) ){
		return input;
	}
	return 0;
}
#endif

int sos_trace_stack(u32 count){
	u32 * sp;
	cortexm_svcall(svcall_get_stack_pointer, &sp);

	/*
	 * cortexm_svcall() reads the PSP in handler mode. So
	 * the PSP has a hardware stack from on top of it when it
	 * is calculated. This needs to be adjusted for.
	 *
	 * The hardware stack frame as 8 registers
	 *
	 */
	sp += 8;

#if OLD_WAY_STACK_TRACE == 0
	CORTEXM_DECLARE_LINK_REGISTER(first_link_register);
	char message[20] = {0};
	int len;
	strncpy(message, "stackTrace", 16);
	len = strnlen(message, 16);

	u32 stack_top = (u32)(sos_task_table[task_get_current()].mem.data.address +
									 sos_task_table[task_get_current()].mem.data.size);

	u32 next_link_register = first_link_register & ~0x01;

	u16 * code_pointer;
	u32 push_count = 0;

	u32 stack_jump = 0;
	do {
		code_pointer = scan_code_for_push(next_link_register, &stack_jump);
		//code points near the entrance of the function

		for(int i=0; i < stack_jump; i++){
			mcu_debug_printf(
						"stack preview %d:%x -> %08x\n",
						i,
						sp + i,
						sp[i]
						);
		}

		sp += (stack_jump);
		mcu_debug_printf("Pushed %p < %p registers\n", sp, stack_top);


		next_link_register = sp[-1]  & ~0x01;
		mcu_debug_printf("Next link is %08x\n", next_link_register);

		sos_trace_event_addr(
					LINK_POSIX_TRACE_MESSAGE,
					message,
					len,
					((u32)code_pointer)+1
					);

		push_count++;

	} while( ((u32)sp < stack_top) &&
					 (push_count < count) );

	return push_count;

#else
	sp	= (void*)((u32)sp & ~0x03);
	u32 stack_count =
			(u32)(sos_task_table[task_get_current()].mem.data.address +
			sos_task_table[task_get_current()].mem.data.size)
			- (u32)sp;

	u32 * stack = (u32*)sp;
	char message[17];
	int len;
	strncpy(message, "stackTrace", 16);
	len = strnlen(message, 16);

	u32 address;
	stack_count = stack_count/sizeof(u32);
	u32 push_count = 0;
	for(int i=stack_count-1; i >= 0; i--){
		address = lookup_caller_adddress(stack[i]);
		if( address != 0 ){
			sos_trace_event_addr(
						LINK_POSIX_TRACE_MESSAGE,
						message,
						len,
						address
						);
			push_count++;
			if( push_count == count ){
				return push_count;
			}
		}
	}
	return push_count;
#endif
}

#if OLD_WAY_STACK_TRACE == 0
u16 * scan_code_for_push(u32 link_address, u32 * stack_jump){
	u16 * code_pointer = (u16*)link_address;

	/*
	 * push is encoded as b5xx: xx is the register mask -- can be a 16-bit instruction
	 * stmdb is encoded as e92d xxxx: xxxx is the register mask
	 * subtract from sp as 0xb08x: x is subtraction / 4
	 *
	 *
	 */

	mcu_debug_printf("scan starting at %p\n", link_address);
	*stack_jump = 0;
	while( (code_pointer != 0) &&
				 ((decode_stack_modifier_opcodes(code_pointer, stack_jump)) == 0)  ){

		if( (*code_pointer & 0xe000) == 0xe000 ){
			code_pointer--;
		}
		code_pointer--;
	}

	mcu_debug_printf("push at %p\n", code_pointer);

	return code_pointer;
}

u32 decode_push_opcode(u16 * opcode){
	u32 count = 0;
	if( (*opcode & 0xff00) == 0xb500 ){
		u8 register_mask = *opcode & 0xff;
		count++;
		for(u32 i=0; i < 8; i++){
			if( (1<<i) & register_mask ){
				count++;
			}
		}
	}
	return count;
}


u32 decode_store_on_stack_opcode(u16 * opcode){
	u32 count = 0;
	if( *opcode == 0xe92d ){
		u16 register_mask = *(opcode+1);
		for(u32 i=0; i < 16; i++){
			if( (1<<i) & register_mask ){
				count++;
			}
		}
	}
	return count;
}

u32 decode_subtract_stack_opcode(u16 * opcode){
	if( (*opcode & 0xff80) == 0xb080 ){
		return *opcode & 0x007f;
	}
	return 0;
}

u16 decode_stack_modifier_opcodes(u16 * machine_code, u32* stack_jump){
	u32 jump = decode_subtract_stack_opcode(machine_code);
	if( jump ){
		*stack_jump += jump;
		mcu_debug_printf("subtract %d opcode: %04x %08x\n", jump, *machine_code, machine_code);
		return 0;
	}

	jump = decode_store_on_stack_opcode(machine_code);
	if( jump ){
		*stack_jump += jump;
		mcu_debug_printf("store %d opcode: %04x %08x\n", jump, *machine_code, machine_code);
		return 1;
	}

	jump = decode_push_opcode(machine_code);
	if( jump ){
		*stack_jump += jump;
		mcu_debug_printf("push %d opcode: %04x %08x\n", jump, *machine_code, machine_code);
		return 1;
	}

	return 0;
}
#endif



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

#include <fcntl.h>
#include <errno.h>
#include <stddef.h>
#include "mcu/debug.h"
#include "device/mcfifo.h"

static u32 get_ready_channels(const mcfifo_config_t * config, mcfifo_state_t * state);

int mcfifo_open(const devfs_handle_t * handle){
	return 0;
}

int mcfifo_ioctl(const devfs_handle_t * handle, int request, void * ctl){
	const mcfifo_config_t * config = handle->config;
	mcfifo_state_t * state = handle->state;
	mcfifo_fiforequest_t * fifo_request = ctl;
	mcfifo_info_t * info = ctl;
	mcu_channel_t * channel = ctl;
	mcu_action_t * action = ctl;

	switch(request){
	case I_MCFIFO_GETVERSION: return MCFIFO_VERSION;
	case I_MCFIFO_GETINFO:
		memset(info, 0, sizeof(mcfifo_info_t));
		info->size = config->size;
		info->count = config->count;
		info->o_ready = get_ready_channels(config, state);
		return 0;

	case I_MCFIFO_SETATTR:
		return 0;

	case I_MCFIFO_GETOWNER:
		if( channel->loc < config->count ){
			channel->value = state->owner_array[channel->loc];
			return 0;
		} else {
			errno = EINVAL;
		}
		return -1;

	case I_MCFIFO_SETOWNER:
		if( channel->loc < config->count ){
			state->owner_array[channel->loc] = channel->value;
			return 0;
		} else {
			errno = EINVAL;
		}
		return -1;

	case I_MCFIFO_FIFOREQUEST:
		if( fifo_request->channel < config->count ){

			return fifo_ioctl_local(
					config->fifo_config_array + fifo_request->channel,
					state->fifo_state_array + fifo_request->channel,
					fifo_request->request,
					fifo_request->ctl);

		} else {
			errno = EINVAL;
		}
		return -1;

	case I_MCU_SETACTION:
		//mcu action channel to figure out which fifo
		if( action->channel < config->count ){

			return fifo_ioctl_local(
					config->fifo_config_array + action->channel,
					state->fifo_state_array + action->channel,
					I_MCU_SETACTION,
					action);
		}
	}

	errno = EINVAL;
	return -1;
}

int mcfifo_read(const devfs_handle_t * handle, devfs_async_t * async){
	u32 loc = (u32)async->loc;
	int ret = -1;
	const mcfifo_config_t * config = handle->config;
	mcfifo_state_t * state = handle->state;


	if( loc < config->count ){
		ret = fifo_read_local(config->fifo_config_array + loc, state->fifo_state_array + loc, async);
	} else {
		errno = EINVAL;
	}
	return ret;
}

int mcfifo_write(const devfs_handle_t * handle, devfs_async_t * async){
	u32 loc = (u32)async->loc;
	int ret = -1;
	const mcfifo_config_t * config = handle->config;
	mcfifo_state_t * state = handle->state;

	if( loc < config->count ){
		ret = fifo_write_local(&config->fifo_config_array[loc], &state->fifo_state_array[loc], async);
	} else {
		errno = EINVAL;
	}
	return ret;
}

int mcfifo_close(const devfs_handle_t * handle){
	return 0;
}

u32 get_ready_channels(const mcfifo_config_t * config, mcfifo_state_t * state){
	u32 o_ready;
	fifo_info_t info;
	fifo_state_t * fifo_state;
	const fifo_config_t * fifo_config;
	int i;

	o_ready = 0;
	for(i=0; i < config->count; i++){
		fifo_config = config->fifo_config_array + i;
		fifo_state = state->fifo_state_array + i;
		fifo_getinfo(&info, fifo_config, fifo_state);
		if( info.used > 0 ){
			o_ready |= (1<<i);
		}
	}
	return o_ready;
}

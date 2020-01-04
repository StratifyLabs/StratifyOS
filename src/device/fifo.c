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

#include <fcntl.h>
#include <errno.h>
#include <stddef.h>
#include "mcu/debug.h"
#include "device/fifo.h"


void fifo_inc_tail(fifo_state_t * state, int size){
   state->atomic_position.access.tail++;
   if ( state->atomic_position.access.tail == size ){
      state->atomic_position.access.tail = 0;
   }
}

void fifo_inc_head(fifo_state_t * state, int size){
   state->atomic_position.access.head++;
   if ( state->atomic_position.access.head == size ){
      state->atomic_position.access.head = 0;
   }


   if( state->atomic_position.access.head == state->atomic_position.access.tail ){
      //set tail to size when full
      state->atomic_position.access.tail = size;
   }

}

int fifo_is_write_ok(fifo_state_t * state, u16 size, int writeblock){
   int ret = 1;

   if( state->atomic_position.access.tail == size ){  //the tail is set to size when the buffer is full
      if( writeblock || (state->o_flags & FIFO_FLAG_IS_READ_BUSY) ){
         //cannot write anymore data at this time
         ret = 0;
      } else {
         //OK to write but it will cause an overflow
         if( state->o_flags & FIFO_FLAG_IS_READ_BUSY ){
            state->o_flags |= FIFO_FLAG_IS_WRITE_WHILE_READ_BUSY;
         }
         fifo_set_overflow(state, 1);
      }
   }

   //return OK or not to write: 1 = OK, 0 = write is blocked
   return ret;
}

int fifo_is_writeblock(fifo_state_t * state){
   return ((state->o_flags & FIFO_FLAG_SET_WRITEBLOCK) != 0);
}

void fifo_set_writeblock(fifo_state_t * state, int value){
   if( value ){
      state->o_flags |= FIFO_FLAG_SET_WRITEBLOCK;
   } else {
      state->o_flags &= ~FIFO_FLAG_SET_WRITEBLOCK;
   }
}

int fifo_is_overflow(fifo_state_t * state){
   return ((state->o_flags & FIFO_FLAG_IS_OVERFLOW) != 0);
}

void fifo_set_overflow(fifo_state_t * state, int value){
   if( value ){
      state->o_flags |= FIFO_FLAG_IS_OVERFLOW;
   } else {
      state->o_flags &= ~FIFO_FLAG_IS_OVERFLOW;
   }
}

int fifo_read_buffer(const fifo_config_t * config, fifo_state_t * state, char * buf, int nbyte){
   int i;
   u16 size = config->size;
   int read_was_clobbered = 0;
   char * dest_buffer = config->buffer;
   fifo_atomic_position_t atomic_position;
   for(i=0; i < nbyte; i++){

      state->o_flags |= FIFO_FLAG_IS_READ_BUSY;
      atomic_position.atomic_access = state->atomic_position.atomic_access;

      if( atomic_position.access.head != atomic_position.access.tail ){
         if( atomic_position.access.tail == size ){
            //buffer is full -- restore tail position
            atomic_position.access.tail = atomic_position.access.head;
         }
         buf[i] = dest_buffer[atomic_position.access.tail];
         atomic_position.access.tail++;
         if( atomic_position.access.tail == size ){
            atomic_position.access.tail = 0;
         }
         //an interrupt here before the tail is assigned will cause a problem
         state->atomic_position.access.tail = atomic_position.access.tail;
         //an interrupt here is OK because the write can write to the open spot
         if( state->o_flags & FIFO_FLAG_IS_WRITE_WHILE_READ_BUSY ){
            read_was_clobbered = 1;
            //if the read was clobbered the buffer is full
            state->atomic_position.access.tail = size;
         }

         state->o_flags &= ~(FIFO_FLAG_IS_WRITE_WHILE_READ_BUSY|FIFO_FLAG_IS_READ_BUSY);

         if( read_was_clobbered ){
            return i;
         }

      } else {
         state->o_flags &= ~(FIFO_FLAG_IS_WRITE_WHILE_READ_BUSY|FIFO_FLAG_IS_READ_BUSY);
         break;
      }

   }
   return i; //number of bytes read
}


int fifo_write_buffer(const fifo_config_t * cfgp, fifo_state_t * state, const char * buf, int nbyte, int non_blocking){
   int i;
   int size = cfgp->size;
   int writeblock = 1;
   if( non_blocking == 0 ){
      writeblock = fifo_is_writeblock(state);
   }
   for(i=0; i < nbyte; i++){
      if( fifo_is_write_ok(state, size, writeblock) ){
         cfgp->buffer[state->atomic_position.access.head] = buf[i];
         fifo_inc_head(state, size);
      } else {
         break;
      }
   }
   return i; //number of bytes written
}

void fifo_flush(fifo_state_t * state){
	state->atomic_position.access.head = 0;
	state->atomic_position.access.tail = 0;
	fifo_set_overflow(state, 0);
}


void fifo_getinfo(fifo_info_t * info, const fifo_config_t * config, fifo_state_t * state){
   //flags are the flags that the driver supports
   info->o_flags = FIFO_FLAG_SET_WRITEBLOCK | FIFO_FLAG_IS_OVERFLOW | FIFO_FLAG_INIT | FIFO_FLAG_EXIT;
   info->size = config->size;

   fifo_atomic_position_t atomic_position;
   //grab head and tail atomically in case the operation is interrupted
   atomic_position.atomic_access = state->atomic_position.atomic_access;

   if( atomic_position.access.tail == config->size ){ //check to see if buffer is full
      info->size_ready = config->size;
   } else if( atomic_position.access.head >= atomic_position.access.tail ){
      info->size_ready = atomic_position.access.head - atomic_position.access.tail;
   } else {
      info->size_ready = config->size - atomic_position.access.tail + atomic_position.access.head;
   }
   info->overflow = fifo_is_overflow(state);
   fifo_set_overflow(state, 0);
}

void fifo_data_received(const fifo_config_t * config, fifo_state_t * state){
   if( state->transfer_handler.read != 0 ){
      int bytes_read;
      if( (bytes_read = fifo_read_buffer(config,
                                         state,
                                         state->transfer_handler.read->buf,
                                         state->transfer_handler.read->nbyte)) > 0 ){
         devfs_execute_read_handler(
                  &state->transfer_handler,
                  0,
                  bytes_read,
                  MCU_EVENT_FLAG_DATA_READY);
      }
   }
}

void fifo_cancel_async_read(fifo_state_t * state){
   devfs_execute_read_handler(&state->transfer_handler, 0, -1, MCU_EVENT_FLAG_CANCELED);
}

void fifo_cancel_async_write(fifo_state_t * state){
   devfs_execute_write_handler(&state->transfer_handler, 0, -1, MCU_EVENT_FLAG_CANCELED);
}

int fifo_data_transmitted(const fifo_config_t * cfgp, fifo_state_t * state){
   int bytes_written;
   if( state->transfer_handler.write != NULL ){
      if( (bytes_written = fifo_write_buffer(cfgp, state,
                                             state->transfer_handler.write->buf_const,
                                             state->transfer_handler.write->nbyte,
                                             0)) > 0 ){
         devfs_execute_write_handler(
                  &state->transfer_handler,
                  0,
                  bytes_written,
                  MCU_EVENT_FLAG_WRITE_COMPLETE);
      }
   }

   return 1; //leave the callback in place ??
}

int fifo_open(const devfs_handle_t * handle){
   const fifo_config_t * config = handle->config;
   fifo_state_t * state = handle->state;
   return fifo_open_local(config, state);
}

int fifo_ioctl(const devfs_handle_t * handle, int request, void * ctl){
   const fifo_config_t * config = handle->config;
   fifo_state_t * state = handle->state;
   return fifo_ioctl_local(config, state, request, ctl);
}

int fifo_write(const devfs_handle_t * handle, devfs_async_t * async){
   const fifo_config_t * config = handle->config;
   fifo_state_t * state = handle->state;
   return fifo_write_local(config, state, async, 1);
}

int fifo_read(const devfs_handle_t * handle, devfs_async_t * async){
   const fifo_config_t * config = handle->config;
   fifo_state_t * state = handle->state;
   return fifo_read_local(config, state, async, 1);
}

int fifo_close(const devfs_handle_t * handle){
   const fifo_config_t * config = handle->config;
   fifo_state_t * state = handle->state;
   return fifo_close_local(config, state);
}


int fifo_open_local(const fifo_config_t * config, fifo_state_t * state){
   return 0;
}

int fifo_close_local(const fifo_config_t * config, fifo_state_t * state){
   return 0;
}

int fifo_ioctl_local(const fifo_config_t * config, fifo_state_t * state, int request, void * ctl){
   fifo_attr_t * attr = ctl;
   mcu_action_t * action = ctl;
   switch(request){
      case I_FIFO_GETINFO:
         fifo_getinfo(ctl, config, state);
         return 0;
      case I_MCU_SETACTION:

         if( action->handler.callback == 0 ){

            if(action->o_events & MCU_EVENT_FLAG_WRITE_COMPLETE ){
               fifo_cancel_async_write(state);
            }

            if(action->o_events & MCU_EVENT_FLAG_DATA_READY ){
               fifo_cancel_async_write(state);
            }

            return 0;
         }

         //fifo doesn't store a local handler so it can't set an arbitrary action
         return SYSFS_SET_RETURN(ENOTSUP);
      case I_FIFO_INIT:
         state->transfer_handler.read = NULL;
         state->transfer_handler.write = NULL;
         /* no break */
      case I_FIFO_FLUSH:
         fifo_flush(state);
         fifo_data_transmitted(config, state); //something might be waiting to write the fifo
         return 0;
      case I_FIFO_SETATTR:
         if( attr->o_flags & FIFO_FLAG_SET_WRITEBLOCK ){
            fifo_set_writeblock(state, 1);
            if( attr->o_flags & FIFO_FLAG_IS_OVERFLOW ){
               fifo_set_writeblock(state, 0);
            }
         }


         if( fifo_is_writeblock(state) ){
            //make sure the FIFO is not currently blocked
            fifo_data_transmitted(config, state); //something might be waiting to write the fifo
         }
         return 0;
   }
   return SYSFS_SET_RETURN(EINVAL);
}

int fifo_read_local(const fifo_config_t * config, fifo_state_t * state, devfs_async_t * async, int allow_callback){
   int bytes_read;

   DEVFS_DRIVER_IS_BUSY(state->transfer_handler.read, async);

   bytes_read = fifo_read_buffer(config, state, async->buf, async->nbyte); //see if there are bytes in the buffer
   if ( bytes_read == 0 ){
      if( (async->flags & O_NONBLOCK) || (state->atomic_position.access.tail == config->size) ){
         bytes_read = SYSFS_SET_RETURN(EAGAIN);
      }
   } else if( (bytes_read > 0) && allow_callback ){
      //see if anything needs to write the FIFO
      fifo_data_transmitted(config, state);
   }

   if( bytes_read != 0 ){
      state->transfer_handler.read = 0;
   }

   return bytes_read;
}

int fifo_write_local(const fifo_config_t * config, fifo_state_t * state, devfs_async_t * async, int allow_callback){
   int bytes_written;
   int non_blocking;

   DEVFS_DRIVER_IS_BUSY(state->transfer_handler.write, async);

   non_blocking = ((async->flags & O_NONBLOCK) != 0);
   bytes_written = fifo_write_buffer(config, state, async->buf_const, async->nbyte, non_blocking); //see if there are bytes in the buffer
   if ( bytes_written == 0 ){
      if( non_blocking && fifo_is_writeblock(state) ){
         bytes_written = SYSFS_SET_RETURN(EAGAIN);
      }
   } else if( (bytes_written > 0) && allow_callback ){
      fifo_data_received(config, state);
   }

   if( bytes_written != 0 ){
      state->transfer_handler.write = 0;
   }

   return bytes_written;
}

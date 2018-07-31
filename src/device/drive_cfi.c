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
#include <unistd.h>
#include "mcu/mcu.h"
#include "mcu/pio.h"
#include "mcu/spi.h"
#include "mcu/debug.h"

#include "drive_cfi_local.h"

#if 0

//utility functions
static int drive_cfi_load_sfdp(const devfs_handle_t * handle);
static void drive_cfi_write_command(const devfs_handle_t * handle, u8 opcode);
static void drive_cfi_write_command_with_address(const devfs_handle_t * handle, u8 opcode, u32 addr);
static void drive_cfi_assert_cs(const devfs_handle_t * handle);
static void drive_cfi_deassert_cs(const devfs_handle_t * handle);

//event handlers
static int drive_cfi_handle_write_complete(void * context, const mcu_event_t * event);


//CFI commands
static void drive_cfi_reset(const devfs_handle_t * handle);
static void drive_cfi_write_enable(const devfs_handle_t * handle);
static void drive_cfi_write_disable(const devfs_handle_t * handle);
static void drive_cfi_erase_sector(const devfs_handle_t * handle, u32 addr);
static u8 drive_cfi_read_status(const devfs_handle_t * handle);

static void drive_cfi_chip_erase(const devfs_handle_t * handle);
static void drive_cfi_global_block_unlock(const devfs_handle_t * handle);

void drive_cfi_assert_cs(const devfs_handle_t * handle){
    devfs_handle_t pio_handle;
    const drive_cfi_config_t * config = handle->config;
    pio_handle.port = config->cs.port;
    pio_handle.config = 0;
    mcu_pio_clrmask(&pio_handle, (void*)(ssize_t)(1<<config->cs.pin));
}

void drive_cfi_deassert_cs(const devfs_handle_t * handle){
    devfs_handle_t pio_handle;
    const drive_cfi_config_t * config = handle->config;
    pio_handle.port = config->cs.port;
    pio_handle.config = 0;
    mcu_pio_setmask(&pio_handle, (void*)(ssize_t)(1<<config->cs.pin));
}

void drive_cfi_write_command_with_address(const devfs_handle_t * handle, u8 opcode, u32 addr){
    u8 * addrp = (u8*)&addr;
    //send the opcode
    mcu_spi_swap(handle, (void*)(ssize_t)opcode);
    //send the 3-byte address MSB to LSB (assumes little-endian arch)
    mcu_spi_swap(handle, (void*)(ssize_t)addrp[2]);
    mcu_spi_swap(handle, (void*)(ssize_t)addrp[1]);
    mcu_spi_swap(handle, (void*)(ssize_t)addrp[0]);
}

void drive_cfi_write_command(const devfs_handle_t * handle, u8 opcode){
    drive_cfi_assert_cs(handle);
    mcu_spi_swap(handle, (void*)(ssize_t)opcode);
    drive_cfi_deassert_cs(handle);
}


void drive_cfi_reset(const devfs_handle_t * handle){
    drive_cfi_write_command(handle, CFI_COMMAND_ENABLE_RESET);
    drive_cfi_write_command(handle, CFI_COMMAND_RESET_DEVICE);
}

void drive_cfi_write_enable(const devfs_handle_t * handle){
    drive_cfi_write_command(handle, CFI_COMMAND_WRITE_ENABLE);
}

void drive_cfi_write_disable(const devfs_handle_t * handle){
    drive_cfi_write_command(handle, CFI_COMMAND_WRITE_DISABLE);
}

#if 0
void drive_cfi_write_ebsy(const devfs_handle_t * handle){
    drive_cfi_write_command(handle, SST25VF_INS_EBSY);
}

void drive_cfi_write_dbsy(const devfs_handle_t * handle){
    drive_cfi_write_command(handle, SST25VF_INS_DBSY);
}
#endif


void drive_cfi_erase_sector(const devfs_handle_t * handle, u32 addr){
    drive_cfi_write_enable(handle);
    drive_cfi_assert_cs(handle);
    drive_cfi_write_command_with_address(handle, CFI_COMMAND_SECTOR_ERASE, addr);
    drive_cfi_deassert_cs(handle);
}

void drive_cfi_block_erase_32kb(const devfs_handle_t * handle, u32 addr /*! Any address in the 32KB block */){
    drive_cfi_state_t * state = (drive_cfi_state_t*)handle->state;
    drive_cfi_write_enable(handle);
    drive_cfi_assert_cs(handle);
    drive_cfi_write_command_with_address(handle, state->sfdp.opcode_erase_size1, addr);
    drive_cfi_deassert_cs(handle);
}

void drive_cfi_block_erase_64kb(const devfs_handle_t * handle, u32 addr /*! Any address in the 64KB block */){
    drive_cfi_state_t * state = (drive_cfi_state_t*)handle->state;
    drive_cfi_write_enable(handle);
    drive_cfi_assert_cs(handle);
    drive_cfi_write_command_with_address(handle, state->sfdp.opcode_erase_size2, addr);
    drive_cfi_deassert_cs(handle);
}

void drive_cfi_chip_erase(const devfs_handle_t * handle){
    drive_cfi_write_enable(handle);
    drive_cfi_write_command(handle, CFI_COMMAND_CHIP_ERASE);
    drive_cfi_read_status(handle);
}

u8 drive_cfi_read_status(const devfs_handle_t * handle){
    u8 status;
    drive_cfi_assert_cs(handle);
    mcu_spi_swap(handle, (void*)CFI_COMMAND_READ_STATUS);
    status = mcu_spi_swap(handle, (void*)(ssize_t)0xFF);
    drive_cfi_deassert_cs(handle);
    return status;
}

void drive_cfi_write_status(const devfs_handle_t * handle, uint8_t status){
    drive_cfi_write_enable(handle);
    drive_cfi_assert_cs(handle);
    mcu_spi_swap(handle, (void*)(ssize_t)CFI_COMMAND_WRITE_STATUS);
    mcu_spi_swap(handle, (void*)(ssize_t)status);
    drive_cfi_deassert_cs(handle);
}

void drive_cfi_power_down(const devfs_handle_t * handle){
    drive_cfi_write_command(handle, CFI_COMMAND_RELEASE_POWER_DOWN);
}

void drive_cfi_power_up(const devfs_handle_t * handle){
    drive_cfi_write_command(handle, CFI_COMMAND_RELEASE_POWER_DOWN);
}

void drive_cfi_global_block_lock(const devfs_handle_t * handle){
    drive_cfi_write_command(handle, CFI_COMMAND_GLOBAL_BLOCK_LOCK);
}

void drive_cfi_global_block_unlock(const devfs_handle_t * handle){
    //manipulate the status registers
    drive_cfi_write_command(handle, CFI_COMMAND_GLOBAL_BLOCK_UNLOCK);
}

int drive_cfi_ioctl(const devfs_handle_t * handle, int request, void * ctl){
    //drive_cfi_config_t * config = (drive_cfi_config_t*)handle->config;
    //drive_cfi_state_t * state = (drive_cfi_state_t*)handle->state;
    drive_info_t * info;
    drive_attr_t * attr;
    u32 o_flags;
    int i;
    int result;

    switch(request){
    case I_DRIVE_GETVERSION:
        return DRIVE_VERSION;

    case I_DRIVE_SETATTR:
        attr = ctl;
        o_flags = attr->o_flags;


        if( o_flags & DRIVE_FLAG_INIT ){
            //initialize the drive
            result = drive_cfi_load_sfdp(handle);
            if( result < 0 ){ return result; }

            //any other initialization?

        }

        if( o_flags & (DRIVE_FLAG_ERASE_DEVICE|DRIVE_FLAG_ERASE_BLOCKS) ){

            if( o_flags & DRIVE_FLAG_ERASE_DEVICE ){
                drive_cfi_chip_erase(handle);
            }

            if( o_flags & DRIVE_FLAG_ERASE_BLOCKS ){
                for(i=attr->start; i <= attr->end; i++){
                    drive_cfi_erase_sector(handle, attr->start);
                }
            }
        }

        if( o_flags & DRIVE_FLAG_PROTECT ){
            drive_cfi_global_block_lock(handle);
        }

        if( o_flags & DRIVE_FLAG_UNPROTECT ){
            drive_cfi_global_block_unlock(handle);
        }

        if( o_flags & DRIVE_FLAG_POWERUP ){
            drive_cfi_power_up(handle);
        }

        if( o_flags & DRIVE_FLAG_POWERDOWN ){
            drive_cfi_power_down(handle);
        }

        break;


    case I_DRIVE_ISBUSY:

        //read the status reg to see if the drive is busy

        break;

    case I_DRIVE_GETINFO:
        info = ctl;
        info->address_size = 1;

        //how much info can be pulled from the chip using JEDEC -- how much must be in the config

        return 0;
    default:
        return mcu_spi_ioctl(handle, request, ctl);
    }

    return 0;
}

int drive_cfi_handle_write_complete(void * context, const mcu_event_t * event){
    const devfs_handle_t * handle = context;
    drive_cfi_state_t * state = (drive_cfi_state_t*)handle->state;
    drive_cfi_deassert_cs(handle);
    mcu_execute_event_handler(&state->handler, MCU_EVENT_FLAG_WRITE_COMPLETE, (void*)event);
    return 0;
}

int drive_cfi_write(const devfs_handle_t * handle, devfs_async_t * async){
    drive_cfi_state_t * state = (drive_cfi_state_t*)handle->state;

    if( state->handler.callback != 0 ){
        return SYSFS_SET_RETURN(EBUSY);
    }

    //this must enforce page program sizes and not allow overlapping -- must be page aligned


    //the state handler will be used after the write is complete
    state->handler = async->handler;

    //hijack async handler to execute write complete so that CS can be deasserted
    async->handler.callback = drive_cfi_handle_write_complete;
    async->handler.context = (void*)handle;

    drive_cfi_write_enable(handle);

    drive_cfi_assert_cs(handle);
    drive_cfi_write_command_with_address(handle, CFI_COMMAND_PAGE_PROGRAM, async->loc);

    return mcu_spi_write(handle, async);
}

int drive_cfi_handle_data_ready(void * context, const mcu_event_t * event){
    const devfs_handle_t * handle = context;
    drive_cfi_state_t * state = handle->state;
    drive_cfi_deassert_cs(handle);

    //use mcu_execute_handler
    mcu_execute_event_handler(&(state->handler), MCU_EVENT_FLAG_DATA_READY, 0);
    return 0;
}


int drive_cfi_read(const devfs_handle_t * handle, devfs_async_t * async){
    drive_cfi_state_t * state = (drive_cfi_state_t*)handle->state;
    //const drive_cfi_config_t * dcfg = (const sst25vf_config_t *)(handle->config);


    if ( state->handler.callback ){
        return SYSFS_SET_RETURN(EBUSY);
    }

#if 0
    if ( rop->loc >= dcfg->size ){
        return SYSFS_SET_RETURN(EINVAL);
    }
#endif

    //the state handler will be used after the write is complete
    state->handler = async->handler;

    async->handler.context = (void*)handle;
    async->handler.callback = (mcu_callback_t)drive_cfi_handle_data_ready;

    //if ( async->loc + async->nbyte > dcfg->size ){
    //    async->nbyte = async->size - rop->loc; //update the bytes read to not go past the end of the disk
    //}

    drive_cfi_assert_cs(handle);
    drive_cfi_write_command_with_address(handle, CFI_COMMAND_FAST_READ, async->loc);
    mcu_spi_swap(handle, NULL); //dummy byte output for fast reads
    return mcu_spi_read(handle, async); //get the data
}

int drive_cfi_load_sfdp(const devfs_handle_t * handle){
    //drive_cfi_state_t * state = handle->state;

    //use the bus to populate state->sfdp
    return 0;
}


#endif



/*! @} */



#include "sos/dev/qspi.h"
#include "mcu/pio.h"
#include "mcu/debug.h"

#include "device/drive_cfi.h"


static int drive_cfi_qspi_execute_command(
		const devfs_handle_t * handle,
		u8 instruction,
		u8 dummy_cycles,
		u8 * data,
		u32 data_size,
		u32 address,
		u32 o_flags);

static int drive_cfi_qspi_execute_quick_command(
		const devfs_handle_t * handle,
		u8 instruction,
		u32 address,
		u32 o_flags);

static u8 drive_cfi_qspi_read_status(const devfs_handle_t * handle);
static int drive_initialize(const devfs_handle_t * handle);


int drive_cfi_qspi_open(const devfs_handle_t * handle){
	int result;
	const drive_cfi_config_t * config = handle->config;

	result = config->serial_device->driver.open(&config->serial_device->handle);
	if( result < 0 ){ return result; }

	//initialize if not yet initialized

	return drive_initialize(handle);
}

int drive_initialize(const devfs_handle_t * handle){
	const drive_cfi_config_t * config = handle->config;
	drive_cfi_state_t * state = handle->state;
	int result;

	if( state->is_initialized == 0 ){
		//init the CS pin if it is available
		//set serial driver attributes to defaults
		result = config->serial_device->driver.ioctl(
					&config->serial_device->handle,
					I_QSPI_SETATTR,
					0);

		if( result < 0 ){ return result; }


		if( config->qspi_flags & QSPI_FLAG_IS_OPCODE_QUAD ){
			//enter QPI mode
			drive_cfi_qspi_execute_quick_command(
						handle,
						config->opcode.enter_qpi_mode,
						0,
						0);
		}

		if( config->qspi_flags & QSPI_FLAG_IS_ADDRESS_32_BITS ){
			drive_cfi_qspi_execute_quick_command(
						handle,
						config->opcode.enter_4byte_address_mode,
						0,
						config->qspi_flags);
		}

		drive_cfi_qspi_execute_quick_command(handle, config->opcode.write_enable, 0, 0);
		drive_cfi_qspi_execute_quick_command(handle, config->opcode.unprotect, 0, 0);

		if( config->opcode.write_status != 0 && config->opcode.write_status != 0xff ){
			drive_cfi_qspi_execute_quick_command(handle, config->opcode.write_enable, 0, config->qspi_flags);
			u8 update_status = config->opcode.initial_status_value;
			drive_cfi_qspi_execute_command(handle, config->opcode.write_status, 0, &update_status, 1, 0, config->qspi_flags);
		}
	}
	state->is_initialized++;

	return 0;
}

int drive_cfi_qspi_ioctl(const devfs_handle_t * handle, int request, void * ctl){
	const drive_cfi_config_t * config = handle->config;
	drive_attr_t * attr = ctl;
	drive_info_t * info = ctl;
	int result;
	u8 status;

	switch(request){
		case I_DRIVE_GETVERSION: return DRIVE_VERSION;

		case I_DRIVE_SETATTR:
			{
				if( attr == 0 ){ return SYSFS_SET_RETURN(EINVAL); }
				u32 o_flags = attr->o_flags;

				if( o_flags & DRIVE_FLAG_INIT ){

					//set serial driver attributes to defaults
					result = drive_initialize(handle);
					if( result < 0 ){ return result; }

				}

				if( o_flags & DRIVE_FLAG_PROTECT ){
					drive_cfi_qspi_execute_quick_command(
								handle,
								config->opcode.write_enable,
								0,
								config->qspi_flags
								);
					drive_cfi_qspi_execute_quick_command(
								handle,
								config->opcode.protect,
								0,
								config->qspi_flags
								);
				}

				if( o_flags & DRIVE_FLAG_UNPROTECT ){
					drive_cfi_qspi_execute_quick_command(
								handle,
								config->opcode.write_enable,
								0,
								config->qspi_flags
								);

					drive_cfi_qspi_execute_quick_command(
								handle,
								config->opcode.unprotect,
								0,
								config->qspi_flags
								);
				}

				if( o_flags & DRIVE_FLAG_RESET ){
					drive_cfi_qspi_execute_quick_command(
								handle,
								config->opcode.enable_reset,
								0,
								config->qspi_flags
								);

					drive_cfi_qspi_execute_quick_command(
								handle,
								config->opcode.reset,
								0,
								config->qspi_flags
								);
				}

				if( o_flags & DRIVE_FLAG_POWERUP ){
					drive_cfi_qspi_execute_quick_command(
								handle,
								config->opcode.power_up,
								0,
								config->qspi_flags
								);

				}

				if( o_flags & DRIVE_FLAG_POWERDOWN ){
					drive_cfi_qspi_execute_quick_command(
								handle,
								config->opcode.power_down,
								0,
								config->qspi_flags
								);
				}

				if( o_flags & DRIVE_FLAG_ERASE_BLOCKS ){
					//erase the smallest possible section size

					u32 erase_size = attr->end - attr->start;
					u8 opcode;
					if( (erase_size >= config->opcode.sector_erase) &&
						 (attr->start % config->opcode.sector_erase == 0) ){
						erase_size = config->info.erase_sector_size;
						opcode = config->opcode.sector_erase;
					} else {
						erase_size = config->info.erase_block_size;
						opcode = config->opcode.block_erase;
					}

					drive_cfi_qspi_execute_quick_command(
								handle,
								config->opcode.write_enable,
								0,
								config->qspi_flags
								);

					drive_cfi_qspi_execute_quick_command(
								handle,
								opcode,
								attr->start + config->info.partition_start,
								QSPI_FLAG_IS_ADDRESS_WRITE | config->qspi_flags
								);

					//only one block can be erased at a time
					return erase_size;
				}

				if( (config->opcode.device_erase != 0xff) &&
					 (o_flags & DRIVE_FLAG_ERASE_DEVICE) ){
					drive_cfi_qspi_execute_quick_command(
								handle, config->opcode.write_enable,
								0,
								config->qspi_flags
								);

					drive_cfi_qspi_execute_quick_command(
								handle,
								config->opcode.device_erase,
								0,
								config->qspi_flags);

				}

			}
			break;

		case I_DRIVE_GETINFO:
			info->o_flags = DRIVE_FLAG_INIT |
					DRIVE_FLAG_RESET |
					DRIVE_FLAG_PROTECT |
					DRIVE_FLAG_UNPROTECT |
					DRIVE_FLAG_ERASE_BLOCKS |
					DRIVE_FLAG_POWERUP |
					DRIVE_FLAG_POWERDOWN |
					0;

			if( config->opcode.device_erase != 0xff ){
				info->o_events |= DRIVE_FLAG_ERASE_DEVICE;
			}

			info->o_events = MCU_EVENT_FLAG_WRITE_COMPLETE | MCU_EVENT_FLAG_DATA_READY;
			info->addressable_size = config->info.addressable_size; //one byte for each address location
			info->write_block_size = config->info.write_block_size; //can write one byte at a time
			info->num_write_blocks = config->info.num_write_blocks;
			info->erase_block_size = config->info.erase_block_size;
			info->erase_block_time = config->info.erase_block_time;
			info->erase_device_time = config->info.erase_device_time;
			info->bitrate = config->info.bitrate;
			info->page_program_size = config->opcode.page_program_size;
			info->partition_start = config->info.partition_start;

			break;

		case I_DRIVE_ISBUSY:
			status = drive_cfi_qspi_read_status(handle);
			if( (status & config->opcode.busy_status_mask) != 0 ){
				//device is busy
				return 1;
			}
			break;

	}

	return SYSFS_RETURN_SUCCESS;
}

int drive_cfi_qspi_read(const devfs_handle_t * handle, devfs_async_t * async){
	const drive_cfi_config_t * config = handle->config;

	//check for the end of the drive
	int num_blocks =
			async->nbyte / config->info.addressable_size;
	if( async->loc + num_blocks >
		 config->info.num_write_blocks ){
		num_blocks =
				config->info.num_write_blocks - async->loc;
		if( num_blocks <= 0 ){
			return SYSFS_RETURN_EOF;
		}
		async->nbyte =
				num_blocks * config->info.addressable_size;
	}

	//get ready for the read by sending the read command
	int result = drive_cfi_qspi_execute_command(
				handle,
				config->opcode.fast_read,
				config->opcode.read_dummy_cycles,
				0, //data is null because it will be ready with read()
				async->nbyte, //the number of bytes to read
				async->loc + config->info.partition_start,  //the address to read
				config->qspi_flags | QSPI_FLAG_IS_ADDRESS_WRITE
				);

	if( result < 0 ){ return result; }

	return config->serial_device->driver.read(
				&config->serial_device->handle,
				async
				);
}



int drive_cfi_qspi_write(const devfs_handle_t * handle, devfs_async_t * async){
	const drive_cfi_config_t * config = handle->config;
	//check for the end of the drive
	int num_blocks = async->nbyte / config->info.addressable_size;
	if( async->loc + num_blocks > config->info.num_write_blocks ){
		num_blocks = config->info.num_write_blocks - async->loc;
		if( num_blocks <= 0 ){
			return SYSFS_RETURN_EOF;
		}
		async->nbyte = num_blocks * config->info.addressable_size;
	}

	u32 page_size = config->opcode.page_program_size;
	u32 page_program_mask = page_size-1;

	if( ((async->loc & page_program_mask) + async->nbyte) > page_size ){
		//allow a partial page program but don't allow overflow
		async->nbyte = page_size - (async->loc & page_program_mask);
	}

	drive_cfi_qspi_execute_quick_command(handle, config->opcode.write_enable, 0, config->qspi_flags);

	//write enable instruction
	int result = drive_cfi_qspi_execute_command(
				handle,
				config->opcode.page_program,
				config->opcode.write_dummy_cycles,
				0, //no data
				async->nbyte,
				async->loc + config->info.partition_start,
				config->qspi_flags | QSPI_FLAG_IS_ADDRESS_WRITE
				);

	if( result < 0 ){
		return result;
	}

	return config->serial_device->driver.write(&config->serial_device->handle, async);

}

int drive_cfi_qspi_close(const devfs_handle_t * handle){
	drive_cfi_state_t * state = handle->state;
	if( state->is_initialized ){ state->is_initialized--; }
	return mcu_qspi_close(handle);
}


u8 drive_cfi_qspi_read_status(const devfs_handle_t * handle){
	const drive_cfi_config_t * config = handle->config;
	u8 status;
	int result = drive_cfi_qspi_execute_command(
				handle,
				config->opcode.read_busy_status,
				0,
				&status,
				1,
				0,
				config->qspi_flags | QSPI_FLAG_IS_DATA_READ
				);

	if( result < 0 ){
		return 0xff;
	}

	return status;
}

int drive_cfi_qspi_execute_quick_command(
		const devfs_handle_t * handle,
		u8 instruction,
		u32 address,
		u32 o_flags){
	return drive_cfi_qspi_execute_command(
				handle,
				instruction,
				0,
				0,
				0,
				address,
				o_flags
				);
}

int drive_cfi_qspi_execute_command(
		const devfs_handle_t * handle,
		u8 instruction,
		u8 dummy_cycles,
		u8 * data,
		u32 data_size,
		u32 address,
		u32 o_flags){
	const drive_cfi_config_t * config = handle->config;

	qspi_command_t command;
	command.o_flags =	o_flags | QSPI_FLAG_IS_OPCODE_WRITE;
	command.opcode = instruction;
	command.dummy_cycles = dummy_cycles;
	command.data_size = data_size;
	command.address = address;
	if( o_flags & QSPI_FLAG_IS_DATA_WRITE ){
		memcpy(command.data, data, data_size);
	}
	int result = config->serial_device->driver.ioctl(&config->serial_device->handle, I_QSPI_EXECCOMMAND, &command);
	if( (result == 0) &&
		 (o_flags & QSPI_FLAG_IS_DATA_READ) ){
		memcpy(data,
				 command.data,
				 data_size);
	}

	return result;
}

// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEV_CFIFO_H_
#define DEV_CFIFO_H_

#include "fifo.h"
#include "sos/dev/cfifo.h"

typedef struct MCU_PACK {
	u32 * owner_array;
	fifo_state_t * fifo_state_array;
} cfifo_state_t;

/*! \brief MCFIFO Configuration
 * \details This structure defines the static MCFIFO configuration.
 *
 */
typedef struct MCU_PACK {
	u16 count /*! The number of channels */;
	u16 size /*! The size of the buffer (only size-1 is usable) */;
	const fifo_config_t * fifo_config_array;
} cfifo_config_t;

int cfifo_open(const devfs_handle_t * handle);
int cfifo_ioctl(const devfs_handle_t * handle, int request, void * ctl);
int cfifo_read(const devfs_handle_t * handle, devfs_async_t * async);
int cfifo_write(const devfs_handle_t * handle, devfs_async_t * async);
int cfifo_close(const devfs_handle_t * handle);

#define CFIFO_DECLARE_CONFIG_STATE_2(cfifo_name, cfifo_size) \
    fifo_state_t fifo_name##_state MCU_SYS_MEM; \
    static char cfifo_name##_buffer[2][cfifo_size]; \
    const fifo_config_t cfifo_name##_config_fifo_array[2] = { \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[0] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[1] } \
    }; \
    const cfifo_config_t cfifo_name##_config = { \
        .count = 2, \
        .size = cfifo_size, \
        .fifo_config_array = cfifo_name##_config_fifo_array \
    }; \
    fifo_state_t cfifo_name##_state_fifo_array[2]; \
    cfifo_state_t board_fifo_state = { \
        .fifo_state_array = cfifo_name##_state_fifo_array \
    }

#define CFIFO_DECLARE_CONFIG_STATE_4(cfifo_name, cfifo_size) \
    fifo_state_t fifo_name##_state MCU_SYS_MEM; \
    static char cfifo_name##_buffer[4][cfifo_size]; \
    const fifo_config_t cfifo_name##_config_fifo_array[4] = { \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[0] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[1] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[2] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[3] } \
    }; \
    const cfifo_config_t cfifo_name##_config = { \
        .count = 4, \
        .size = cfifo_size, \
        .fifo_config_array = cfifo_name##_config_fifo_array \
    }; \
    fifo_state_t cfifo_name##_state_fifo_array[4]; \
    cfifo_state_t board_fifo_state = { \
        .fifo_state_array = cfifo_name##_state_fifo_array \
    }

#define CFIFO_DECLARE_CONFIG_STATE_8(cfifo_name, cfifo_size) \
    fifo_state_t fifo_name##_state MCU_SYS_MEM; \
    static char cfifo_name##_buffer[8][cfifo_size]; \
    const fifo_config_t cfifo_name##_config_fifo_array[8] = { \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[0] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[1] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[2] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[3] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[4] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[5] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[6] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[7] } \
    }; \
    const cfifo_config_t cfifo_name##_config = { \
        .count = 8, \
        .size = cfifo_size, \
        .fifo_config_array = cfifo_name##_config_fifo_array \
    }; \
    fifo_state_t cfifo_name##_state_fifo_array[8]; \
    cfifo_state_t board_fifo_state = { \
        .fifo_state_array = cfifo_name##_state_fifo_array \
    }

#define CFIFO_DECLARE_CONFIG_STATE_16(cfifo_name, cfifo_size) \
    fifo_state_t fifo_name##_state MCU_SYS_MEM; \
    static char cfifo_name##_buffer[16][cfifo_size]; \
    const fifo_config_t cfifo_name##_config_fifo_array[16] = { \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[0] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[1] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[2] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[3] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[4] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[5] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[6] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[7] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[8] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[9] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[10] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[11] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[12] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[13] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[14] }, \
        { .size = cfifo_size, .buffer = cfifo_name##_buffer[15] } \
    }; \
    const cfifo_config_t cfifo_name##_config = { \
        .count = 16, \
        .size = cfifo_size, \
        .fifo_config_array = cfifo_name##_config_fifo_array \
    }; \
    fifo_state_t cfifo_name##_state_fifo_array[16]; \
    cfifo_state_t board_fifo_state = { \
        .fifo_state_array = cfifo_name##_state_fifo_array \
    }




#endif /* DEV_CFIFO_H_ */


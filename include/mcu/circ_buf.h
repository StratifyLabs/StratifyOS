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

/*! \addtogroup CIRC_BUF Circular Buffers
 * @{
 *
 * \ingroup UTIL
 *
 * \details The HWPL module provides an interface to use circular buffers.  A circular buffer
 * is used when reading/writing the UART.  This API is made available to any other code needing
 * a circular buffer.
 *
 */

/*! \file
 * \brief Circular Buffer Header File
 */

#ifndef CIRCBUF_H_
#define CIRCBUF_H_

#include "arch.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Cirucular Buffer Data Structure
 * \details This is the memory structure used by a circular buffer.
 *
 */
typedef struct {
	int size /*! The size of the circular buffer */;
	volatile int head /*! The head of the buffer */;
	volatile int tail /*! The tail of the buffer */;
	volatile char * buffer /*! A pointer to the circular buffer to be used */;
} circ_buf_t;

/*! \details This scans the circular buffer for the specified byte.
 * This is useful to see if a new line has been received.
 */
int circ_buf_find_byte(circ_buf_t * cb, char byte);

/*! \details This gets a byte from the buffer and updates the tail.
 * \return The byte retrieved from the buffer.
 */
char circ_buf_get(circ_buf_t * cb);

/*! \details This checks to see if new data is available in the buffer.
 * \return Non-zero if data is available.
 */
static inline int circ_buf_rdy(circ_buf_t * cb) MCU_ALWAYS_INLINE;

/*! \details This checks to see fi the buffer is empty.
 * \return Non-zero if buffer is empty.
 */
static inline int circ_buf_empty(circ_buf_t * cb) MCU_ALWAYS_INLINE;

/*! \details This function reads how many bytes are ready to
 * read from the buffer.
 * \return The number of bytes in the buffer that are ready to read.
 */
int circ_buf_bytes_ready(circ_buf_t * cb);


static inline void circ_buf_put(circ_buf_t * cb, char b) MCU_ALWAYS_INLINE;

int circ_buf_rdy(circ_buf_t * cb){ return (cb->head != cb->tail); }
int circ_buf_empty(circ_buf_t * cb){ return (cb->head == cb->tail); }

/*!
 * \details This puts a byte on the buffer.
 *
 */
void circ_buf_put(circ_buf_t * cb /*! A pointer to the cirular buffer */,
		char b /*! The byte to put */){
	int tmp;
	tmp = cb->head + 1;
	if ( tmp == cb->size ){
		tmp = 0;
	}
	cb->buffer[cb->head] = b;
	cb->head = tmp;
}

typedef struct {
	uint8_t size /*! The size of the circular buffer */;
	volatile uint8_t head /*! The head of the buffer */;
	volatile uint8_t tail /*! The tail of the buffer */;
	volatile char * buffer /*! A pointer to the circular buffer to be used */;
} circ_buf8_t;


void circ_buf8_put(circ_buf8_t * cb, char b);
char circ_buf8_get(circ_buf8_t * cb);
static inline char circ_buf8_rdy(circ_buf8_t * cb) MCU_ALWAYS_INLINE;
char circ_buf8_rdy(circ_buf8_t * cb){ return (cb->head != cb->tail); }

#ifdef __cplusplus
}
#endif

#endif /* CIRCBUF_H_ */

/*! @} */

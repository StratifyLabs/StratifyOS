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


/*! \addtogroup aio
 * @{
 */

/*! \file */

#ifndef AIO_H_
#define AIO_H_

#include <sys/signal.h>
#include <sys/types.h>
#include "sos/fs/types.h"

#ifdef __cplusplus
extern "C" {
#endif


/*! \brief AIO Data Structure
 * \details This is the data structure used
 * when performing asynchronous reads or writes.
 *
 */
struct aiocb {
	int aio_fildes /*! \brief The file descriptor */;
	off_t aio_offset /*! \brief The file offset */;
	volatile void * aio_buf /*! \brief The read/write buffer */;
	size_t aio_nbytes /*! \brief The number of bytes to read or write */;
	int aio_reqprio /*! \brief The AIO request priority */;
	struct sigevent aio_sigevent /*! \brief The AIO sigevent */;
	int aio_lio_opcode /*! \brief The AIO list opcode */;
    devfs_async_t async;
	//list
	//items in list
};

#define AIO_ALLDONE 1
#define AIO_CANCELED 2
#define AIO_NOTCANCELED 3
#define LIO_NOP 4
#define LIO_NOWAIT 5
#define LIO_READ 6
#define LIO_WAIT 7
#define LIO_WRITE 8

int aio_cancel(int fildes, struct aiocb * aiocbp);
int aio_error(const struct aiocb * aiocbp);
int aio_fsync(int, struct aiocb * aiocbp);
int aio_read(struct aiocb * aiocbp);
ssize_t aio_return(struct aiocb * aiocbp);
int aio_suspend(struct aiocb *const list[], int nent, const struct timespec * timeout);
int aio_write(struct aiocb * aiocbp);
int lio_listio(int mode, struct aiocb * const list[], int nent, struct sigevent * sig);

#ifdef __cplusplus
}
#endif

#endif /* AIO_H_ */

/*! @} */

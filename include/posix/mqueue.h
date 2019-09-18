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

#ifndef MQUEUE_H_
#define MQUEUE_H_

#include <sys/types.h>
#include <signal.h>
#include <limits.h>
#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \details This is the message queue handler type.
 *
 */
typedef ssize_t mqd_t;


//as a special case LOOP around when opened with APPEND
#define MQ_FLAGS_LOOP O_APPEND

/*! \details This structure defines the attributes
 * for a message queue.
 */
struct mq_attr {
	long mq_flags /*! message queue flags */;
	long mq_maxmsg /*! the maximum number of messages */;
	long mq_msgsize /*! the message size */;
	long mq_curmsgs /*! number of messages currently queued */;
};

/*! \details This defines the maximum
 * priority that can be assigned to a message.
 * \hideinitializer
 */
#define MQ_PRIO_MAX INT_MAX


int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat);
int mq_setattr(mqd_t mqdes, const struct mq_attr * mqstat, struct mq_attr * omqstat);

mqd_t mq_open(const char * name, int oflag, ...);
int mq_close(mqd_t mqdes);
int mq_unlink(const char * name);

int mq_notify(mqd_t mqdes, const struct sigevent *notification);

ssize_t mq_receive(mqd_t mqdes, char * msg_ptr, size_t msg_len, unsigned * msg_prio);
ssize_t mq_timedreceive(mqd_t mqdes, char * msg_ptr, size_t msg_len, unsigned * msg_prio, const struct timespec * abs_timeout);
int mq_send(mqd_t mqdes, const char * msg_ptr, size_t msg_len, unsigned msg_prio);
int mq_timedsend(mqd_t mqdes, const char * msg_ptr, size_t msg_len, unsigned msg_prio,const struct timespec * abs_timeout);



//non standard access (for trace)
void mq_discard(mqd_t mqdes);
void mq_flush(mqd_t mqdes);

ssize_t mq_tryreceive(mqd_t mqdes, char * msg_ptr, size_t msg_len, unsigned * msg_prio);
int mq_trysend(mqd_t mqdes, const char * msg_ptr, size_t msg_len, unsigned msg_prio);


#ifdef __cplusplus
}
#endif


#endif /* MQUEUE_H_ */

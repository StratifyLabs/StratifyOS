// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef MQUEUE_H_
#define MQUEUE_H_

#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \details This is the message queue handler type.
 *
 */
typedef ssize_t mqd_t;

// as a special case LOOP around when opened with APPEND
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
int mq_setattr(mqd_t mqdes, const struct mq_attr *mqstat, struct mq_attr *omqstat);

mqd_t mq_open(const char *name, int oflag, ...);
int mq_close(mqd_t mqdes);
int mq_unlink(const char *name);

int mq_notify(mqd_t mqdes, const struct sigevent *notification);

ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio);
ssize_t mq_timedreceive(
  mqd_t mqdes,
  char *msg_ptr,
  size_t msg_len,
  unsigned *msg_prio,
  const struct timespec *abs_timeout);
int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio);
int mq_timedsend(
  mqd_t mqdes,
  const char *msg_ptr,
  size_t msg_len,
  unsigned msg_prio,
  const struct timespec *abs_timeout);

// non standard access (for trace)
void mq_discard(mqd_t mqdes);
void mq_flush(mqd_t mqdes);

ssize_t mq_tryreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio);
int mq_trysend(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio);

#ifdef __cplusplus
}
#endif

#endif /* MQUEUE_H_ */

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

/*! \addtogroup mqueue
 * @{
 *
 * \details This interface allows processes to share data by sending and receiving messages.
 *
 * Here is an example of how to create a new message queue and put a message in the new queue:
 *
 * \code
 * #include <mqueue.h>
 * #include <stdio.h>
 *
 * void create_queue_and_send_msg(){
 * 	mqd_t mdes;
 * 	struct mq_attr attr;
 * 	char msg[32];
 *
 * 	attr.mq_maxmsg = 4;
 * 	attr.mq_msgsize = 32;
 *
 * 	mdes = mq_open("/path/to/queue", O_CREAT|O_EXCL|O_RDWR, 0666, &attr);
 * 	if ( mdes == (mqd_t)-1 ){
 * 		perror("failed to create queue");
 * 		return;
 * 	}
 *
 * 	//now send a message in the queue
 * 	strcpy(msg, "this is the message");
 * 	if ( mq_send(mdes, msg, strlen(msg), 0) < 0 ){
 * 		perror("failed to send message");
 * 	}
 *
 * }
 *
 * \endcode
 *
 * Another process can read the message in the queue my using the following code:
 *
 * \code
 * #include <mqueue.h>
 * #include <stdio.h>
 *
 * void read_msg(){
 * 	mqd_t mdes;
 * 	char msg[32];
 * 	unsigned msg_prio;
 *
 * 	mdes = mq_open("/path/to/queue", O_RDWR);
 * 	if ( mdes == (mqd_t)-1 ){
 * 		perror("failed to create queue");
 * 		return;
 * 	}
 *
 * 	//now send a message in the queue
 * 	if ( mq_receive(mdes, msg, strlen(msg), &msg_prio) < 0 ){
 * 		perror("failed to send message");
 * 	}
 *
 * 	printf("received: %s\n", msg);
 *
 * }
 * \endcode
 */

/*! \file */

#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syslimits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include "sos/fs/sysfs.h"

#include "mcu/debug.h"
#include "mqueue.h"
#include "../scheduler/scheduler_local.h"

//#define MSG_RD_ONLY 0
//#define MSG_RDWR 1
//#define MSG_NONBLOCK 2
//#define MSG_FLAGS_MASK 0x03

/*! \cond */
#define MSG_FILE_HDR_SIGNATURE 0x0A5518664
#define MSG_FILE_HDR_NOT_SIGNATURE (~MSG_FILE_HDR_SIGNATURE)

struct message {
	int prio;
	int size;
	int age;
	//! \todo Add a checksum to the message -- generate on send and check on receive
};

#define MQ_STATUS_REFS_MASK (0xFFFF)
#define MQ_STATUS_UNLINK_ON_CLOSE_MASK (1<<16)
#define MQ_STATUS_NONBLOCK_MASK (1<<17)
#define MQ_STATUS_RDWR_MASK (1<<18)
#define MQ_STATUS_LOOP_MASK (1<<19)

typedef struct {
	size_t max_size; //maximum message size
	size_t max_msgs; //maximum number of messages
	size_t age; //The running age -- 0 is the oldest message
	int mode; //not currently implemented
	char name[NAME_MAX]; //The name of the queue
	struct message * msg_table; //a pointer to the message table
	uint32_t status; //how many tasks are accessing the message queue, other flags
	pthread_mutex_t mutex;
} mq_t;

typedef struct {
	mq_t mq;
	void * next;
} mq_list_t;

static mq_list_t * mq_first = 0;

//static void root_send(void * args) MCU_ROOT_EXEC_CODE;
//static void root_receive(void * args) MCU_ROOT_EXEC_CODE;
static void root_wake_blocked(void * args) MCU_ROOT_EXEC_CODE;
static void root_block_on_mq(void * args) MCU_ROOT_EXEC_CODE;


static int mq_entry_size(const mq_t * mq){
	return sizeof(struct message) + mq->max_size;
}

static struct message * mq_next_message(const struct message * msg, int entry_size){
	const void * ptr;
	ptr = msg + entry_size;
	return (struct message *)ptr;
}


static mq_t * mq_find_named(const char * name){
	mq_list_t * entry;
	for(entry = mq_first; entry != 0; entry = entry->next){
		if( entry->mq.msg_table != 0 ){
			if( strncmp(entry->mq.name, name, NAME_MAX) == 0 ){
				return &entry->mq;
			}
		}
	}
	return 0;
}

static ssize_t mq_cur_msgs(const mq_t * mq){
	ssize_t i;
	ssize_t count = 0;

	struct message * imsg = mq->msg_table;
	int entry_size = mq_entry_size(mq);

	for(i=0; i < mq->max_msgs; i++){
		if( imsg->size != 0 ){
			count++;
		}
		imsg = mq_next_message(imsg, entry_size);
	}
	return count;
}

static void * mq_message_data(struct message * msg){
	void * ptr = msg;
	return ptr + sizeof(struct message);
}

static mq_t * mq_find_free(){
	mq_list_t * entry;
	mq_list_t * new_entry;
	mq_list_t * last_entry;
	last_entry = 0;
	for(entry = mq_first; entry != 0; entry = entry->next){
		last_entry = entry;
		if( entry->mq.msg_table == 0 ){
			return &entry->mq;
		}
	}

	//no free message queues
	new_entry = _malloc_r(sos_task_table[0].global_reent, sizeof(mq_list_t));
	if( new_entry == 0 ){
		return 0;
	}

	if( last_entry == 0 ){
		mq_first = new_entry;
	} else {
		last_entry->next = new_entry;
	}
	new_entry->next = 0;
	return &new_entry->mq;
}

static void mq_init_table(mq_t * mq){
	int i;
	struct message * imsg = mq->msg_table;
	int entry_size = mq_entry_size(mq);
	for(i=0; i < mq->max_msgs; i++){
		imsg->size = 0;
		imsg = mq_next_message(imsg, entry_size);
	}
}

static struct message * mq_find_oldest_highest(const mq_t * mq){
	int i;
	struct message * msg;
	int max_prio = 0;
	int max_age = INT_MAX;
	struct message * imsg = mq->msg_table;
	int entry_size = mq_entry_size(mq);

	msg = 0;
	//find the oldest, highest priority message from the queue
	for(i=0; i < mq->max_msgs; i++ ){
		if( imsg->size != 0 ){
			if ( imsg->prio > max_prio ){
				max_prio = imsg->prio;
				max_age = imsg->age;
				msg = imsg;
			} else if ( imsg->prio == max_prio ){
				if ( (imsg->age < max_age) ){
					max_prio = imsg->prio;
					max_age = imsg->age;
					msg = imsg;
				}
			}
		}
		imsg = mq_next_message(imsg, entry_size);
	}
	return msg;
}

static struct message * mq_find_free_msg(const mq_t * mq){
	int i;
	struct message * imsg = mq->msg_table;
	int entry_size = mq_entry_size(mq);
	for(i=0; i < mq->max_msgs; i++){
		if( imsg->size == 0 ){
			return imsg;
		}
		imsg = mq_next_message(imsg, entry_size);
	}

	return 0;
}


static int mq_init_mutex(mq_t * mq){
	pthread_mutexattr_t mutexattr;

	if( pthread_mutexattr_init(&mutexattr) < 0 ){
		return -1;
	}

	pthread_mutexattr_setpshared(&mutexattr, true);
	pthread_mutexattr_setprioceiling(&mutexattr, 25);

	if ( pthread_mutex_init(&mq->mutex, &mutexattr) ){
		return -1;
	}

	return 0;

}


static mq_t * mq_get_ptr(mqd_t des){
	mq_t * ptr = (mq_t*)des;
	mq_list_t * entry;
	for(entry = mq_first; entry != 0; entry = entry->next){
		if( ptr == &(entry->mq) ){
			return ptr;
		}
	}
	errno = EBADF;
	return 0;
}

typedef struct {
	uint32_t signature;
	uint32_t not_signature;
	mq_t * mq;
} msg_file_hdr_t;

static void check_for_blocked_task(void * block);
static int block_on_mq(void * block, const struct timespec * abs_timeout);

typedef struct {
	void * block;
	struct mcu_timeval abs_timeout;
} root_block_on_mq_t;

typedef struct {
	mq_t * mq;
	size_t msg_len;
	int size;
	int entry_size;
	struct message * new_msg;
} root_send_receive_t;
/*! \endcond */

/*! \details This function gets the message queue attributes and stores them at \a mqstat.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EBADF: \a mqdes is not a valid message queue descriptor
 *
 */
int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat){
	mq_t * mq = mq_get_ptr(mqdes);
	if ( mq == NULL ){
        errno = EBADF;
        mcu_debug_log_error(MCU_DEBUG_MQUEUE, "Cannot get attr of null");
		return -1;
	}

	//read the mq in priv mode
    mcu_debug_log_error(MCU_DEBUG_MQUEUE, "Cannot get attr of null");
    mqstat->mq_maxmsg = mq->max_msgs;
	mqstat->mq_msgsize = mq->max_size;
	mqstat->mq_curmsgs = mq_cur_msgs(mq);

	mqstat->mq_flags = 0;

	if ( mq->status & MQ_STATUS_NONBLOCK_MASK ){
		mqstat->mq_flags |= O_NONBLOCK;
	}

	if ( mq->status & MQ_STATUS_RDWR_MASK ){
		mqstat->mq_flags |= O_RDWR;
	}

	return 0;
}

/*! \details This function is not supported.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - ENOTSUP
 *
 */
int mq_setattr(mqd_t mqdes, const struct mq_attr * mqstat, struct mq_attr * omqstat){
    if( omqstat ){
        if( mq_getattr(mqdes, omqstat) < 0 ){
            return -1;
        }
    }

    mq_t * mq = mq_get_ptr(mqdes);
    if( mq == 0 ){
        errno = EBADF;
        return -1;
    }

    //update the flags -- other properties are ignored
    if( mqstat->mq_flags & O_NONBLOCK ){
        mq->status |= MQ_STATUS_NONBLOCK_MASK;
    } else {
        mq->status &= ~MQ_STATUS_NONBLOCK_MASK;
    }

    if ( (mqstat->mq_flags & O_WRONLY) || (mqstat->mq_flags & O_RDWR) ){
        mq->status |= MQ_STATUS_RDWR_MASK;
    } else {
        mq->status &= ~MQ_STATUS_RDWR_MASK;
    }

    if( mqstat->mq_flags & MQ_FLAGS_LOOP ){
        mq->status |= MQ_STATUS_LOOP_MASK;
    } else {
        mq->status &= ~MQ_STATUS_LOOP_MASK;
    }


    return 0;
}


/*! \details This function opens or creates a new message queue.
 *
 * \param name The name of the message queue
 * \param oflag The flags to use when opening (O_CREAT, O_EXCL, O_RDWR, O_RDONLY)
 * \param mode The access mode settings for the queue
 * \param attr The message queue attributes
 *
 * If using O_CREAT, the following prototype is used:
 * \code
 * mqd_t mq_open(const char * name, int oflag, mode_t mode, const struct mq_attr * attr);
 * \endcode
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - ENAMETOOLONG:  \a name length is greater than NAME_MAX
 * - EEXIST:  O_CREAT and O_EXCL are set in \a oflag but the queue already exists
 * - ENOENT:  O_CREAT is not set in \a oflag and the queue does not exist
 * - ENOMEM:  not enough memory for the queue
 * - EACCES:  permission to create \a name queue is denied
 * - EINVAL: O_CREAT is set and \a attr is not null but \a mq_maxmsg or \a mq_msgsize is less than or equal to zero
 *
 *
 */
mqd_t mq_open(const char * name /*! the full path to the message queue */,
		int oflag /*! the flags used for opening the message queue */,
		... /*! mode_t mode, const struct mq_attr * attr when O_CREAT is set in \a oflag */){
	mq_t * new_mq;
	ssize_t mqdes;
	int action;
	mode_t mode;
	const struct mq_attr * attr;
	va_list ap;
	struct _reent * reent_ptr;
	int tmp;

	if ( strnlen(name, NAME_MAX) == NAME_MAX ){
		errno = ENAMETOOLONG;
		return -1;
	}

	//Check to see if the  message queue exists
	new_mq = mq_find_named(name);

	//Check the flags to determine the appropriate action
	if ( oflag & O_CREAT ){
		if ( oflag & O_EXCL ){
			if ( new_mq == 0 ){
				//Create the new message queue
				action = 0;
			} else {
				errno = EEXIST;
				return -1;
			}
		} else {
			if ( new_mq == 0 ){
				//Create a new message queue
				action = 0;
			} else {
				//Read the existing message queue
				action = 1;
			}
		}
	} else {
		if ( new_mq == 0 ){
			errno = ENOENT;
			return -1;
		} else {
			//Read the existing message queue
			action = 1;
		}
	}

	switch(action){
	case 0:
		va_start(ap, oflag);
		mode = va_arg(ap, mode_t);
		attr = va_arg(ap, const struct mq_attr *);
		va_end(ap);

		//check for valid message attributes
		if ( (attr->mq_maxmsg <= 0) || (attr->mq_msgsize <= 0) ){
			errno = EINVAL;
			return -1;
		}

		//Create the new message queue
		reent_ptr = sos_task_table[0].global_reent;
		new_mq = mq_find_free();
		if ( new_mq == NULL ){
			//errno is set by malloc
			return -1;
		}

		//initialize the mutex
		if( mq_init_mutex(new_mq) < 0 ){
			return -1;
		}

		new_mq->mode = mode;
		new_mq->max_msgs = attr->mq_maxmsg;
		tmp = attr->mq_msgsize & 0x03;
		if( tmp != 0 ){ //make sure the size is word aligned
			//not aligned -- add bytes to make sure it's aligned
			new_mq->max_size = attr->mq_msgsize + 4 - tmp;
		} else {
			//aligned
			new_mq->max_size = attr->mq_msgsize;
		}
		new_mq->status = 1;
		new_mq->age = 0;
		new_mq->msg_table = _calloc_r(reent_ptr, new_mq->max_msgs , (new_mq->max_size + sizeof(struct message)) );
		if ( new_mq->msg_table == NULL ){
			return -1;
		}

		mq_init_table(new_mq);
		strncpy(new_mq->name, name, NAME_MAX);
		break;

	case 1:
		//increment the reference to this mq (priv mode)
		new_mq->status++;
		break;
	}

	if ( oflag & O_NONBLOCK ){
		new_mq->status |= MQ_STATUS_NONBLOCK_MASK;
	}

	if ( (oflag & O_WRONLY) || (oflag & O_RDWR) ){
		new_mq->status |= MQ_STATUS_RDWR_MASK;
	}

	if( oflag & MQ_FLAGS_LOOP ){
		new_mq->status |= MQ_STATUS_LOOP_MASK;
	}

	mqdes = (int)new_mq;


	return mqdes;
}

/*! \details This function closes the message queue.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EBADF:  \a mqdes is not a valid descriptor
 *
 */
int mq_close(mqd_t mqdes /*! the message queue handler */){
	mq_t * mq;

	mq = mq_get_ptr(mqdes);
	if ( mqdes == 0 ){
		return -1;
	}

	mq = mq_get_ptr(mqdes);
	if( mq == 0 ){
		return -1;
	}

	if( (mq->status & (MQ_STATUS_REFS_MASK)) > 0 ){
		mq->status--;
	}

	if ( (mq->status & MQ_STATUS_REFS_MASK) == 0 ){
		//Should message queue be unlinked now?
		if ( mq->status & MQ_STATUS_UNLINK_ON_CLOSE_MASK ){
			_free_r(sos_task_table[0].global_reent, mq->msg_table);
			mq->msg_table = 0;
		}
	}

	return 0;
}

/*! \details This function deletes a message queue if no other processes
 * have it open.  If another process has the queue open, the queue will be deleted when
 * said process closes the queue using \ref mq_close().
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - ENAMETOOLONG:  length of \a name exceeds NAME_MAX
 * - EACCES: write permission to \a name is denied
 * - ENOENT:  the queue does not exist
 * - EIO:  I/O error while deleting the queue
 *
 */
int mq_unlink(const char * name /*! the full path to the message queue */){
	mq_t * mq;

	if ( strnlen(name, NAME_MAX) == NAME_MAX ){
		errno = ENAMETOOLONG;
		return -1;
	}

	mq = mq_find_named(name);
	if( mq == 0 ){
		errno = ENOENT;
		return -1;
	}

	if ( (mq->status & MQ_STATUS_REFS_MASK) == 0 ){
		_free_r(sos_task_table[0].global_reent, mq->msg_table);
		mq->msg_table = 0;
	} else {
		//Mark the message queue for deletion when all refs are done
		mq->status |= MQ_STATUS_UNLINK_ON_CLOSE_MASK;
	}

	return 0;
}

void mq_discard(mqd_t mqdes){
	mq_t * mq = mq_get_ptr(mqdes);
	if( mq == 0 ){
		return;
	}

	_free_r(sos_task_table[0].global_reent, mq->msg_table);
	mq->msg_table = 0;
}

void mq_flush(mqd_t mqdes){
	mq_t * mq = mq_get_ptr(mqdes);
	if( mq != 0 ){
		mq_init_table(mq);
	}
}



/*! \details This function is not supported.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - ENOTSUP:  feature is not supported
 *
 */
int mq_notify(mqd_t mqdes,
		const struct sigevent *notification){
	errno = ENOTSUP;
	return -1;
}

/*! \details This function removes a message from the queue and stores
 * the message at \a msg_ptr.
 *
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EAGAIN:  no message on the queue and O_NONBLOCK is set in the descriptor flags
 * - EIO:  I/O error while accessing the queue
 * - EMSGSIZE:  \a msg_len is less than the message size of the queue
 * - EBADF: \a mqdes is not a valid message queue descriptor
 *
 */
ssize_t mq_receive(mqd_t mqdes /*! the message queue handle */,
		char * msg_ptr /*! a pointer to the destination memory for the message */,
		size_t msg_len /*! the number of bytes available in \a msg_ptr (must be at least the size of the
		message size) */,
		unsigned * msg_prio /*! if not NULL, the priority of the received message is stored here */){
	return mq_timedreceive(mqdes, msg_ptr, msg_len, msg_prio, NULL);
}


/*
void root_receive(void * args){
	root_send_receive_t * p = (root_send_receive_t*)args;
	void * ptr;
	int current;

	p->size = 0;

	//current = mq_find_oldest_highest(p->mq);

	if ( current != -1 ){
		//calculate the pointer to the entry
		ptr = p->mq->msg_table;
		ptr += current * p->entry_size;
		p->new_msg = ptr;

		//Mark message as retrieved
		if ( p->msg_len < p->new_msg->size){
			//The target buffer is too small to hold the entire message
			errno = EMSGSIZE;
			p->size = -1;
		} else {
			//Remove the message from the queue
			p->size = p->new_msg->size;
			p->new_msg->size = 0;  //Mark message as received in the table
		}
	}

}
 */

/*! \details This function removes a message from the queue and stores
 * the message at \a msg_ptr.  If no messages are available, the thread is blocked
 * until either a messsage is available or the value of \a CLOCK_REALTIME is less then \a abs_timeout.
 *
 * If O_NONBLOCK is set in \a mqdes, the function returns immediately whether a message is ready
 * or not.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EAGAIN:  no message on the queue and O_NONBLOCK is set in the descriptor flags
 * - EIO:  I/O error while accessing the queue
 * - ETIMEDOUT:  \a abs_timeout was exceeded by \a CLOCK_REALTIME
 * - EMSGSIZE:  \a msg_len is less than the message size of the queue
 * - EBADF: \a mqdes is not a valid message queue descriptor
 *
 */
ssize_t mq_timedreceive(mqd_t mqdes /*! see \ref mq_receive() */,
		char * msg_ptr /*! see \ref mq_receive() */,
		size_t msg_len /*! see \ref mq_receive() */,
		unsigned * msg_prio /*! see \ref mq_receive() */,
		const struct timespec * abs_timeout /*! the absolute timeout value */){

	struct message * new_msg;
	int size;

	mq_t * mq = mq_get_ptr(mqdes);
	if( mq == 0 ){
		return -1;
	}

	size = 0;
	do {
		if( pthread_mutex_lock(&(mq->mutex)) < 0 ){
			return -1;
		}
		new_msg = mq_find_oldest_highest(mq);
		if ( new_msg != 0 ){

			//calculate the pointer to the entry
			//Mark message as retrieved
			if ( msg_len < new_msg->size ){
				//The target buffer is too small to hold the entire message
				errno = EMSGSIZE;
				size = -1;
			} else {
				//copy the message data
				memcpy(msg_ptr, mq_message_data(new_msg), new_msg->size);
				if( msg_prio != 0 ){
					*(msg_prio) = new_msg->prio;
				}

				//Remove the message from the queue
				size = new_msg->size;
				new_msg->size = 0;

			}
		} else {
			size = 0; //no data available
		}
		if( pthread_mutex_unlock(&(mq->mutex)) < 0 ){
			return -1;
		}
		if( size == 0 ){
			if( mq->status & MQ_STATUS_NONBLOCK_MASK ){
				errno = EAGAIN;
				return -1;
			}
			size = block_on_mq(mq, abs_timeout);
		}
	} while( size == 0 ); //wait for either a successful receive or an error

	if( size > 0 ){
		//message was successfully received -- now see if any threads are blocked trying to send to this queue
		check_for_blocked_task(mq);
	}

	return size;
}

/*! \details This function sends a message pointed to by \a msg_ptr.  If there is no room
 * in the queue (and O_NONBLOCK is not set in \a mqdes), the thread is blocked until
 * a message is removed from the queue.
 *
 * If O_NONBLOCK is set in \a mqdes, the function returns immediately whether a message is sent
 * or not.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EAGAIN:  no room on the queue and O_NONBLOCK is set in the descriptor flags
 * - EIO:  I/O error while accessing the queue
 * - EBADF: \a mqdes is not a valid message queue descriptor
 *
 */
int mq_send(mqd_t mqdes /*! the message queue handle */,
		const char * msg_ptr /*! a pointer to the message to be sent */,
		size_t msg_len /*! the message size (must be less than or equal to \a mq_msgsize associated with \a mqdes */,
		unsigned msg_prio /*! the priority of the message (see \ref MQ_PRIO_MAX) */){
	return mq_timedsend(mqdes, msg_ptr, msg_len, msg_prio, NULL);
}

/*
void root_send(void * args){
	root_send_receive_t * p = (root_send_receive_t*)args;
	void * ptr;
	int i;
	int oldest;

	//if this stays 0, there is no room for a message
	p->size = 0;

	ssize_t cur = mq_cur_msgs(p->mq);

	//check for a loop condition with a full queue
	if( (p->mq->status & MQ_STATUS_LOOP_MASK) != 0 ){
		//if mq is full, discard the oldest message
		if ( cur == p->mq->max_msgs ){
			//oldest = mq_find_oldest_highest(p->mq);
			ptr = p->mq->msg_table;
			ptr += oldest * p->entry_size;
			p->new_msg = ptr;
			p->new_msg->size = p->msg_len;
			p->size = p->msg_len;
			return;
		}
	}


	//normal condition -- no message is sent if queue is full
	if ( cur < p->mq->max_msgs ){
		//add the message in the first free spot
		ptr = p->new_msg;
		//Find an open spot
		for(i = 0; i < p->mq->max_msgs; i++){
			if ( p->new_msg->size == 0 ){
				//This slot is free
				p->new_msg->size = p->msg_len;
				p->size = p->msg_len;
				return;
			}
			ptr += p->entry_size;
			p->new_msg = ptr;
		}
	}
}
 */

/*! \details This function sends a message pointed to by \a msg_ptr.  If there is no room
 * in the queue (and O_NONBLOCK is not set in \a mqdes), the thread is blocked until
 * a message is removed from the queue or until the value of \a CLOCK_REALTIME exceeds
 * \a abs_timeout.
 *
 * If O_NONBLOCK is set in \a mqdes, the function returns immediately whether a message is sent
 * or not.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EAGAIN:  no message on the queue and O_NONBLOCK is set in the descriptor flags
 * - EIO:  I/O error while accessing the queue
 * - ETIMEDOUT:  \a abs_timeout was exceeded by \a CLOCK_REALTIME
 * - EBADF: \a mqdes is not a valid message queue descriptor
 *
 */
int mq_timedsend(mqd_t mqdes /*! see \ref mq_send() */,
		const char * msg_ptr /*! see \ref mq_send() */,
		size_t msg_len /*! see \ref mq_send() */,
		unsigned msg_prio /*! see \ref mq_send() */,
		const struct timespec * abs_timeout /*! the absolute timeout value */){

	mq_t * mq;
	int size;
	struct message * new_msg;

	mq = mq_get_ptr(mqdes);
	if( mq == 0 ){
		return -1;
	}

	if ( mq->status & MQ_STATUS_RDWR_MASK ){

		//Check the length of the message
		if (mq->max_size < msg_len ){
			//The message to send is too big
			errno = EMSGSIZE;
			return -1;
		}

		do {

			if( pthread_mutex_lock(&(mq->mutex)) < 0 ){
				return -1;
			}

			//if this stays 0, there is no room for a message
			size = 0;

			new_msg = mq_find_free_msg(mq);

			if( new_msg != 0 ){
				size = msg_len;
			} else if( (mq->status & MQ_STATUS_LOOP_MASK) != 0 ){
				//if mq is full, discard the oldest message
				new_msg = mq_find_oldest_highest(mq);
				size = msg_len;
			}

			if( size > 0 ){
				memcpy(mq_message_data(new_msg), msg_ptr, msg_len);
				new_msg->size = msg_len;
				new_msg->age = mq->age++;
				new_msg->prio = msg_prio;
			}

			if( pthread_mutex_unlock(&(mq->mutex)) < 0 ){
				return -1;
			}

			if( size == 0 ){
				if ( mq->status & MQ_STATUS_NONBLOCK_MASK ){
					//Non-blocking mode:  return an error
					errno = EAGAIN;
					size = -1;
				} else {
					size = block_on_mq(mq, abs_timeout);
				}
			}

		} while( size  == 0 );

		if( size > 0 ){
			//message was successfully sent -- now see if any threads are blocked trying to send to this queue
			check_for_blocked_task(mq);
		}
		return size;
	}

	errno = EACCES;
	return -1;
}

ssize_t mq_tryreceive(mqd_t mqdes, char * msg_ptr, size_t msg_len, unsigned * msg_prio){
	struct timespec abs_timeout;
	abs_timeout.tv_sec = 0;
	abs_timeout.tv_nsec = 0;
	return mq_timedreceive(mqdes, msg_ptr, msg_len, msg_prio, &abs_timeout);
}

int mq_trysend(mqd_t mqdes, const char * msg_ptr, size_t msg_len, unsigned msg_prio){
	struct timespec abs_timeout;
	abs_timeout.tv_sec = 0;
	abs_timeout.tv_nsec = 0;
	return mq_timedsend(mqdes, msg_ptr, msg_len, msg_prio, &abs_timeout);
}

/*! \cond */
void root_block_on_mq(void * args){
	root_block_on_mq_t * argsp = (root_block_on_mq_t*)args;
	scheduler_timing_root_timedblock(argsp->block, &argsp->abs_timeout);
}

int block_on_mq(void * block, const struct timespec * abs_timeout){
	root_block_on_mq_t args;
	args.block = block;
	if( abs_timeout != 0 ){
		if( (abs_timeout->tv_sec == 0) && (abs_timeout->tv_nsec == 0) ){
			errno = EAGAIN;
			return -1;
		}
	}
	scheduler_timing_convert_timespec(&args.abs_timeout, abs_timeout);
	cortexm_svcall(root_block_on_mq, &args);
	if ( scheduler_unblock_type( task_get_current() ) == SCHEDULER_UNBLOCK_SLEEP ){
		errno = ETIMEDOUT;
		return -1;
	}
	return 0;
}

void root_wake_blocked(void * args){
	int * task = (int*)args;
	int id = *task;
	scheduler_root_assert_active(id, SCHEDULER_UNBLOCK_MQ);
    scheduler_root_update_on_wake(id, task_get_priority(id));
}

void check_for_blocked_task(void * block){
	int new_thread;
	new_thread = scheduler_get_highest_priority_blocked(block);
	if ( new_thread != -1 ){
		cortexm_svcall(root_wake_blocked, &new_thread);
	} else {
		//See if any tasks need to be notified if a message was just sent
	}
}
/*! \endcond */

/*! @} */



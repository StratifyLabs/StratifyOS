#ifndef PTHREAD_MUTEX_LOCAL_H
#define PTHREAD_MUTEX_LOCAL_H

#include <pthread.h>
#include <sdk/types.h>

typedef struct {
  int id;
  pthread_mutex_t *mutex;
} pthread_mutex_root_unlock_t;

void pthread_mutex_root_unlock(pthread_mutex_root_unlock_t *args) MCU_ROOT_EXEC_CODE;

#endif // PTHREAD_MUTEX_LOCAL_H

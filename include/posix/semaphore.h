// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SEM_NAME_MAX 23

typedef struct {
  int is_initialized;
  int value;
  int pshared;
  pid_t pid;
  int references;
  char name[SEM_NAME_MAX + 1];
  int mode;
} sem_t;

#define SEM_FAILED ((void *)0)
#define SEM_VALUE_MAX INT_MAX

int sem_close(sem_t *sem);
int sem_destroy(sem_t *sem);
int sem_getvalue(sem_t *sem, int *sval);
int sem_init(sem_t *sem, int pshared, unsigned int value);
sem_t *sem_open(const char *name, int oflag, ...);
int sem_post(sem_t *sem);
int sem_timedwait(sem_t *sem, const struct timespec *timeout);
int sem_trywait(sem_t *sem);
int sem_unlink(const char *name);
int sem_wait(sem_t *sem);

#ifdef __cplusplus
}
#endif

#endif /* SEMAPHORE_H_ */

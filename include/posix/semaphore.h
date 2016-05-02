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

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_


#include <sys/syslimits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int is_initialized;
	int value;
	int pshared;
	pid_t pid;
	int references;
	char name[NAME_MAX];
	int mode;
} sem_t;

#define SEM_FAILED ((void*)0)
#define SEM_VALUE_MAX INT_MAX

int sem_close(sem_t *sem);
int sem_destroy(sem_t *sem);
int sem_getvalue(sem_t *sem, int *sval);
int sem_init(sem_t *sem, int pshared, unsigned int value);
sem_t * sem_open(const char * name, int oflag, ...);
int sem_post(sem_t *sem);
int sem_timedwait(sem_t * sem, const struct timespec * timeout);
int sem_trywait(sem_t *sem);
int sem_unlink(const char *name);
int sem_wait(sem_t *sem);

#ifdef __cplusplus
}
#endif


#endif /* SEMAPHORE_H_ */

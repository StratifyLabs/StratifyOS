// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup pthread
 * @{
 *
 */

/*! \file */

#include "config.h"

#include <pthread.h>
#include <errno.h>
#include "sched.h"

#include "../scheduler/scheduler_local.h"


/*! \cond */
typedef struct {
	int tid;
	int policy;
	const struct sched_param * param;
} root_set_pthread_scheduling_param_t;
static void svcall_set_scheduling_param(void * args) MCU_ROOT_EXEC_CODE;
/*! \endcond */

/*! \details This function gets \a thread's scheduling policy and scheduling parameters and
 * stores them in \a policy and \a param respectively.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - ESRCH:  thread is not a valid
 * - EINVAL:  policy or param is NULL
 *
 */
int pthread_getschedparam(pthread_t thread, int *policy,
								  struct sched_param *param){

	if ( scheduler_check_tid(thread) ){
		errno = ESRCH;
		return -1;
	}

	if ( (policy == NULL) || (param == NULL) ){
		errno = EINVAL;
		return -1;
	}

	*policy = PTHREAD_ATTR_GET_SCHED_POLICY( (&(sos_sched_table[thread].attr)) );
	memcpy(param, (const void *)&(sos_sched_table[thread].attr.schedparam), sizeof(struct sched_param));
	return 0;
}

/*! \details This function sets \a thread's scheduling policy and scheduling parameters to
 * \a policy and \a param respectively.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - ESRCH:  thread is not a valid
 * - EINVAL:  param is NULL or the priority is invalid
 *
 */
int pthread_setschedparam(pthread_t thread,
								  int policy,
								  struct sched_param *param){

	int min_prio;
	int max_prio;
	root_set_pthread_scheduling_param_t args;

	if ( param == NULL ){
		errno = EINVAL;
		return -1;
	}

	if ( scheduler_check_tid(thread) < 0 ){
		errno = ESRCH;
		return -1;
	}

	max_prio = sched_get_priority_max(policy);
	min_prio = sched_get_priority_min(policy);

	if ( ((uint8_t)param->sched_priority >= min_prio) &&
		  ((uint8_t)param->sched_priority <= max_prio) ){
		args.tid = thread;
		args.policy = policy;
		args.param = param;
		cortexm_svcall(svcall_set_scheduling_param, &args);
		return 0;
	}

	//The scheduling priority is invalid
	errno = EINVAL;
	return -1;
}

/*! \cond */
void svcall_set_scheduling_param(void * args){
	CORTEXM_SVCALL_ENTER();
	root_set_pthread_scheduling_param_t * p = (root_set_pthread_scheduling_param_t*)args;
	int id;
	id = p->tid;

	if( task_enabled(id) ){

		PTHREAD_ATTR_SET_SCHED_POLICY( (&(sos_sched_table[id].attr)), p->policy);
		memcpy((void*)&sos_sched_table[id].attr.schedparam, p->param, sizeof(struct sched_param));

		//Issue #161 -- need to set the effective priority -- not just the prio ceiling
		task_set_priority(id, sos_sched_table[id].attr.schedparam.sched_priority);

		if ( p->policy == SCHED_FIFO ){
			task_assert_fifo(id);
		} else {
			task_deassert_fifo(id);
		}

		if( task_enabled_active_not_stopped(id) ){
			scheduler_root_update_on_stopped();
		}
	}

#if 0
	sos_debug_log_info(SOS_DEBUG_PTHREAD, "%d:Set %d -> %d (%d)",
							 task_get_current_priority(),
							 id, sos_sched_table[id].attr.schedparam.sched_priority,
							 task_enabled_active_not_stopped(id));
#endif

}
/*! \endcond */

/*! @} */




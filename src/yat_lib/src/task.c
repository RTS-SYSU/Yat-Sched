#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <sched.h>

#include "yat.h"
#include "internal.h"

#define SCHED_NORMAL 0

int task_mode(int mode)
{
	struct sched_param param;
	int me     = gettid();
	int policy = sched_getscheduler(gettid());
	int old_mode = policy == SCHED_YAT ? YAT_RT_TASK : BACKGROUND_TASK;

	memset(&param, 0, sizeof(param));
	param.sched_priority = 0;
	if (old_mode == YAT_RT_TASK && mode == BACKGROUND_TASK) {
		/* transition to normal task */
		return sched_setscheduler(me, SCHED_NORMAL, &param);
	} else if (old_mode == BACKGROUND_TASK && mode == YAT_RT_TASK) {
		/* transition to RT task */
		return sched_setscheduler(me, SCHED_YAT, &param);
	} else {
		errno = -EINVAL;
		return -1;
	}
}

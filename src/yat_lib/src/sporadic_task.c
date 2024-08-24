#include "sporadic_task.h"

SporadicTask init_task(){
	SporadicTask task;
	task.id=0;
	task.priority=36;
	task.pure_resource_execution_time = 0;
	task.Ri = 0;
	task.spin = 0;
	task.interference = 0;
	task.local = 0;
	task.resource_num=0;
	return task;
}
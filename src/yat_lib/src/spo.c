#define MIN(a, b) ((a) < (b) ? (a) : (b))

#include "spo.h"

#define MAX_TASKS 16
// #define extendCal 5
#define RESOURCE_SIZE 20
#define useCorrection 0

// ori_task[100];
// resourse[100];
// task_num[partition_num];  // 每个partition的任务数量

// tasks[partition_num][MAX_TASKS];

// priority[partition_num][MAX_TASKS];

// set_partition();
// 根据tasks数组做分配
// set_prority();
// 根据Prority数组做分配

int compare(const void *a, const void *b) {
    // 将void指针转换为long指针，并获取其值
    long value1 = *((long*)a);
    long value2 = *((long*)b);

    if (value1 < value2) {
        return 1; // 返回正数表示降序
    } else if (value1 > value2) {
        return -1; // 返回负数表示升序
    } else {
        return 0; // 返回0表示相等
    }
}

int comparePriority(const void *a, const void *b) {
    SporadicTask *task1 = (SporadicTask *)a;
    SporadicTask *task2 = (SporadicTask *)b;

    if (task1->priority < task2->priority) return -1;
    if (task1->priority > task2->priority) return 1;
    return 0;
}

int compareSlack(const void *a, const void *b) {
    SporadicTask *task1 = (SporadicTask *)a;
    SporadicTask *task2 = (SporadicTask *)b;

    if (task1->addition_slack_BTB > task2->addition_slack_BTB) return -1;
    if (task1->addition_slack_BTB < task2->addition_slack_BTB) return 1;
    if (task1->addition_slack_BTB == task2->addition_slack_BTB) {
			if (task1->deadline > task2->deadline)
				return -1;
			if (task1->deadline < task2->deadline)
				return 1;
			if (task1->deadline == task2->deadline)
				return 0;
		}
    return 0;
}

void remove_task(SporadicTask* unassignedTasks, int index){
    for(int i=index;i<MAX_TASKS-1;i++){
        unassignedTasks[i]=unassignedTasks[i+1];
    }
    unassignedTasks[MAX_TASKS-1]=init_task();
}

int getResponseTimeSPO(SporadicTask** tasks, Resource* resources, int* task_num, int num_partitions, int isMSRP) {
    if (tasks == NULL)
        return 0;

    int extendCal = 5;
    // Assign priorities by Deadline Monotonic
    tasks = assignPrioritiesByDM(tasks, MAX_TASKS, num_partitions,task_num);

    long dummy_response_time[MAX_TASKS][MAX_TASKS];  // dummy_response_time全部赋值为deadline
    for (int i = 0; i < num_partitions; i++) {
        for (int j = 0; j < task_num[i]; j++) {
            dummy_response_time[i][j] = tasks[i][j].deadline;
        }
    }

    // Now we check each task in each processor
    for (int i = 0; i < num_partitions; i++) {
        int partition = i;
        SporadicTask unassignedTasks[MAX_TASKS];
        for (int j = 0; j < task_num[i]; j++) {
            unassignedTasks[j] = tasks[partition][j];
        }
        int sratingP = 35;
        int prioLevels = task_num[i];

        // For each priority level
        int unassign_num = task_num[partition] - 1;
        for (int currentLevel = 0; currentLevel < prioLevels; currentLevel++) {
            int startingIndex = unassign_num;

            for (int j = startingIndex; j >= 0; j--) {
                SporadicTask task = unassignedTasks[j];
                int originalP = task.priority;
                task.priority = sratingP;

                for(int index=0;index<MAX_TASKS;index++){
                    if(tasks[partition][index].id==task.id){
                        tasks[partition][index].priority=task.priority;
                        break;
                    }
                }

                // Sorting tasks based on priority
                qsort(tasks[partition], task_num[i], sizeof(SporadicTask), comparePriority);

                long timeBTB = getResponseTimeForSBPO(partition, tasks, num_partitions, task_num, resources, dummy_response_time, 1, 1, isMSRP, task, extendCal);

                task.priority = originalP;

                for(int index=0;index<MAX_TASKS;index++){
                    if(tasks[partition][index].id==task.id){
                        tasks[partition][index].priority=task.priority;
                        break;
                    }
                }

                qsort(tasks[partition], task_num[i], sizeof(SporadicTask), comparePriority);

                // Update addition_slack_BTB
                task.addition_slack_BTB = task.deadline - timeBTB;

                for(int index=0;index<MAX_TASKS;index++){
                    if(tasks[partition][index].id==task.id){
                        tasks[partition][index].addition_slack_BTB=task.addition_slack_BTB;
                        break;
                    }
                }
            }

            // Sorting tasks based on addition_slack_BTB
            for(int index=0;index<=unassign_num;index++){
                for (int index_j = 0; index_j < task_num[i]; index_j++) {
                    if(unassignedTasks[index].id==tasks[partition][index_j].id){
                        unassignedTasks[index].addition_slack_BTB=tasks[partition][index_j].addition_slack_BTB;
                    }
                }
            }
            qsort(unassignedTasks, unassign_num+1, sizeof(SporadicTask), compareSlack);

            // Update priority and remove task
            unassignedTasks[0].priority = sratingP;
            for(int index=0;index<MAX_TASKS;index++){
                if(tasks[partition][index].id==unassignedTasks[0].id){
                    tasks[partition][index].priority=unassignedTasks[0].priority;
                    break;
                }
            }
            // check correction
            /*printf("partition:%d\n", partition);
            for (int index = 0; index <= unassign_num; index++) {
                printf("id:%d  slack:%ld\n", unassignedTasks[index].id, unassignedTasks[index].addition_slack_BTB);
            }*/

            qsort(tasks[partition], task_num[i], sizeof(SporadicTask), comparePriority);

            remove_task(unassignedTasks,0);
            unassign_num--;
            sratingP -= 2;
        }
        qsort(tasks[partition], task_num[i], sizeof(SporadicTask),comparePriority);
        // Update dummy_response_time
        long* response_time_temp = malloc(MAX_TASKS * sizeof(long));
        getResponseTimeForOnePartition(partition, tasks, num_partitions, task_num, resources, dummy_response_time, response_time_temp,1, 1, isMSRP, 1);
        for (int i = 0; i < task_num[partition]; i++) {
            dummy_response_time[partition][i] = response_time_temp[i];
        }
        free(response_time_temp);
    }

    int isEqual = 0, missdeadline = 0;
    long **response_time=malloc(MAX_TASKS*sizeof(long*));
    for (int i1 = 0; i1 < MAX_TASKS; i1++) {
        response_time[i1] = malloc(MAX_TASKS * sizeof(long));
    }
    initResponseTime(tasks,num_partitions, task_num, response_time);

    /* a huge busy window to get a fixed Ri */
    while (!isEqual) {
        // printf("1");
        isEqual = 1;
        long** response_time_plus = malloc(MAX_TASKS * sizeof(long*));
        for (int i = 0; i < MAX_TASKS; i++) {
            response_time_plus[i] = malloc(MAX_TASKS * sizeof(long));
        }
        busyWindow(tasks, num_partitions, task_num, resources, response_time,response_time_plus, 1, 1, isMSRP);

        for (int i = 0; i < num_partitions; i++) {
            for (int j = 0; j < task_num[i]; j++) {
                if (response_time[i][j] != response_time_plus[i][j])
                    isEqual = 0;
                if (response_time_plus[i][j] > tasks[i][j].deadline)
                    missdeadline = 1;
            }
        }

        cloneList(response_time_plus, response_time,num_partitions,MAX_TASKS);
        for (int i = 0; i < MAX_TASKS; i++) {
            free(response_time_plus[i]);
        }
        free(response_time_plus);
        if (missdeadline)
            break;
    }

    //free memory
    for (int i1 = 0; i1 < MAX_TASKS; i1++) {
        free(response_time[i1]);
    }
    free(response_time);

    return 1;
}

long getResponseTimeForSBPO(int partition, SporadicTask **tasks,int num_partition,  int* tasks_num, Resource *resources, long response_time[][MAX_TASKS],
                            bool btbHit, bool useRi, bool isMSRP, SporadicTask calTask, int extendCal) {

    // Allocate memory for response_time_new
    long response_time_new [MAX_TASKS][MAX_TASKS];

    // Clone response_time to response_time_new
    for (int i = 0; i < num_partition; i++) {
        for (int j = 0; j < tasks_num[i]; j++) {
            response_time_new[i][j] = response_time[i][j];
        }
    }

    // Allocate memory for response_time_plus
    long** response_time_plus = malloc(MAX_TASKS * sizeof(long*));
    for (int i = 0; i < MAX_TASKS; i++) {
        response_time_plus[i] = malloc(MAX_TASKS * sizeof(long));
    }

    // Clone response_time to response_time_plus
    for (int i = 0; i < num_partition; i++) {
        for (int j = 0; j < tasks_num[i]; j++) {
            response_time_plus[i][j] = response_time[i][j];
        }
    }

    for (int i = 0; i < tasks_num[partition]; i++) {
		response_time_plus[partition][i] = tasks[partition][i].WCET + tasks[partition][i].pure_resource_execution_time;
	}

    bool isEqual = false;
    bool shouldFinish = false;

    while (!isEqual && !shouldFinish) {
        isEqual = true;

        // Clone response_time_plus to response_time_new
		for (int i = 0; i < tasks_num[partition]; i++) {
			response_time_new[partition][i] = response_time_plus[partition][i];
		}

        // Perform response time analysis
        for (int j = 0; j < tasks_num[partition]; j++) {
            SporadicTask task = tasks[partition][j];
            if (response_time_plus[partition][j] >= task.deadline * extendCal && task.id != calTask.id) {
                response_time_plus[partition][j] = task.deadline * extendCal;
                continue;
            }

            task.spin = spinDelay(task, tasks, num_partition, tasks_num, resources, response_time_plus, response_time_plus[partition][j], btbHit, useRi);
			task.interference = highPriorityInterference(task, tasks, tasks_num[partition], response_time_plus[partition][j]);
			task.local = localBlocking(task, tasks, tasks_num, partition, resources, response_time_plus, response_time_plus[partition][j], btbHit, useRi, isMSRP);
            task.Ri = task.WCET + task.spin + task.interference + task.local;
            tasks[partition][j].spin=task.spin;
            tasks[partition][j].interference=task.interference;
            tasks[partition][j].local=task.local;
            tasks[partition][j].Ri=task.Ri;
            // printf("%d %d %d %ld\n", task.spin,task.interference,task.local,task.Ri);

            response_time_plus[partition][j] = task.Ri > task.deadline * extendCal ? task.deadline * extendCal : task.Ri;
            // printf("%d %d %d %ld\n", task.spin, task.interference, task.local, response_time_plus[partition][j]);
        }

        // Check if response_time_plus is equal to response_time_new
        for (int i = 0; i < tasks_num[partition]; i++) {
            if (response_time_plus[partition][i] != response_time_new[partition][i])
                isEqual = false;
            if (tasks[partition][i].Ri < tasks[partition][i].deadline * extendCal)
                shouldFinish = false;
        }
    }

    // Return Ri of calTask
    for(int i=0;i<tasks_num[partition];i++){
        if(tasks[partition][i].id==calTask.id){
            calTask=tasks[partition][i];
        }
    }
    // free memory
    for (int i = 0; i < MAX_TASKS; i++) {
        free(response_time_plus[i]);
    }
    free(response_time_plus);
    return calTask.Ri;
}

void getResponseTimeForOnePartition(int partition, SporadicTask** tasks, int num_partition,  int* tasks_num, Resource* resources,
	long response_time[][MAX_TASKS],long* response_time_temp, bool btbHit, bool useRi, bool isMSRP, int extendCal) {

	bool isEqual = false;
    // Allocate memory for response_time_new
    long response_time_new [MAX_TASKS][MAX_TASKS];

    // Clone response_time to response_time_new
    for (int i = 0; i < num_partition; i++) {
        for (int j = 0; j < tasks_num[i]; j++) {
            response_time_new[i][j] = response_time[i][j];
        }
    }

    // Allocate memory for response_time_plus
    long** response_time_plus = malloc(MAX_TASKS * sizeof(long*));
    for (int i = 0; i < MAX_TASKS; i++) {
        response_time_plus[i] = malloc(MAX_TASKS * sizeof(long));
    }

    // Clone response_time to response_time_plus
    for (int i = 0; i < num_partition; i++) {
        for (int j = 0; j < tasks_num[i]; j++) {
            response_time_plus[i][j] = response_time[i][j];
        }
    }

	for (int i = 0; i < tasks_num[partition]; i++) {
		response_time_plus[partition][i] = tasks[partition][i].WCET + tasks[partition][i].pure_resource_execution_time;
	}

	while (!isEqual) {
		isEqual = true;

        for (int i = 0; i < tasks_num[partition]; i++) {
			response_time_new[partition][i] = response_time_plus[partition][i];
		}

		for (int j = 0; j < tasks_num[partition]; j++) {
            SporadicTask task = tasks[partition][j];
            if (response_time_plus[partition][j] >= task.deadline * extendCal) {
                response_time_plus[partition][j] = task.deadline ;
                continue;
            }

            task.spin = spinDelay(task, tasks, num_partition, tasks_num, resources, response_time_plus, response_time_plus[partition][j], btbHit, useRi);
			task.interference = highPriorityInterference(task, tasks, tasks_num[partition], response_time_plus[partition][j]);
			task.local = localBlocking(task, tasks, tasks_num, partition, resources, response_time_plus, response_time_plus[partition][j], btbHit, useRi, isMSRP);
			response_time_plus[partition][j] = task.Ri = task.WCET + task.spin + task.interference + task.local;

            tasks[partition][j].spin=task.spin;
            tasks[partition][j].interference=task.interference;
            tasks[partition][j].local=task.local;
            tasks[partition][j].Ri=task.Ri;
		}

		for (int i = 0; i < tasks_num[partition]; i++) {
			if (response_time_new[partition][i] != response_time_plus[partition][i])
				isEqual = false;
		}

	}

    for (int i = 0; i < tasks_num[partition]; i++) {
        response_time_temp[i] = response_time_plus[partition][i];
    }
    // free memory
    for (int i = 0; i < MAX_TASKS; i++) {
        free(response_time_plus[i]);
    }
    free(response_time_plus);
}

void busyWindow(SporadicTask** tasks, int num_partition,  int* tasks_num, Resource* resources, long **response_time, long ** response_time_plus, bool btbHit, bool useRi,
			bool isMSRP) {
    // Allocate memory for response_time_plus

	for (int i = 0; i < num_partition; i++) {
		for (int j = 0; j < tasks_num[i]; j++) {
			SporadicTask task = tasks[i][j];
			if (response_time[i][j] > task.deadline) {
				response_time_plus[i][j] = response_time[i][j];
				continue;
			}

			task.spin = spinDelay(task, tasks, num_partition, tasks_num, resources, response_time, response_time[i][j], btbHit, useRi);
			task.interference = highPriorityInterference(task, tasks, tasks_num[i], response_time[i][j]);
			task.local = localBlocking(task, tasks, tasks_num, i, resources, response_time, response_time[i][j], btbHit, useRi, isMSRP);
			response_time_plus[i][j] = task.Ri = task.WCET + task.spin + task.interference + task.local;
            // printf("%d %d %d %ld\n", task.spin, task.interference, task.local, task.Ri);

            tasks[i][j].spin=task.spin;
            tasks[i][j].interference=task.interference;
            tasks[i][j].local=task.local;
            tasks[i][j].Ri=task.Ri;

			if (task.Ri > task.deadline) {
				return ;
			}
		}
	}
}


long spinDelay(SporadicTask t, SporadicTask **tasks, int num_partition, int* tasks_num, Resource *resources, long **Ris, long Ri, bool btbHit, bool useRi) {
    long spin_delay = 0;

    for (int k = 0; k < RESOURCE_SIZE; k++) {
        Resource resource = resources[k];
        if (resource.id == -1)
            continue;
        int Rindex = getIndexRInTask(t, resource);
        long noqT = Rindex < 0 ? 0 : t.number_of_access_in_one_release[Rindex];
        long noqHT = getNoRFromHP(t, resource, tasks[t.partition], tasks_num[t.partition], Ris[t.partition], btbHit, useRi, Ri);
        long spin = noqT + noqHT;
        // printf("%ld %ld %ld\n", noqT,noqHT,spin);

        for (int j = 0; j < num_partition; j++) {
            if (j != t.partition) {
                int noqRemote = getNoRRemote(resource, tasks[j], tasks_num[j], Ris[j], Ri, btbHit, useRi);
                // printf("%ld\n", Ri);
                spin += MIN(noqT + noqHT, noqRemote);
            }
        }
        spin_delay += spin * resource.csl;
        // printf("spin:%ld csl:%ld spin_delay:%ld\n", spin, resource.csl, spin_delay);
    }
    return spin_delay;
}

long highPriorityInterference(SporadicTask t, SporadicTask **allTasks, int tasks_size, long Ri) {
    long interference = 0;
    int partition = t.partition;
    SporadicTask *tasks = allTasks[partition];

    for (int i = 0; i < tasks_size; i++) {
        if (tasks[i].priority < t.priority) {
            SporadicTask hpTask = tasks[i];
            interference += ceil((double)(Ri) / (double)(hpTask.period)) * (hpTask.WCET);
        }
    }
    return interference;
}

long localBlocking(SporadicTask t, SporadicTask **tasks, int* tasks_num,int partition, Resource *resources, long **Ris, long Ri, bool btbHit, bool useRi, bool isMSRP) {
    Resource* localBlockingResources = malloc(RESOURCE_SIZE * sizeof(Resource));
    localBlockingResources = isMSRP ? getLocalBlockingResourcesMSRP(t, resources, localBlockingResources,tasks[t.partition],tasks_num[t.partition]) : NULL;
    int resourse_num=0;
    resourse_num=getLocalBlockingNumMSRP(localBlockingResources);
    long local_blocking_each_resource[RESOURCE_SIZE];
    int count=0;

    for (int i = 0; i < resourse_num; i++) {
        if(localBlockingResources==NULL)
            break;
        Resource res = localBlockingResources[i];
        long local_blocking = res.csl;

        if (res.isGlobal) {
            for (int parition_index = 0; parition_index < res.partition_num; parition_index++) {
                int partition = res.partitions[parition_index];
                int norHP = getNoRFromHP(t, res, tasks[t.partition], tasks_num[t.partition], Ris[t.partition], btbHit, useRi, Ri);
                // 是否含有resourse
                int contain=0;
                int res_index=0;
                for(int j=0;j<t.resource_num;j++){
                    if(t.resource_required_index[j]==res.id-1){
                        contain=1;
                        res_index=j;
                        break;
                    }
                }
                int norT = contain ? t.number_of_access_in_one_release[res_index]: 0;
                int norR = getNoRRemote(res, tasks[partition], tasks_num[partition], Ris[partition], Ri, btbHit, useRi);

                if (partition != t.partition && (norHP + norT) < norR) {
                    local_blocking += res.csl;
                }
            }
        }
        local_blocking_each_resource[count]=local_blocking;
        count++;
    }

    if (count >= 1) {
        // 排序
        qsort(local_blocking_each_resource, count, sizeof(long), compare);
    }

    free(localBlockingResources);
    return count > 0 ? local_blocking_each_resource[0] : 0;
}

int getNoRRemote(Resource resource, SporadicTask* tasks, int tasks_size, long* Ris, long Ri, bool btbHit, bool useRi) {
	int number_of_request_by_Remote_P = 0;

	for (int i = 0; i < tasks_size; i++) {
        int contain=0;
        for(int j=0;j<tasks[i].resource_num;j++){
            if(tasks[i].resource_required_index[j]==resource.id-1){
                contain=1;
                break;
            }
        }
		if (contain) {
			SporadicTask remote_task = tasks[i];
			int indexR = getIndexRInTask(remote_task, resource);
            long temp1 = useRi ? Ris[i] : remote_task.deadline;
            /*if (temp1 < 0) {
                printf("temp1:%ld  i:%d  task_id:%d \n", temp1, i,remote_task.id);
            }*/
			number_of_request_by_Remote_P += ( (long) ceil((double) (Ri + (btbHit ? temp1 : 0)) / (double) remote_task.period))
					* (long)remote_task.number_of_access_in_one_release[indexR];
            /*if (number_of_request_by_Remote_P < 0) {
                printf("Ri:%ld\n", Ri);
                printf("%f %d %ld\n", ceil((double)(Ri + (btbHit ? temp1 : 0)) / (double)remote_task.period),
                    remote_task.number_of_access_in_one_release[indexR], number_of_request_by_Remote_P);
            }*/
		}
	}
	return number_of_request_by_Remote_P;
}


long getNoRFromHP(SporadicTask task, Resource resource, SporadicTask* tasks, int tasks_size, long* Ris, bool btbHit, bool useRi, long Ri) {
	long number_of_request_by_HP = 0;
	int priority = task.priority;

	for (int i = 0; i < tasks_size; i++) {
        int contain=0;
        for(int j=0;j<tasks[i].resource_num;j++){
            if(tasks[i].resource_required_index[j]==resource.id-1){
                contain=1;
                break;
            }
        }
		if (tasks[i].priority < priority && contain) {  // 优先级越小，优先级越高
			SporadicTask hpTask = tasks[i];
			int indexR = getIndexRInTask(hpTask, resource);
            if (indexR == -1)
                continue;
			if (useCorrection) {
				number_of_request_by_HP += (long)ceil((double) (Ri) / (double) hpTask.period) * hpTask.number_of_access_in_one_release[indexR];
			} else {
                long temp1 = useRi ? Ris[i] : hpTask.deadline;
				number_of_request_by_HP += (long)ceil((double) (Ri + (long)(btbHit ? temp1 : 0)) / (double) hpTask.period)
					* (long) hpTask.number_of_access_in_one_release[indexR];
			}
		}
	}
    // printf("%ld\n", number_of_request_by_HP);
	return number_of_request_by_HP;
}

int getIndexRInTask(SporadicTask task, Resource resource) {
	int indexR = -1;
    int contain=0;
    for(int i=0;i<task.resource_num;i++){
        if(task.resource_required_index[i]==resource.id-1){
            contain=1;
            indexR=i;
            break;
        }
    }
	return indexR;
}

int getCeilingForProcessor(Resource res, SporadicTask* tasks,int task_size) {
	int ceiling = 37;

	for (int k = 0; k < task_size; k++) {
		SporadicTask task = tasks[k];
        int contain=0;
        for(int i=0;i<task.resource_num;i++){
            if(task.resource_required_index[i]==res.id-1){
                contain=1;
                break;
            }
        }

		if (contain) {
			ceiling = task.priority < ceiling ? task.priority : ceiling;  // 优先级越小，优先级越高
		}
	}

	return ceiling;
}

Resource* getLocalBlockingResourcesMSRP(SporadicTask task, Resource* resources, Resource* localBlockingResources, SporadicTask* localTasks,int task_size) {
    for(int i=0;i<RESOURCE_SIZE;i++){
        localBlockingResources[i]=init_resource();
    }
	int partition = task.partition;

    int index=0;
	for (int i = 0; i < RESOURCE_SIZE; i++) {
		Resource resource = resources[i];
        if (resource.id == -1)
            continue;
		// local resources that have a higher ceiling
		if (resource.partition_num == 1 && resource.partitions[0] == partition && getCeilingForProcessor(resource, localTasks,task_size) <= task.priority) {
			for (int j = 0; j < resource.requested_tasks_num; j++) {
				SporadicTask LP_task = resource.requested_tasks[j];
				if (LP_task.partition == partition && LP_task.priority > task.priority) {  // 优先级越小，优先级越高
					localBlockingResources[index]=resource;
                    index++;
					break;
				}
			}
		}
        int contain=0;
        for(int ii=0;ii<resource.partition_num;ii++){
            if(resource.partitions[ii]==partition){
                contain=1;
                break;
            }
        }
		// global resources that are accessed from the partition
		if (contain && resource.partition_num > 1) {
			for (int j = 0; j < resource.requested_tasks_num; j++) {
				SporadicTask LP_task = resource.requested_tasks[j];
				if (LP_task.partition == partition && LP_task.priority > task.priority) {  // 优先级越小，优先级越高
					localBlockingResources[index]=resource;
                    index++;
					break;
			    }
			}
		}
	}

	return localBlockingResources;
}

int getLocalBlockingNumMSRP(Resource* localBlockingResources){
    int count=0;
    for(int i=0;i<RESOURCE_SIZE;i++){
        if(localBlockingResources[i].id!=-1 && localBlockingResources[i].id != 0){
            count++;
        }
    }
    return count;
}

void initResponseTime(SporadicTask** tasks, int num_partition, int* task_num, long **response_time) {

	for (int i = 0; i < num_partition; i++) {
		SporadicTask* task_on_a_partition = tasks[i];

        qsort(task_on_a_partition, task_num[i], sizeof(SporadicTask), comparePriority);


		for (int j = 0; j < task_num[i]; j++) {
			SporadicTask t = task_on_a_partition[j];
			response_time[i][j] = t.Ri = t.WCET + t.pure_resource_execution_time;
            task_on_a_partition[j].Ri=task_on_a_partition[j].WCET+task_on_a_partition[j].pure_resource_execution_time;
		}
	}
}

void cloneList(long **oldList, long **newList,int num_partitions,int col_size) {
	for (int i = 0; i < num_partitions; i++) {
		for (int j = 0; j < col_size; j++) {
			newList[i][j] = oldList[i][j];
		}
	}
}
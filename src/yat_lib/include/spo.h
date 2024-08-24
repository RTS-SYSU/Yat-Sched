#pragma once

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "sporadic_task.h"
#include "resource.h"
#include "priority_generator.h"

#define MAX_TASKS 16

int compare(const void *a, const void *b);

int comparePriority(const void *a, const void *b);

int compareSlack(const void *a, const void *b);

void remove_task(SporadicTask* unassignedTasks, int index);

int getResponseTimeSPO(SporadicTask** tasks, Resource* resources, int* task_num, int num_partitions, int isMSRP);

long getResponseTimeForSBPO(int partition, SporadicTask **tasks,int num_partition,  int* tasks_num, Resource *resources, long response_time[][MAX_TASKS],
                            bool btbHit, bool useRi, bool isMSRP, SporadicTask calTask, int extendCal) ;

void getResponseTimeForOnePartition(int partition, SporadicTask** tasks, int num_partition, int* tasks_num, Resource* resources,
	long response_time[][MAX_TASKS], long* response_time_temp, bool btbHit, bool useRi, bool isMSRP, int extendCal);

void busyWindow(SporadicTask** tasks, int num_partition, int* tasks_num, Resource* resources, long **response_time, long** response_time_plus, bool btbHit, bool useRi,
	bool isMSRP);


long spinDelay(SporadicTask t, SporadicTask **tasks, int num_partition, int* tasks_num, Resource *resources, long **Ris, long Ri, bool btbHit, bool useRi);

long highPriorityInterference(SporadicTask t, SporadicTask **allTasks, int tasks_size, long Ri);

long localBlocking(SporadicTask t, SporadicTask** tasks, int* tasks_num, int partition, Resource* resources, long** Ris, long Ri, bool btbHit, bool useRi, bool isMSRP);

int getNoRRemote(Resource resource, SporadicTask* tasks, int tasks_size, long* Ris, long Ri, bool btbHit, bool useRi);


long getNoRFromHP(SporadicTask task, Resource resource, SporadicTask* tasks, int tasks_size, long* Ris, bool btbHit, bool useRi, long Ri);

int getIndexRInTask(SporadicTask task, Resource resource);

int getCeilingForProcessor(Resource res, SporadicTask* tasks,int task_size);

Resource* getLocalBlockingResourcesMSRP(SporadicTask task, Resource* resources, Resource* localBlockingResources, SporadicTask* localTasks, int task_size);

int getLocalBlockingNumMSRP(Resource* localBlockingResources);

void initResponseTime(SporadicTask** tasks, int num_partition, int* task_num, long **response_time);

void cloneList(long **oldList, long **newList,int num_partitions,int col_size);
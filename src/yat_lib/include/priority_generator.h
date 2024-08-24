#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "sporadic_task.h"

#define MAX_TASKS 16

// 比较函数，用于任务按照截止期限进行排序
int compareTasks(const void *a, const void *b);

// 分配优先级函数
SporadicTask** assignPrioritiesByDM(SporadicTask** tasksToAssgin, int max_tasks, int num_partitions, int* task_num);

void deadlineMonotonicPriorityAssignment(SporadicTask* taskset, int NoT);

// 生成优先级数组的函数
void generatePriorities(int number, int priorities[MAX_TASKS]);
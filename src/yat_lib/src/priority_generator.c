#include <stdio.h>
#include <stdlib.h>
#include "priority_generator.h"
#include "sporadic_task.h"

// 比较函数，用于任务按照截止期限进行排序
int compareTasks(const void *a, const void *b) {
    SporadicTask *task1 = (SporadicTask *)a;
    SporadicTask *task2 = (SporadicTask *)b;

    // 按照截止期限进行比较
    if (task1->deadline < task2->deadline) return -1;
    if (task1->deadline > task2->deadline) return 1;
    return 0;
}

// 分配优先级函数
SporadicTask** assignPrioritiesByDM(SporadicTask** tasksToAssgin, int max_tasks, int num_partitions, int* task_num) {
    // 如果任务为空，则返回空
    if (tasksToAssgin == NULL) {
			return NULL;
		}

    // 对每个分区进行优先级分配
    for (int i = 0; i < num_partitions; i++) {
        deadlineMonotonicPriorityAssignment(tasksToAssgin[i], task_num[i]);
    }

    return tasksToAssgin;
}

void deadlineMonotonicPriorityAssignment(SporadicTask* taskset, int NoT) {
    int priorities[MAX_TASKS];
    generatePriorities(NoT, priorities);
	/* deadline monotonic assignment */
	// taskset.sort((t1, t2) -> Double.compare(t1.deadline, t2.deadline));
    qsort(taskset, NoT, sizeof(SporadicTask), compareTasks);
	for (int i = 0; i < NoT; i++) {
		taskset[i].priority = priorities[i];
	}
}

// 生成优先级数组的函数
void generatePriorities(int number, int priorities[MAX_TASKS]) {
    for (int i = 0; i < number; i++) {
        priorities[i] = (i + 1) * 2;
    }
}
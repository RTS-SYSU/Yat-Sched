#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "sporadic_task.h"
#include "resource.h"
#include "spo.h"
#include "task_group.h"
#include "core_and_priority.h"

// #include <stdio.h>
// #include <stdlib.h>
// #include <math.h>
// #include <string.h>
// #include "SporadicTask.h"
// #include "resource.h"
// #include "spo.h"
// #include "taskGroup.h"
// #include "coreAndPriority.h"

#define _CRT_SECURE_NO_WARNINGS 1

#define MAX_TASKS 16
#define RESOURCE_SIZE 20
#define CPU_SIZE 10
/**
 * getInfo方法，返回任务的详细信息。
 * @return 任务的详细信息，包括任务ID、周期、截止时间、优先级、分区和利用率。
 */
void sporadic_task_get_allocateInfo(SporadicTask* task) {
    printf("T%d : Period = %ld, DDL = %ld, Priority = %d, Partition = %d, Util: %.7f\n",
        task->id, task->period, task->deadline, task->priority, task->partition, task->util);
    for (int i = 0; i < task->resource_num; i++) {
        printf("rIdx:%d ; acceIn1rel:%d\n", task->resource_required_index[i], task->number_of_access_in_one_release[i]);
    }
    printf("\n");
}

// 全局变量
Resource* global_resources;
int global_resource_size;
TaskGroup* global_target_group;

typedef struct {
    int g_i_index;
    int g_j_index;
    long cf;
} GroupPair;

// 向上取整函数
double myCeil(double num) {
    long inum = (long)num;
    if (num == (double)inum) {
        return inum;
    }
    return inum + 1;
}

// 计算任务列表的总利用率
double util_sum(SporadicTask* list, int size) {
    double util = 0;
    for (int i = 0; i < size; i++) {
        util += list[i].util;
    }
    return util;
}

// 只有getCFOneTaskOneGroup()调用
/**
 * 计算maxPeriod内任务向指定资源发出的请求总数*资源csl
 *
 * @param task 任务对象
 * @param resources 资源数组
 * @param rIndex 资源索引
 * @param maxPeriod 最大周期
 * @return maxPeriod内任务向指定资源发出的请求总数*资源csl
 */
long getCFOneTasksOneR(SporadicTask task, Resource* resources, int rIndex, long maxPeriod) {
    long cf = 0;
    int flag = -1;
    for (int i = 0; i < task.resource_num; i++) {
        if (task.resource_required_index[i] == rIndex) {
            flag = i;
            break;
        }
    }
    if (flag == -1) {
        return cf;
    }
    int N_i_k = task.number_of_access_in_one_release[flag]; // 由 τ_i 在一次release中向 r^k 发出的请求的数量
    // TODO: 这里可能有bug，因为rIndex是资源的ID，不是资源的索引
    // printf("rIndex=%d",rIndex);
    long c_k = resources[rIndex].csl;
    // printf("c_k=%d",c_k);
    // printf("(long)myCeil(maxPeriod / (double)task.period)=%f\n",myCeil(maxPeriod / (double)task.period));
    long total_number_per_max_period = (long)myCeil(maxPeriod / (double)task.period) * N_i_k;
    cf += total_number_per_max_period * c_k;
    // printf("getCFOneTasksOneR cf = %d",cf);
    return cf;
}

// 公式14
/**
 * 计算任务与任务组间的资源争用。
 *
 * @param task 任务
 * @param group 任务组
 * @param group_size 任务组大小
 * @param resources 资源数组
 * @param resource_size 资源数组大小
 * @return 资源争用
 */
long getCFOneTaskOneGroup(SporadicTask task, SporadicTask* group, int group_size, Resource* resources, int resource_size) {
    int rIndex_group2[RESOURCE_SIZE];
    int rIndex_group2_size = 0;

    for (int i = 0; i < group_size; i++) {
        for (int j = 0; j < group[i].resource_num; j++) {
            rIndex_group2[rIndex_group2_size++] = group[i].resource_required_index[j];
        }
    }

    int rIndex_intersection[RESOURCE_SIZE];
    int rIndex_intersection_size = 0;

    // 求资源需求的交集
    for (int i = 0; i < task.resource_num; i++) {
        for (int j = 0; j < rIndex_group2_size; j++) {
            if (task.resource_required_index[i] == rIndex_group2[j]) {
                rIndex_intersection[rIndex_intersection_size++] = task.resource_required_index[i];
                // printf("rIndex_intersection= %d\n",task.resource_required_index[i]);
                break;
            }
        }
    }


    long max_period = task.period;

    long cf_1 = 0, cf_2 = 0, cf_sum = 0;
    // 对于每一个共同访问的资源
    for (int i = 0; i < rIndex_intersection_size; i++) {
        cf_1 += getCFOneTasksOneR(task, resources, rIndex_intersection[i], max_period);
        for (int j = 0; j < group_size; j++) {
            // group2中所有任务在maxPeriod周期内访问该资源的总次数
            cf_2 += getCFOneTasksOneR(group[j], resources, rIndex_intersection[i], max_period);
        }
        cf_sum += cf_1 < cf_2 ? cf_1 : cf_2;
        cf_1 = 0;
        cf_2 = 0;
    }

    return cf_sum;
}

// 公式15
/**
 * 计算两个任务组间的资源争用
 *
 * @param group1 任务组1
 * @param group1_size 组1大小
 * @param group2 任务组2
 * @param group2_size 组2大小
 * @param resources 资源数组
 * @return 两个组间的资源争用总和
 */
long getCFTwoGroups(SporadicTask* group1, int group1_size, SporadicTask* group2, int group2_size, Resource* resources) {
    /* 计算两个group间的contention factor */
    // Step1. 求两个group中任务共同访问的资源;
    long cf_sum = 0;
    for (int i = 0; i < group1_size; i++) {
        cf_sum += getCFOneTaskOneGroup(group1[i], group2, group2_size, resources, global_resource_size);
    }
    for (int j = 0; j < group2_size; j++) {
        cf_sum += getCFOneTaskOneGroup(group2[j], group1, group1_size, resources, global_resource_size);
    }
    // printf("cf_sum=%ld\n", cf_sum);
    return cf_sum;
}

/**
 * 计算任务组内的资源争用。
 *
 * @param group 任务组。
 * @param group_size 任务组大小。
 * @param resources 资源数组。
 * @return 该组的资源争用值。
 */
long getCFOneGroup(SporadicTask* group, int group_size, Resource* resources) {
    long cf_sum = 0;

    for (int i = 0; i < group_size; i++) {
        SporadicTask taskTempGroup[1];
        SporadicTask* tempGroup = malloc((group_size - 1) * sizeof(SporadicTask));

        taskTempGroup[0] = group[i];

        int tempGroup_size = 0;
        for (int j = 0; j < group_size; j++) {
            if (j != i) {
                tempGroup[tempGroup_size++] = group[j];
            }
        }

        cf_sum += getCFTwoGroups(taskTempGroup, 1, tempGroup, tempGroup_size, resources);
        free(tempGroup);
    }
    return cf_sum;
}

int compareBygetCFTwoGroups(const void* a, const void* b) {
    TaskGroup* group1 = (TaskGroup*)a;
    TaskGroup* group2 = (TaskGroup*)b;

    long cf1 = getCFTwoGroups((group1->tasks), group1->group_size, (global_target_group->tasks), global_target_group->group_size, global_resources);
    long cf2 = getCFTwoGroups((group2->tasks), group2->group_size, (global_target_group->tasks), global_target_group->group_size, global_resources);

    return (cf2 > cf1) - (cf2 < cf1);
}

int compareBygetCFOneTaskOneGroup(const void* a, const void* b) {
    SporadicTask* t1 = (SporadicTask*)a;
    SporadicTask* t2 = (SporadicTask*)b;

    long cf1 = getCFOneTaskOneGroup(*t1, global_target_group->tasks, global_target_group->group_size, global_resources, global_resource_size);
    long cf2 = getCFOneTaskOneGroup(*t2, global_target_group->tasks, global_target_group->group_size, global_resources, global_resource_size);

    return (cf2 > cf1) - (cf2 < cf1);
}

// 按cf降序排序
int compareBygetCFOneGroup(const void* a, const void* b) {
    TaskGroup* group1 = (TaskGroup*)a;
    TaskGroup* group2 = (TaskGroup*)b;

    long cf1 = getCFOneGroup(group1->tasks, group1->group_size, global_resources);
    long cf2 = getCFOneGroup(group2->tasks, group2->group_size, global_resources);

    return (cf2 > cf1) - (cf2 < cf1);
}

// 降序排序
int compareGroupPairs(const void* a, const void* b) {
    GroupPair* gp1 = (GroupPair*)a;
    GroupPair* gp2 = (GroupPair*)b;
    return (gp2->cf > gp1->cf) - (gp2->cf < gp1->cf);
}

// 降序排序
int compareTasksByUtil(const void* a, const void* b) {
    SporadicTask* t1 = (SporadicTask*)a;
    SporadicTask* t2 = (SporadicTask*)b;
    return (t2->util > t1->util) - (t2->util < t1->util);
}

// 升序排序
int compareTasksByDeadline(const void* a, const void* b) {
    SporadicTask* t1 = (SporadicTask*)a;
    SporadicTask* t2 = (SporadicTask*)b;
    return (t1->deadline > t2->deadline) - (t1->deadline < t2->deadline);
}

/**
 * 将任务组进行分组。
 *
 * @param task_groups 任务组数组
 * @param num_groups 任务组数量的指针
 * @param resources 资源数组
 * @param maxUtilPerCore 每个处理器核心允许的最大利用率
 * @return 分组后的任务组数组
 */
TaskGroup* Grouping(TaskGroup* task_groups, int* num_groups, Resource* resources, double maxUtilPerCore) {
    int initial_num_groups = *num_groups;
    // 计算group_list中每一对儿group的contention factor并记录
    // GroupPair groupPairs[MAX_TASKS * (MAX_TASKS - 1) / 2];
    GroupPair* groupPairs = (GroupPair*)malloc(sizeof(GroupPair) * initial_num_groups * (initial_num_groups - 1) / 2);
    int pair_count = 0;
    for (int i = 0; i < initial_num_groups; i++) {
        for (int j = i + 1; j < initial_num_groups; j++) {
            long cf = getCFTwoGroups(task_groups[i].tasks, task_groups[i].group_size, task_groups[j].tasks, task_groups[j].group_size, resources);
            groupPairs[pair_count++] = (GroupPair){ i, j, cf };
            // printf("Group %d and Group %d, cf=%ld\n", i, j, cf);
        }
    }
    // 按contention factor降序排序
    qsort(groupPairs, pair_count, sizeof(GroupPair), compareGroupPairs);

    // 尝试融合contention factor最大的一对group
    for (int k = 0; k < pair_count; k++) {
        int i = groupPairs[k].g_i_index;
        int j = groupPairs[k].g_j_index;
        double new_util_sum = util_sum(task_groups[i].tasks, task_groups[i].group_size) + util_sum(task_groups[j].tasks, task_groups[j].group_size);
        // 若融合后利用率不大于maxUtilPerCore,则进行融合
        if (new_util_sum <= maxUtilPerCore) {
            // printf("Group %d and Group %d are merged\n", i, j);
            // cf为0,则不再融合
            if (groupPairs[k].cf == 0) {
                // printf("Group %d and Group %d are merged,cf=0\n", i, j);
                return task_groups;
            }
            memcpy(task_groups[i].tasks + task_groups[i].group_size, task_groups[j].tasks, sizeof(SporadicTask) * task_groups[j].group_size);
            task_groups[i].group_size += task_groups[j].group_size;
            for (int l = j; l < initial_num_groups - 1; l++) {
                task_groups[l] = task_groups[l + 1];
            }
            (*num_groups)--;
            initial_num_groups--;
            // 融合完毕,进行下一组融合(recursion)
            return Grouping(task_groups, num_groups, resources, maxUtilPerCore);
        }
    }
    free(groupPairs);
    // 所有group对均不能融合,算法结束
    return task_groups;
}

/**
 * 它利用率和 CF 递归地将任务分配给可用的核心。
 *
 * @param group_list 一个TaskGroup结构的数组，表示要分配的任务组列表。
 * @param num_groups group_list数组中的任务组数量。
 * @param resources 一个Resource结构的数组，表示可用的资源。
 * @param utilPerPartition 一个双精度数组，表示每个分区的利用率。
 * @param total_partitions 总的分区数（核心数）。
 * @param final_tasks 一个TaskGroup结构的数组，表示最终分配的任务。
 *
 * @return 如果所有任务组都成功分配，返回1；否则返回0。
 *
 * 函数首先检查是否所有的任务组都已经分配。如果是，那么函数返回true。
 * 然后，函数找出利用率最低的处理器核心，并根据该核心上的任务和任务组列表中所有组的contention factor进行排序。
 * 如果利用率最低的核心可以容纳contention factor最大的任务组，那么将该任务组分配到该核心上，并更新利用率。
 * 如果不能容纳整个任务组，那么将任务组内的任务按contention factor从大到小排序，并尝试将每个任务分配到核心上。
 * 如果一个任务都不能分配，那么函数返回false。
 * 最后，函数递归调用自身，继续分配剩余的任务组。
 */
int RAFallocating(TaskGroup* group_list, int num_groups, Resource* resources, double* utilPerPartition, int total_partitions, TaskGroup* final_tasks) {
    // 递归终止条件：当所有group_list都被分配时，返回1
    if (num_groups == 0) {
        return 1;
    }

    // 找到剩余利用率最高的核心
    int min_util_index = -1;
    double min_util_value = 2;
    for (int j = 0; j < total_partitions; j++) {
        if (utilPerPartition[j] < min_util_value) {
            min_util_value = utilPerPartition[j];
            min_util_index = j;
        }
    }

    global_target_group = &final_tasks[min_util_index];

    // 根据核心上的任务和所有组的cf对group_list进行排序（降序）
    qsort(group_list, num_groups, sizeof(TaskGroup), compareBygetCFTwoGroups);

    if (util_sum(group_list[0].tasks, group_list[0].group_size) + utilPerPartition[min_util_index] < 1) {
        // 如果剩余利用率最高的核心可以容纳cf最大的任务组，将该组分配给该核心
        for (int i = 0; i < group_list[0].group_size; i++) {
            group_list[0].tasks[i].partition = min_util_index;
            // 更新利用率数组
            utilPerPartition[min_util_index] += group_list[0].tasks[i].util;
            final_tasks[min_util_index].tasks[final_tasks[min_util_index].group_size] = group_list[0].tasks[i];
            final_tasks[min_util_index].group_size++;
        }
        for (int i = 0; i < num_groups - 1; i++) {
            group_list[i] = group_list[i + 1];
        }
        num_groups--;
    }
    else { // 如果整个组无法容纳
        SporadicTask* group = group_list[0].tasks;
        // 对组内任务按cf从大到小排序
        qsort(group, group_list[0].group_size, sizeof(SporadicTask), compareBygetCFOneTaskOneGroup);
        int isAllocatedFlag = 0;
        for (int k = 0; k < group_list[0].group_size; k++) {
            if (group[k].util + utilPerPartition[min_util_index] < 1) {
                isAllocatedFlag = 1;
                // 将一个任务分配给核心
                group[k].partition = min_util_index;
                utilPerPartition[min_util_index] += group[k].util;
                final_tasks[min_util_index].tasks[final_tasks[min_util_index].group_size] = group[k];
                final_tasks[min_util_index].group_size++;
                // 从组中移除任务
                for (int i = k; i < group_list[0].group_size - 1; i++) {
                    group_list[0].tasks[i] = group_list[0].tasks[i + 1];
                }
                group_list[0].group_size--;
                k--;
                // 降序排序
                qsort(group, group_list[0].group_size, sizeof(SporadicTask), compareBygetCFOneTaskOneGroup);
            }
            else {
                break;
            }
        }
        // 如果没有任务被分配，返回false
        if (!isAllocatedFlag) {
            return 0;
        }
    }
    return RAFallocating(group_list, num_groups, resources, utilPerPartition, total_partitions, final_tasks);
}

/**
 * 根据资源访问情况进行任务分组和分配,将任务分配给不同的分区。
 *
 * RAF这个方法将任务分配给不同的分区。
 * 该方法接收一个需要分配的任务列表，一个资源列表，总的分区数，以及每个核心的最大利用率作为输入。
 * 该方法返回一个SporadicTask对象的列表的列表，其中每个列表代表一个分区，并包含分配给该分区的任务。
 *
 * @param tasksToAllocate 要分配的任务数组
 * @param numTasks 任务数组的大小
 * @param resources 资源数组
 * @param total_partitions 总分区数
 * @param maxUtilPerCore 每个处理器核心允许的最大利用率
 * @return 分配后的TaskGroup数组，每个TaskGroup代表一个分区，如果分配失败则返回NULL
 */
TaskGroup* RAF(SporadicTask* tasksToAllocate, int numTasks, Resource* resources, int total_partitions, double maxUtilPerCore) {

    // TaskGroup final_tasks[CPU_SIZE];
    // （初始化）建立最终返回分组
    // final_tasks的组数为total_partitions
    TaskGroup* final_tasks = malloc(total_partitions * sizeof(TaskGroup));
    for (int i = 0; i < total_partitions; i++) {
        final_tasks[i].group_size = 0;
        final_tasks[i].tasks = malloc(numTasks * sizeof(SporadicTask));
    }

    // 升序排序
    qsort(tasksToAllocate, numTasks, sizeof(SporadicTask), compareTasksByDeadline);

    // linux priority值越小优先级越高
    for (int i = 0; i < numTasks; i++) {
        tasksToAllocate[i].priority = i + 1;
    }

    // 将访问资源的任务和不访问资源的任务分成两个list
    SporadicTask tasksWithResource[MAX_TASKS];
    SporadicTask tasksNoResource[MAX_TASKS];
    int numTasksWithResource = 0, numTasksNoResource = 0;
    for (int i = 0; i < numTasks; i++) {
        if (tasksToAllocate[i].resource_num > 0) {
            tasksWithResource[numTasksWithResource++] = tasksToAllocate[i];
        }
        else {
            tasksNoResource[numTasksNoResource++] = tasksToAllocate[i];
        }
    }

    /* Step1.对于访问资源的任务进行分组 */
    // 初始化每个task一个group
    TaskGroup* task_group_list = malloc(numTasksWithResource * sizeof(TaskGroup));
    for (int i = 0; i < numTasksWithResource; i++) {
        task_group_list[i].tasks = malloc(numTasks * sizeof(SporadicTask));
        task_group_list[i].tasks[0] = tasksWithResource[i];
        task_group_list[i].group_size = 1;
    }
    int group_list_size = numTasksWithResource;
    // 递归进行分组
    TaskGroup* group_list = Grouping(task_group_list, &group_list_size, resources, maxUtilPerCore);
    // CF 大到小降序排序
    qsort(group_list, group_list_size, sizeof(TaskGroup), compareBygetCFOneGroup);

    /* Step2.将所有任务合理地放置在processor上 */
    // 每个核心的利用率初始化为0
    double utilPerPartition[CPU_SIZE] = { 0 };

    /* Step2-1.先将分组(访问资源的任务)放在核心上 */
    // 分组数小于等于核心数
    if (group_list_size <= total_partitions) {
        for (int i = 0; i < group_list_size; i++) {
            // 第i个group放在第i个核心上,并更新每个核心的利用率
            for (int j = 0; j < group_list[i].group_size; j++) {
                final_tasks[i].tasks[j] = group_list[i].tasks[j];
                final_tasks[i].tasks[j].partition = i;
                utilPerPartition[i] += final_tasks[i].tasks[j].util;
            }
            final_tasks[i].group_size = group_list[i].group_size;
        }
    }
    else { // 分组数大于核心数
        for (int i = 0; i < total_partitions; i++) {
            // 第i个group放在第i个核心上（前total_partition个组）
            for (int j = 0; j < group_list[i].group_size; j++) {
                final_tasks[i].tasks[j] = group_list[i].tasks[j];
                final_tasks[i].tasks[j].partition = i;
                utilPerPartition[i] += final_tasks[i].tasks[j].util;
            }
            final_tasks[i].group_size = group_list[i].group_size;
        }
        // 分好的全移除
        TaskGroup* remaining_groups = malloc((group_list_size - total_partitions) * sizeof(TaskGroup));
        for (int i = 0; i < group_list_size - total_partitions; i++) {
            remaining_groups[i] = group_list[total_partitions + i];
        }
        // 递归分配剩余的组
        if (!RAFallocating(remaining_groups, group_list_size - total_partitions, resources, utilPerPartition, total_partitions, final_tasks)) {
            free(remaining_groups);
            return NULL;
        }
        free(remaining_groups);
    }

    /* Step2-2.采用Worst-Fit-Decreasing将不访问资源的任务放在核心上 */
    // 按利用率从大到小排序（降序）
    qsort(tasksNoResource, numTasksNoResource, sizeof(SporadicTask), compareTasksByUtil);
    for (int i = 0; i < numTasksNoResource; i++) {
        int target = -1;
        double minUtil = 2;
        // 找到利用率最小的核心作为本次分配的目标
        for (int j = 0; j < total_partitions; j++) {
            if (utilPerPartition[j] < minUtil) {
                minUtil = utilPerPartition[j];
                target = j;
            }
        }

        if (target == -1) {
            fprintf(stderr, "RAF error!\n");
            return NULL;
        }
        /* 当前任务的利用率（task.util）小于或等于
           目标处理器的剩余利用率（1 - minUtil）则分配
        */
        if (1 - minUtil >= tasksNoResource[i].util) {
            tasksNoResource[i].partition = target;
            final_tasks[target].tasks[final_tasks[target].group_size] = tasksNoResource[i];
            final_tasks[target].group_size++;
            utilPerPartition[target] += tasksNoResource[i].util;
        }
        else {
            fprintf(stderr, "RAF error!\n");
            return NULL;
        }
    }

    return final_tasks;
}

//   #pragma warning(disable : 4996)
void parseTaskLine(char* line, SporadicTask* task) {
    task->id = atoi(strtok(line, ","));
    task->WCET = atoi(strtok(NULL, ","));
    // task->pure_resource_execution_time= atoi(strtok(NULL, ","));
    // task->deadline = atoi(strtok(NULL, ","));
    task->period = atoi(strtok(NULL, ","));
    task->deadline = task->period;
    // task->Ri = task->deadline;
    // task->partition = atoi(strtok(NULL, ","));
    // task->priority = atoi(strtok(NULL, ","));
    task->util = atof(strtok(NULL, ","));
    char* resources = strtok(NULL, ",");
    task->resource_num = atoi(strtok(NULL, ","));
    char* accesses = strtok(NULL, ",");

    // 打印period
    // printf("period=%d\n", task->period);

    task->addition_slack_BTB = 1e9; // 初始化

    task->pure_resource_execution_time = 0;

    // Parse resource_required_index
    task->resource_required_index = malloc(task->resource_num * sizeof(int));
    char* token = strtok(resources, ";");
    for (int i = 0; i < task->resource_num && token != NULL; i++) {
        task->resource_required_index[i] = atoi(token);
        token = strtok(NULL, ";");
    }

    // Parse number_of_access_in_one_release
    task->number_of_access_in_one_release = malloc(task->resource_num * sizeof(int));
    token = strtok(accesses, ";");
    for (int i = 0; i < task->resource_num && token != NULL; i++) {
        task->number_of_access_in_one_release[i] = atoi(token);
        token = strtok(NULL, ";");
    }

}

void parseResourceLine(char* line, Resource* resource) {
    resource->id = atoi(strtok(line, ","));
    resource->csl = atoi(strtok(NULL, ","));
    // printf("resource id=%d,csl=%d",resource->id,resource->csl);
}

TaskGroup* set_up(const char* const_str, int total_partitions, int* num_threads)
{
    FILE* file = fopen(const_str, "r");
    if (!file) {
        printf("Error opening file.\n");
        return 1;
    }

    char line[1024];
    SporadicTask* tasksToAllocate;

    // Read task count
    fgets(line, sizeof(line), file);
    int numTasks = atoi(line);
    *num_threads = numTasks;
    tasksToAllocate = malloc(numTasks * sizeof(SporadicTask));

    // Read tasks
    fgets(line, sizeof(line), file); // Skip header
    for (int i = 0; i < numTasks; i++) {
        fgets(line, sizeof(line), file);
        parseTaskLine(line, &tasksToAllocate[i]);
    }

    // Read resource count
    fgets(line, sizeof(line), file);
    global_resource_size = atoi(line);
    global_resources = malloc(global_resource_size * sizeof(Resource));

    // Read resources
    fgets(line, sizeof(line), file); // Skip header
    for (int i = 0; i < global_resource_size; i++) {
        fgets(line, sizeof(line), file);
        parseResourceLine(line, &global_resources[i]);
    }

    fclose(file);

    for (int i = 0; i < numTasks; i++)
    {
        tasksToAllocate[i].partition = -1; // 分区
        tasksToAllocate[i].priority = -1;  // 优先级

        // 计算pure_resource_execution_time
        for (int j = 0; j < tasksToAllocate[i].resource_num; j++)
        {
            tasksToAllocate[i].pure_resource_execution_time +=
                tasksToAllocate[i].number_of_access_in_one_release[j] * global_resources[tasksToAllocate[i].resource_required_index[j]].csl;
        }
        tasksToAllocate[i].Ri = tasksToAllocate[i].WCET + tasksToAllocate[i].pure_resource_execution_time;
    }


    // double totalUtil = 0.1 * numTasks;
    // double maxUtilPerCore = totalUtil / total_partitions;
    // if (maxUtilPerCore <= 0.5)
    //     maxUtilPerCore = 0.5;
    // else if (maxUtilPerCore <= 0.6)
    //     maxUtilPerCore = 0.6;
    // else if (maxUtilPerCore <= 0.65)
    //     maxUtilPerCore = 0.65;
    // else
    //     maxUtilPerCore = 1;
    double maxUtilPerCore = 0.76;

    // 将tasksToAllocate、global_resources等参数传给内核，并得到内核返回的二维数组计算结果
    TaskGroup* allocated_tasks = RAF(tasksToAllocate, numTasks, global_resources, total_partitions, maxUtilPerCore);

    SporadicTask** Tasks = malloc(MAX_TASKS * sizeof(SporadicTask*));
    Resource* resources = malloc(RESOURCE_SIZE * sizeof(Resource));

    if (allocated_tasks != NULL) {
        // for (int i = 0; i < total_partitions; i++) {
        // 	if (allocated_tasks[i].group_size == 0) {
        // 		tasks.remove(i);
        // 		i--;
        // 	}
        // }

        /* for each resource */
        if (global_resources != NULL && global_resource_size > 0) {
            for (int i = 0; i < global_resource_size; i++) {
                Resource* resource = &global_resources[i];
                resource->requested_tasks_num = 0;
                resource->partition_num = 0;
                resource->requested_tasks = malloc(sizeof(SporadicTask) * numTasks);
                resource->partitions = malloc(sizeof(int) * total_partitions);
                resource->isGlobal = 0;
                // 假设已经有足够的空间或进行了动态分配

                /* for each partition */
                for (int j = 0; j < total_partitions; j++) {
                    /* for each task in the given partition */
                    for (int k = 0; k < allocated_tasks[j].group_size; k++) {
                        SporadicTask* task = &allocated_tasks[j].tasks[k];
                        // 检查task是否需要当前资源
                        int containsResource = 0;
                        for (int l = 0; l < task->resource_num; l++) {
                            if (task->resource_required_index[l] == resource->id - 1) {
                                containsResource = 1;
                                break;
                            }
                        }

                        if (containsResource) {
                            // 添加task到resource的requested_tasks
                            resource->requested_tasks[resource->requested_tasks_num++] = *task;
                            // 检查resource的partitions是否已包含task的partition
                            int containsPartition = 0;
                            for (int m = 0; m < resource->partition_num; m++) {
                                if (resource->partitions[m] == task->partition) {
                                    containsPartition = 1;
                                    break;
                                }
                            }

                            if (!containsPartition) {
                                // 添加task的partition到resource的partitions
                                resource->partitions[resource->partition_num++] = task->partition;
                            }
                        }
                    }
                }

                if (resource->partition_num > 1)
                    resource->isGlobal = 1; // true
            }
        }

        // for (int i = 0; i < total_partitions; i++) {
        //     printf("Allocated Group%d:\n", i);
        //     for (int j = 0; j < allocated_tasks[i].group_size; j++) {
        //         sporadic_task_get_allocateInfo(&allocated_tasks[i].tasks[j]);
        //     }
        // }

        /*
        * 开始进行SPO算法
        */

        // 初始化task_num数组
        int task_num[MAX_TASKS];
        for (int i = 0; i < MAX_TASKS; i++) {
            task_num[i] = 0;
        }
        for (int i = 0; i < total_partitions; i++) {
            task_num[i] = allocated_tasks[i].group_size;
        }

        /*for (int i = 0; i < MAX_TASKS; i++) {
            printf("%d ", task_num[i]);
        }*/

        // 初始化Tasks数组
        for (int i = 0; i < MAX_TASKS; i++) {
            Tasks[i] = malloc(MAX_TASKS * sizeof(SporadicTask));
        }
        for (int i = 0; i < MAX_TASKS; i++) {
            for (int j = 0; j < MAX_TASKS; j++) {
                Tasks[i][j].id = -1;
                // Tasks[i][j].addition_slack_BTB = 1e9;
            }
        }
        for (int i = 0; i < total_partitions; i++) {
            for (int j = 0; j < task_num[i]; j++) {
                Tasks[i][j] = allocated_tasks[i].tasks[j];
                // printf("%d %d %ld\n", i, j, Tasks[i][j].addition_slack_BTB);
            }
        }
        // 初始化Resource数组
        for (int i = 0; i < RESOURCE_SIZE; i++) {
            resources[i].id = -1;
        }
        for (int i = 0; i < global_resource_size; i++) {
            resources[i] = global_resources[i];
        }
        getResponseTimeSPO(Tasks, resources, task_num, total_partitions, 1);
        // // 反转优先级
        // for (int i = 0; i < total_partitions; i++) {
        //     for (int j = 0; j < task_num[i]; j++) {
        //         Tasks[i][j].priority = 99 - Tasks[i][j].priority;
        //     }
        // }
        // 返回给allocated_tasks
        for (int i = 0; i < total_partitions; i++) {
            for (int j = 0; j < task_num[i]; j++) {
                allocated_tasks[i].tasks[j] = Tasks[i][j];
            }
        }
        // // 输出:
        // printf("result of OUR\n");
        // for (int i = 0; i < total_partitions; i++) {
        //     printf("partitons: %d\n", i);
        //     for (int j = 0; j < task_num[i]; j++) {
        //         printf("task_id:%d priority:%d\n",allocated_tasks[i].tasks[j].id, allocated_tasks[i].tasks[j].priority);
        //     }
        // }
        // // 输出:
        // printf("result of OUR\n");
        // for (int i = 0; i < total_partitions; i++) {
        //     printf("partitons: %d\n", i);
        //     for (int j = 0; j < task_num[i]; j++) {
        //         printf("task_id:%d priority:%d\n",allocated_tasks[i].tasks[j].id, allocated_tasks[i].tasks[j].priority);
        //     }
        // }
    }
    // 释放内存
    for (int i = 0; i < MAX_TASKS; i++) {
        free(Tasks[i]);
    }
    free(Tasks);
    free(resources);

    // for (int i = 0; i < numTasks; i++) {
    //     free(tasksToAllocate[i].resource_required_index);
    //     free(tasksToAllocate[i].number_of_access_in_one_release);
    // }
    // free(tasksToAllocate);
    free(global_resources);

    return allocated_tasks;
}

TaskGroup* comparativeExperiment(const char* const_str, int total_partitions, int* num_threads)
{
    FILE* file = fopen(const_str, "r");
    if (!file) {
        printf("Error opening file.\n");
        return 1;
    }

    char line[1024];
    SporadicTask* tasksToAllocate;

    // Read task count
    fgets(line, sizeof(line), file);
    int numTasks = atoi(line);
    *num_threads = numTasks;
    tasksToAllocate = malloc(numTasks * sizeof(SporadicTask));

    // Read tasks
    fgets(line, sizeof(line), file); // Skip header
    for (int i = 0; i < numTasks; i++) {
        fgets(line, sizeof(line), file);
        parseTaskLine(line, &tasksToAllocate[i]);
    }

    fclose(file);

    double maxUtilPerCore = 1;

    // （初始化）建立最终返回分组
    // final_tasks的组数为total_partitions
    TaskGroup* final_tasks = malloc(total_partitions * sizeof(TaskGroup));
    for (int i = 0; i < total_partitions; i++) {
        final_tasks[i].group_size = 0;
        final_tasks[i].tasks = malloc(numTasks * sizeof(SporadicTask));
    }

    // 升序排序
    qsort(tasksToAllocate, numTasks, sizeof(SporadicTask), compareTasksByDeadline);

    // linux priority值越小优先级越高（rtspin也是：highest=1, lowest=511）。实时任务值越大优先级越高。
    // assign priorities by Deadline Monotonic
    for (int i = 0; i < numTasks; i++) {
        tasksToAllocate[i].priority = (i + 1);
    }

    // 每个核心的利用率初始化为0
    double utilPerPartition[CPU_SIZE] = { 0 };

    /* 采用Worst-Fit-Decreasing将任务分配给核心 */
    // 按利用率从大到小排序（降序）
    qsort(tasksToAllocate, numTasks, sizeof(SporadicTask), compareTasksByUtil);
    for (int i = 0; i < numTasks; i++) {
        int target = -1;
        double minUtil = 2;
        // 找到利用率最小的核心作为本次分配的目标
        for (int j = 0; j < total_partitions; j++) {
            if (utilPerPartition[j] < minUtil) {
                minUtil = utilPerPartition[j];
                target = j;
            }
        }

        if (target == -1) {
            fprintf(stderr, "comparativeExperiments error!\n");
            return NULL;
        }
        /* 当前任务的利用率（task.util）小于或等于
           目标处理器的剩余利用率（1 - minUtil）则分配
        */
        if (1 - minUtil >= tasksToAllocate[i].util) {
            tasksToAllocate[i].partition = target;
            final_tasks[target].tasks[final_tasks[target].group_size] = tasksToAllocate[i];
            final_tasks[target].group_size++;
            utilPerPartition[target] += tasksToAllocate[i].util;
        }
        else {
            fprintf(stderr, "RAF error!\n");
            return NULL;
        }
    }
    return final_tasks;
}

// int main()
// {
//     int total_partitions = 4;
//     int num_threads;
//     TaskGroup* allocated_tasks = set_up("C:/Users/liuy/Desktop/OSgame/dataV6.csv", total_partitions, &num_threads);
//     // TaskGroup* allocated_tasks = comparativeExperiment("C:/Users/liuy/Desktop/OSgame/dataV5.csv", total_partitions, &num_threads);
//     // 输出:
//     printf("num_threads:%d\n", num_threads);
//     // for (int i = 0; i < total_partitions; i++)
//     // {
//     //     printf("Allocated Group%d:\n", i);
//     //     for (int j = 0; j < allocated_tasks[i].group_size; j++)
//     //     {
//     //         sporadic_task_get_allocateInfo(&allocated_tasks[i].tasks[j]);
//     //     }
//     // }

//     printf("result of OUR\n");
//     // printf("result of comparativeExperiment\n");
//     for (int i = 0; i < total_partitions; i++)
//     {
//         printf("partitons: %d\n", i);
//         for (int j = 0; j < allocated_tasks[i].group_size; j++)
//         {
//             printf("task_id:%d priority:%d\n", allocated_tasks[i].tasks[j].id, allocated_tasks[i].tasks[j].priority);
//         }
//     }

//     return 0;
// }





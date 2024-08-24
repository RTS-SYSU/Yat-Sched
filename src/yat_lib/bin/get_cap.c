#include <stdio.h>
#include <stdlib.h>
#include "core_and_priority.h"

// 定义 task 结构体，包含任务、分区和优先级信息
typedef struct {
    int task_id;
    int partition;
    int priority;
    int execute_cost;
    int period;
    int resource_num;
    int* resource_required_index;
    int* number_of_access_in_one_release;
    int* resource_csl;
} task;

// 定义比较函数，用于按照 task_id 进行排序
int compare_tasks(const void *a, const void *b) {
    task *taskA = (task *)a;
    task *taskB = (task *)b;
    return (taskA->task_id - taskB->task_id);  // 按 task_id 升序排序
}

int main() {
    int total_partitions = 4;
    int num_threads;
    TaskGroup* allocated_tasks = set_up("/root/yat_lib/data.csv", total_partitions, &num_threads);
    // TaskGroup* allocated_tasks = comparativeExperiment("/root/yat_lib/data.csv", total_partitions, &num_threads);

    // 输出线程数量
    printf("num_threads: %d\n", num_threads);

    // 统计所有任务的总数
    int total_tasks = 0;
    for (int i = 0; i < total_partitions; i++) {
        total_tasks += allocated_tasks[i].group_size;
    }

    // 创建一个数组来存储所有任务及其分区信息
    task *all_tasks = malloc(total_tasks * sizeof(task));

    // all_tasks->resource_required_index = malloc(all_tasks->resource_num * sizeof(int));
    // all_tasks->number_of_access_in_one_release = malloc(all_tasks->resource_num * sizeof(int));
    // 初始化 all_tasks 中每个任务的资源指针，并为其分配内存
    for (int i = 0; i < total_tasks; i++) {
        all_tasks[i].resource_required_index = NULL;
        all_tasks[i].number_of_access_in_one_release = NULL;
        all_tasks[i].resource_csl = NULL;
    }

    int index = 0;

    // 将所有任务及其分区信息存储到 all_tasks 数组中
    for (int i = 0; i < total_partitions; i++) {
        for (int j = 0; j < allocated_tasks[i].group_size; j++) {
            all_tasks[index].task_id = allocated_tasks[i].tasks[j].id;
            all_tasks[index].partition = i;
            all_tasks[index].priority = allocated_tasks[i].tasks[j].priority;
            all_tasks[index].execute_cost = allocated_tasks[i].tasks[j].WCET;
            all_tasks[index].period = allocated_tasks[i].tasks[j].period;
            all_tasks[index].resource_num = allocated_tasks[i].tasks[j].resource_num;

            // 为资源数组分配内存
            if (all_tasks[index].resource_num > 0) {
                all_tasks[index].resource_required_index = malloc(all_tasks[index].resource_num * sizeof(int));
                all_tasks[index].number_of_access_in_one_release = malloc(all_tasks[index].resource_num * sizeof(int));
                all_tasks[index].resource_csl = malloc(all_tasks[index].resource_num * sizeof(int));

                for (int k = 0; k < all_tasks[index].resource_num; k++) {
                    all_tasks[index].resource_required_index[k] = allocated_tasks[i].tasks[j].resource_required_index[k];
                    all_tasks[index].number_of_access_in_one_release[k] = allocated_tasks[i].tasks[j].number_of_access_in_one_release[k];
                    all_tasks[index].resource_csl[k] = allocated_tasks[i].tasks[j].pure_resource_execution_time;
                }
            }

            index++;
        }
    }

    // 对所有任务按照 task_id 进行全局排序
    qsort(all_tasks, total_tasks, sizeof(task), compare_tasks);

    // 输出排序后的结果
    printf("result of OUR\n");
    for (int i = 0; i < total_tasks; i++) {
        printf("task_id:%d partition:%d priority:%d execute_cost:%d period:%d resource_num:%d ",
                all_tasks[i].task_id,
                all_tasks[i].partition,
                all_tasks[i].priority,
                all_tasks[i].execute_cost,
                all_tasks[i].period,
                all_tasks[i].resource_num
                );

        printf("resource_combined:");
        for (int j = 0; j < all_tasks[i].resource_num; j++) {
            printf("%d %d %d ", all_tasks[i].resource_required_index[j], all_tasks[i].number_of_access_in_one_release[j], all_tasks[i].resource_csl[j]);
        }

        printf("csl:");
        for (int j = 0; j < all_tasks[i].resource_num; j++) {
            printf("%d ", all_tasks[i].resource_csl[j]);
        }

        // resource_csl[ all_tasks[i].resource_required_index[j] + 1 ]

        // printf("resource_required_index:");
        // for (int j = 0; j < all_tasks[i].resource_num; j++) {
        //     printf("%d ",all_tasks[i].resource_required_index[j]);
        // }

        // printf("number_of_access_in_one_release:");
        // for (int j = 0; j < all_tasks[i].resource_num; j++) {
        //     printf("%d ",all_tasks[i].number_of_access_in_one_release[j]);
        // }

        printf("\n");
    }

    // 释放动态分配的内存
    for (int i = 0; i < total_tasks; i++) {
        free(all_tasks[i].resource_required_index);
        free(all_tasks[i].number_of_access_in_one_release);
        free(all_tasks[i].resource_csl);
    }
    free(all_tasks);

    return 0;
}

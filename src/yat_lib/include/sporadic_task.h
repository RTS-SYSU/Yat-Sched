#pragma once

#include <stdio.h>

typedef struct {
    int priority;
    long period;
    long deadline;
    long WCET; // 任务最坏情况执行时间（Worst-Case Execution Time）
    int partition;
    int id;
    double util;
    long pure_resource_execution_time;
    long Ri, spin, interference, local, total_blocking, indirectspin;
    long addition_slack_BTB;
    int* resource_required_index;
    int resource_num;
    int* number_of_access_in_one_release;
    long spin_delay_by_preemptions;
    double implementation_overheads, blocking_overheads;
    double mrsp_arrivalblocking_overheads, fifonp_arrivalblocking_overheads, fifop_arrivalblocking_overheads;
    double migration_overheads_plus;
    long addition_slack_by_newOPA;
    int hasResource;
    int* resource_required_index_copy;
    int* number_of_access_in_one_release_copy;
} SporadicTask;

SporadicTask init_task();
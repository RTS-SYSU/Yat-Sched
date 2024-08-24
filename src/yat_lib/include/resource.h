#pragma once

#include <stdio.h>
#include "sporadic_task.h"

typedef struct {
    int id;
    long csl; // 资源临界区长度
    SporadicTask* requested_tasks;
    int requested_tasks_num;
    int* partitions; // 请求该资源的分区列表
    int partition_num;
    int* ceiling;
    int isGlobal;
} Resource;

Resource init_resource();
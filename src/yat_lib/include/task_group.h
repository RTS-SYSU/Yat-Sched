#pragma once

#include <stdio.h>
#include "sporadic_task.h"

typedef struct {
    SporadicTask* tasks; // 任务列表，一维任务数组
    int group_size;
} TaskGroup;


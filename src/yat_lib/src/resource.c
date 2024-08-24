#include "resource.h"

Resource init_resource(){
    Resource res;
    res.id=-1;
    res.csl=0;
    res.partition_num=0;
    res.requested_tasks_num =0;
    res.isGlobal=0;

    return res;
}
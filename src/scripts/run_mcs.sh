#!/bin/sh

# 运行 C 程序并捕获输出
output=$(get_cap)

# 解析输出并提取变量
echo "$output" | while read -r line; do
    if echo "$line" | grep -q "num_threads:"; then
        num_threads=$(echo $line | awk -F ' ' '{print $2}')
        echo "num_threads: $num_threads"
    elif echo "$line" | grep -q "task_id:"; then
        task_id=$(echo $line | awk -F 'task_id:| partition:' '{print $2}')
        partition=$(echo $line | awk -F 'partition:| priority:' '{print $2}')
        priority=$(echo $line | awk -F 'priority:| execute_cost:' '{print $2}')
        execute_cost=$(echo $line | awk -F 'execute_cost:| period:' '{print $2}')
        period=$(echo $line | awk -F 'period:| resource_num:' '{print $2}')
        resource_num=$(echo $line | awk -F 'resource_num:| resource_combined:' '{print $2}')
        resource_combined=$(echo $line | awk -F 'resource_combined:' '{print $2}')
        # resource_required_index=$(echo $line | awk -F 'resource_required_index:| number_of_access_in_one_release:' '{print $2}')
        # number_of_access_in_one_release=$(echo $line | awk -F 'number_of_access_in_one_release:' '{print $2}')

        # 判断 resource_num 是否为 0
        if [ "$resource_num" -ne 0 ]; then
            # echo "run_mcs -w -v -X MSRP -Q $resource_num -p $partition -q $priority -O $execute_cost $period 1 $resource_combined"
            run_mcs -w -v -X MSRP -Q "$resource_num" -p "$partition" -q "$priority" -O "$execute_cost" "$period" 1 "$resource_combined" &
        else
            # echo "run_mcs -w -v -X MSRP -p $partition -q $priority -O $execute_cost $period 1"
            run_mcs -w -v -X MSRP -p "$partition" -q "$priority" -O "$execute_cost" "$period" 1 &
        fi
    fi
done

sleep 1

release_ts && wait

tmux swap-pane -U

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

        # 使用 awk 进行浮点数除法运算
        divided_execute_cost=$(echo "$execute_cost" | awk '{printf "%.3f", $1/1000}')
        divided_period=$(echo "$period" | awk '{printf "%.3f", $1/1000}')

        # resource_num=$(echo $line | awk -F 'resource_num:| resource_combined:' '{print $2}')
        # resource_combined=$(echo $line | awk -F 'resource_combined:' '{print $2}')
        # resource_required_index=$(echo $line | awk -F 'resource_required_index:| number_of_access_in_one_release:' '{print $2}')
        # number_of_access_in_one_release=$(echo $line | awk -F 'number_of_access_in_one_release:' '{print $2}')

        # 判断 resource_num 是否为 0
        if [ "$task_id" -le 4 ]; then
            # echo "run_taclebench -w -v -t $task_id -L 0.15 -X MSRP -p $partition -q $priority -O $divided_execute_cost $divided_period 1 &"
            run_taclebench -w -v -t "$task_id" -L 0.15 -X MSRP -p "$partition" -q "$priority" -O "$divided_execute_cost" "$divided_period" 1 &
        else
            # echo "run_taclebench -w -v -t $task_id -p $partition -q $priority -O $divided_execute_cost $divided_period 1 &"
            run_taclebench -w -v -t "$task_id" -p "$partition" -q "$priority" -O "$divided_execute_cost" "$divided_period" 1 &
        fi
    fi
done

sleep 1

release_ts && wait

tmux swap-pane -U

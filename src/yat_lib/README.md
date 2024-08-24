# yat_lib

## 可执行功能

- `availsched`: 查看可用的调度器
- `showsched`: 查看当前使用的调度器
- `setsched`: 切换调度器，后面需跟 availsched 中列出的可选调度器
- `get_cap`: 获取调度算法的输出，是一个用于过渡的接口
- `my_mt_task`: 用于执行单个自定义的任务
- `release_at`: 用于同步释放所有任务
- `run_mcs`: 执行多临界区的测试任务
- `run_taclebench`: 执行多个 taclebench 的测试任务

## 文件目录结构

- `/arch`: 存放不同体系结构下cycles的获取方式
- `/bin`: 存放用于编译生成可执行二进制文件的代码
- `/include`: 存放src目录中的代码需要用到的头文件
- `/src`: 存放用于调度算法实现的源代码、调度器接口调用代码、我们所选用的taclebench的7个benchmark测例
- `/tests`: 存放用于调度器功能测试的代码
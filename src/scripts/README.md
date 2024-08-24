# 测试所用到的脚本

## debian本地使用

- `start_qemu.sh`: 启动qemu虚拟机
- `genpdf.sh`: 提取qemu内部的.bin文件，在本地生成pdf
- `refresh.sh`: 刷新根文件系统，即重新挂载镜像到根文件系统fs，或许能够解决根文件系统和正在运行中的镜像文件不同步的问题

## qemu内部使用

- `init.sh`: 用于qemu内部执行，启动qemu后首先执行该脚本，设置调度器并开启tmux
- `start_trace.sh`: 启动调度跟踪，抓取数据，然后切换到另一个tmux窗口 执行任务

### 执行任务

- `run_mcs.sh`: 根据data.csv文件运行 **多临界区测试** 的脚本
- `run_taclebench.sh`: 根据data.csv文件运行 **taclebench 测试** 的脚本
- `taclebench.sh`: 用于临时测试，可自由更改参数
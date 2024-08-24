# wcet_test

## 说明

`src` 目录中存放的是从 [tacle-bench](https://github.com/tacle/tacle-bench) 中获取的测试用例，其目录组织如下

```text
src
├── adpcm_dec
├── adpcm_enc
...
```
每个文件夹都是一个测试用例，其中包含了 `c` 文件和一些 `h` 文件，其中所有的入口函数都是 `main` 函数，未经过修改，可以直接编译运行

### 脚本说明

#### compile.py

用于编译测试用例，并根据需要编译生成对应的动态链接库或可执行文件，其可能的参数如下

```text
usage: llvmta benchmark suite compiler [-h] [-w] [-m MAIN] [-l] [-f FLAGS] [-lf LDFLAGS] [-c COMPILER] [-s SRC] [-o OUTPUT]

options:
  -h, --help            show this help message and exit
  -w, --wrap            Wrap the main function
  -m MAIN, --main MAIN  Where the real main function is located
  -l, --lib             Compile as a library
  -f FLAGS, --flags FLAGS
                        Compiler flags to pass to the compiler, default is "-O0 -fPIC" if -l is set, otherwise "-O0"
  -lf LDFLAGS, --ldflags LDFLAGS
                        Compiler flags to pass to the linker, default is "-shared" if -l is set, otherwise ""
  -c COMPILER, --compiler COMPILER
                        Compiler to use, default is "gcc"
  -s SRC, --src SRC     Directory to compile, default is "src"
  -o OUTPUT, --output OUTPUT
                        Output directory, default is "bin" if -l is not set, otherwise "lib"
```

例如，在使用多核心测试框架[Execution_Counter](https://github.com/RTS-SYSU/Execution_Counter)的时候，我们需要将其编译成动态链接库，并将其中的 `main` 函数包装成 `<name>_start` 函数，这时候我们可以使用如下命令

```bash
./compile.py -l -s <dir_to_src> -o <dir_to_lib>
```

而如果我们需要将其编译成可执行文件，但是我们希望通过一个性能计数器来统计执行时间，即我们需要将 `main` 函数替换为 `__wrap_main`，这时候我们可以使用如下命令

```bash
./compile.py -w -m <real_main_file> -s <dir_to_src> -o <dir_to_bin>
```

例如在该目录下的 `main.c` 中，我们有如下代码

```c
#include <stdio.h>

int __wrap_main();

int main() {
  printf("Before wrap main\n");
  __wrap_main();
  printf("After wrap main\n");
  return 0;
}
```

脚本中的 `-m` 参数就是用于指定 `__wrap_main` 函数的实现文件，这里就是 `main.c`



#include <stdint.h>
#include <stdio.h>
#include <time.h>

typedef struct timespec timespec_t;

/* 获取当前时间 */
static inline timespec_t get_time() {
    timespec_t time;
    clock_gettime(CLOCK_MONOTONIC, &time); // 使用 CLOCK_MONOTONIC 获取单调递增的时间
    return time;
}

/* 计算两个 timespec_t 之间的时间差，结果以秒为单位 */
static inline double time_diff(timespec_t start, timespec_t end) {
    double start_sec = start.tv_sec + start.tv_nsec / 1e9;
    double end_sec = end.tv_sec + end.tv_nsec / 1e9;
    return end_sec - start_sec;
}

int __wrap_main();

#ifdef __x86_64__

#define TICKS_MAX UINT64_MAX

typedef uint64_t ticks;

static inline __attribute__((always_inline)) ticks get_CPU_Cycle() {
  uint32_t lo, hi;
  asm volatile("rdtsc" : "=a"(lo), "=d"(hi));

  return ((ticks)hi << 32) | lo;
}
#endif

#ifdef __aarch64__
typedef uint64_t ticks;
#define TICKS_MAX UINT32_MAX

static inline __attribute__((always_inline)) ticks get_CPU_Cycle() {
  ticks val;
  asm volatile("mrs %0, pmccntr_el0" : "=r"(val));
  return val;
}
#endif

int main() {
  // ticks start = 0, end = 0;
  // start = get_CPU_Cycle();
  // __wrap_main();
  // end = get_CPU_Cycle();
  // fprintf(stderr, "CPU cycles: %lu\n", end - start);
  // return 0;
  timespec_t start, end;
  start = get_time();
  __wrap_main();
  end = get_time();
  fprintf(stderr, "Execution time: %f seconds\n", time_diff(start, end));
  return 0;
}
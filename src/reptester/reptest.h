#ifndef REPTEST_H
#define REPTEST_H

#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>

typedef enum {
    ALLOC_NONE,
    ALLOC_MALLOC,
    ALLOC_MMAP,
    ALLOC_KIND_COUNT,
} AllocKind;

typedef enum {
    TEST_MODE_UNINITIALIZED,
    TEST_MODE_TESTING,
    TEST_MODE_COMPLETED,
    TEST_MODE_ERROR,
} TestMode;

typedef struct RepetitionTestResults RepetitionTestResults;
struct RepetitionTestResults
{
    u64 test_count;
    u64 total_time;
    u64 min;
    u64 max;
};

typedef struct RepetitionTester RepetitionTester;
struct RepetitionTester
{
    TestMode mode;
    u8 print_new_min;
    u32 open_block_count;
    u32 close_block_count;
    u64 bytes_accumulated_this_test;
    u64 target_processed_byte_count;
    u64 time_accumulated_this_test;
    u64 cpu_timer_frequency;
    u64 try_for_time;
    u64 test_started_at;
    u64 page_fault_count_this_test;
    RepetitionTestResults results;
};

typedef struct ReadParams ReadParams;
struct ReadParams
{
    char *filepath; // we will need NULL-terminated string
    String8 filedata;
};

typedef void ReadOverHeadTestFunc(Arena *arena, RepetitionTester *tester, ReadParams *params, AllocKind kind);

typedef struct TestFunc TestFunc;
struct TestFunc
{
  String8 name;
  ReadOverHeadTestFunc *func;
};



u64 estimate_cpu_freq(void);
void error(Arena *arena, RepetitionTester *tester, String8 msg);
u8 is_testing(Arena *arena, RepetitionTester *tester);
void new_test_wave(Arena *arena, RepetitionTester *tester, u64 target_byte_count, u64 cpu_timer_freq, u32 seconds_to_try);
void begin_time(RepetitionTester *tester);
void end_time(RepetitionTester *tester);
void print_single_result(Arena *arena, String8 label, f64 cpu_time, u64 cpu_timer_freq, u64 byte_count, u64 page_fault_count);
void print_results(Arena *arena, RepetitionTestResults results, u64 cpu_timer_freq, u64 byte_count);
f64 seconds_from_cpu_time(f64 cpu_time, u64 cpu_timer_freq);
s32 perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags);
u8 *handle_allocation(AllocKind kind, ReadParams *params);
void handle_release(AllocKind kind, ReadParams *params, u8 *buffer);



#endif

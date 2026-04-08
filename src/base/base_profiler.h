#ifndef BASE_PROFILER_H
#define BASE_PROFILER_H

#include <x86intrin.h>
#include <sys/time.h>

#define PROFILER_DEFUALT_BACKING_BUFFER_SIZE (MEBIBYTE(1))
#define PROFILER_DEFUALT_BLOCK_COUNT ((u64)128)
#define OS_TIMER_FREQUENCY ((u64)1000000)

typedef struct profiler_block profiler_block;
struct profiler_block
{
	String8 *name;
	u64 tsc_start;
	u64 tsc_end;
};

typedef struct profiler profiler;
struct profiler
{
	Arena arena;
	s32 fd;
	profiler_block *blocks;
	u64 block_count;
	u64 os_time_start;
	u64 os_time_end;
	u64 profiler_tsc_start;
	u64 profiler_tsc_end;
};

void profiler_init(void);
void profiler_end_and_dump(void);
u64 profiler_block_begin(String8 name);
void profiler_block_end(u64 block_idx);
u64 get_os_timestamp_t(void);

#endif

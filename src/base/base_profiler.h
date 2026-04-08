#ifndef BASE_PROFILER_H
#define BASE_PROFILER_H

#include <x86intrin.h>
#include <sys/time.h>

#define PROFILER_MAX_BLOCK_COUNT ((u64)4096)
#define OS_TIMER_FREQUENCY ((u64)1000000)

typedef struct profiler_block profiler_block;
struct profiler_block
{
	String8 name;
	u64 tsc_elapsed;
	u64 hit_count;
};

typedef struct profiler profiler;
struct profiler
{
	u64 os_time_start;
	u64 os_time_end;
	u64 profiler_tsc_start;
	u64 profiler_tsc_end;
};

extern profiler_block g_profiler_blocks[PROFILER_MAX_BLOCK_COUNT];
extern profiler g_profiler;

u64 get_os_timestamp(void);
void profiler_init(void);
void profiler_end_and_dump(Arena *arena, s32 fd);

#define PROFILER_BEGIN(id_name) \
	static const u32 prof_loc_##id_name = __COUNTER__; \
	g_profiler_blocks[prof_loc_##id_name].name = STR8_LIT(#id_name); \
	u64 prof_start_##id_name = __rdtsc()

#define PROFILER_END(id_name) \
	u64 prof_end_##id_name = __rdtsc(); \
	g_profiler_blocks[prof_loc_##id_name].tsc_elapsed += (prof_end_##id_name - prof_start_##id_name); \
	g_profiler_blocks[prof_loc_##id_name].hit_count += 1

#endif

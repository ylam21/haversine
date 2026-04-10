#ifndef BASE_PROFILER_H
#define BASE_PROFILER_H

#include <x86intrin.h>
#include <sys/time.h>

#ifndef ENABLE_PROFILER
#define ENABLE_PROFILER 0
#endif

#define OS_TIMER_FREQUENCY ((u64)1000000)

#if ENABLE_PROFILER
#define PROFILER_MAX_BLOCK_COUNT ((u64)4096)
#define PROFILER_NULL_PARENT 0xFFFFFFFF

typedef struct profiler_block profiler_block;
struct profiler_block
{
	String8 name;
	u64 tsc_inclusive;
	u64 tsc_exclusive;
	u64 hit_count;
	u32 active_depth;
	u64 processed_byte_count;
};

extern profiler_block g_profiler_blocks[PROFILER_MAX_BLOCK_COUNT];
extern u32 g_profiler_current_parent;

#endif

typedef struct profiler profiler;
struct profiler
{
	u64 os_time_start;
	u64 os_time_end;
	u64 profiler_tsc_start;
	u64 profiler_tsc_end;
};

void profiler_init(void);
void profiler_end_and_print(Arena *arena, s32 fd);
u64 get_os_timestamp_t(void);

extern profiler g_profiler;

#if ENABLE_PROFILER
// 'TPUT' stands for throughput
#define PROFILER_BLOCK_TPUT_BEGIN(id_name, byte_count) \
	static const u32 prof_loc_##id_name = __COUNTER__; \
	g_profiler_blocks[prof_loc_##id_name].name = STR8_LIT(#id_name); \
	u32 prof_was_root_##id_name = (g_profiler_blocks[prof_loc_##id_name].active_depth == 0); \
	g_profiler_blocks[prof_loc_##id_name].active_depth += 1; \
	g_profiler_blocks[prof_loc_##id_name].processed_byte_count += byte_count; \
	u32 prof_parent_##id_name = g_profiler_current_parent; \
	g_profiler_current_parent = prof_loc_##id_name; \
	u64 prof_start_##id_name = __rdtsc()

#define PROFILER_BLOCK_BEGIN(id_name) PROFILER_BLOCK_TPUT_BEGIN(id_name, 0)

#define PROFILER_BLOCK_END(id_name) \
	u64 prof_end_##id_name = __rdtsc(); \
	u64 prof_elapsed_##id_name = prof_end_##id_name - prof_start_##id_name; \
	g_profiler_blocks[prof_loc_##id_name].active_depth -= 1; \
	if (prof_was_root_##id_name) { \
	    g_profiler_blocks[prof_loc_##id_name].tsc_inclusive += prof_elapsed_##id_name; \
	} \
	g_profiler_blocks[prof_loc_##id_name].tsc_exclusive += prof_elapsed_##id_name; \
	g_profiler_current_parent = prof_parent_##id_name; \
	if (g_profiler_current_parent != PROFILER_NULL_PARENT) { \
		g_profiler_blocks[g_profiler_current_parent].tsc_exclusive -= prof_elapsed_##id_name; \
	} \
	g_profiler_blocks[prof_loc_##id_name].hit_count += 1
#else

#define PROFILER_BLOCK_TPUT_BEGIN(id_name, byte_count)
#define PROFILER_BLOCK_BEGIN(id_name)
#define PROFILER_BLOCK_END(id_name)

#endif

#endif

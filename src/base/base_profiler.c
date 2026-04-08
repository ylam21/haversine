#include "base_profiler.h"
#include "base_str8fmt_builder/str8fmt_builder.h"
#include "base_string.h"
#include <unistd.h>
profiler g_profiler = {0};
profiler_block g_profiler_blocks[PROFILER_DEFUALT_BLOCK_COUNT] = {0};
u8 g_profiler_buffer[PROFILER_DEFUALT_BACKING_BUFFER_SIZE] = {0};

void profiler_init(void)
{
    arena_init(&g_profiler.arena, g_profiler_buffer, PROFILER_DEFUALT_BACKING_BUFFER_SIZE);
	g_profiler.blocks = g_profiler_blocks;
	g_profiler.os_time_start = get_os_timestamp_t();
	g_profiler.profiler_tsc_start = __rdtsc();
	g_profiler.fd = STDOUT_FILENO;
}

u64 profiler_block_begin(String8 name)
{
	String8 *s = arena_push(&g_profiler.arena, sizeof(String8));
	u8 *str = arena_push(&g_profiler.arena, name.size);
	if (s && str)
	{
		memcpy(str, name.str, name.size);
		s->str = str;
		s->size = name.size;
		u64 block_idx = g_profiler.block_count;
		profiler_block *b = &g_profiler.blocks[block_idx];
		b->name = s;
		b->tsc_start = __rdtsc();
		g_profiler.block_count += 1;
		return block_idx;
	}
	return -1; // Error
}

void profiler_block_end(u64 block_idx)
{
	profiler_block *b = &g_profiler.blocks[block_idx];
	b->tsc_end = __rdtsc();
}

void profiler_end_and_dump(void)
{
   	g_profiler.os_time_end = get_os_timestamp_t();
	g_profiler.profiler_tsc_end = __rdtsc();
	u64 os_time_elapsed = g_profiler.os_time_end - g_profiler.os_time_start;
	if (os_time_elapsed)
	{
	    u64 cpu_elapsed = g_profiler.profiler_tsc_end - g_profiler.profiler_tsc_start;
		u64 cpu_freq = OS_TIMER_FREQUENCY * cpu_elapsed / os_time_elapsed;
        str8fmt_write(g_profiler.fd, &g_profiler.arena, STR8_LIT("Total time: %u ms (CPU frequency: %u)\n"), os_time_elapsed, cpu_freq);
		u64 i = 0;
		while (i < g_profiler.block_count)
		{
			profiler_block *b = &g_profiler.blocks[i];
			u64 block_elapsed = b->tsc_end - b->tsc_start;
			u64 block_freq = OS_TIMER_FREQUENCY * block_elapsed / os_time_elapsed;

			String8 name_wsuffix = str8fmt(&g_profiler.arena, STR8_LIT("%s:"), *b->name);
			str8fmt_write(g_profiler.fd, &g_profiler.arena, STR8_LIT("%-10s %u (%.2f%%)\n"), name_wsuffix, block_freq, (f64)block_freq/(f64)cpu_freq * 100);
			i += 1;
		}
	}
}

u64 get_os_timestamp_t(void)
{
	struct timeval val;
	gettimeofday(&val, 0);
	return (OS_TIMER_FREQUENCY * (u64)val.tv_sec) + (u64)val.tv_usec;
}

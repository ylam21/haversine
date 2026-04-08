#include "base_profiler.h"
#include "base_string.h"
profiler_block g_profiler_blocks[PROFILER_MAX_BLOCK_COUNT] = {0};
profiler g_profiler = {0};

u64 get_os_timestamp(void)
{
	struct timeval val;
	gettimeofday(&val, 0);
	return (OS_TIMER_FREQUENCY * (u64)val.tv_sec) + (u64)val.tv_usec;
}

void profiler_init(void)
{
	g_profiler.os_time_start = get_os_timestamp();
	g_profiler.profiler_tsc_start = __rdtsc();
}

void profiler_end_and_dump(Arena *arena, s32 fd)
{
	g_profiler.os_time_end = get_os_timestamp();
	g_profiler.profiler_tsc_end = __rdtsc();

	u64 os_time_elapsed = g_profiler.os_time_end - g_profiler.os_time_start;
	u64 total_cpu_elapsed = g_profiler.profiler_tsc_end - g_profiler.profiler_tsc_start;

	if (os_time_elapsed > 0 && total_cpu_elapsed > 0)
	{
		u64 cpu_freq = OS_TIMER_FREQUENCY * total_cpu_elapsed / os_time_elapsed;

		str8fmt_write(fd, arena, STR8_LIT("\n--- PROFILING RESULTS ---\n"));
		str8fmt_write(fd, arena, STR8_LIT("Total time: %u millisec (CPU freq: %u)\n\n"), os_time_elapsed, cpu_freq);

		u32 i = 0;
		while (i < PROFILER_MAX_BLOCK_COUNT)
		{
			profiler_block *b = &g_profiler_blocks[i];
			if (b->hit_count > 0)
			{
				f64 percent = ((f64)b->tsc_elapsed / (f64)total_cpu_elapsed) * 100.0;
				str8fmt_write(fd, arena, STR8_LIT("%-15s: %-10u cycles | Hits %-8u | %.2f%%\n"), b->name, b->tsc_elapsed, b->hit_count, percent);
			}
			i += 1;
		}
		str8fmt_write(fd, arena, STR8_LIT("-------------------------\n"));
	}
}

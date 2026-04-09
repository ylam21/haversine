profiler g_profiler = {0};

#if ENABLE_PROFILER
profiler_block g_profiler_blocks[PROFILER_MAX_BLOCK_COUNT] = {0};
u32 g_profiler_current_parent = PROFILER_NULL_PARENT;
#endif

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

void profiler_end_and_print(Arena *arena, s32 fd)
{
	g_profiler.os_time_end = get_os_timestamp();
	g_profiler.profiler_tsc_end = __rdtsc();

	u64 os_time_elapsed = g_profiler.os_time_end - g_profiler.os_time_start;
	u64 total_cpu_elapsed = g_profiler.profiler_tsc_end - g_profiler.profiler_tsc_start;

	if (os_time_elapsed > 0 && total_cpu_elapsed > 0)
	{
		u64 cpu_freq = OS_TIMER_FREQUENCY * total_cpu_elapsed / os_time_elapsed;

		str8fmt_write(fd, arena, STR8_LIT("\n--- PROFILING RESULTS ---\n"));
		str8fmt_write(fd, arena, STR8_LIT("Total time: %u milliseconds (CPU freq: %u)\n"), os_time_elapsed / 1000, cpu_freq);

		#if ENABLE_PROFILER
		u32 i = 0;
		while (i < PROFILER_MAX_BLOCK_COUNT)
		{
			profiler_block *b = &g_profiler_blocks[i];
			if (b->hit_count > 0)
			{
				f64 inc_percent = ((f64)b->tsc_inclusive / (f64)total_cpu_elapsed) * 100.0;
				f64 exc_percent = ((f64)b->tsc_exclusive / (f64)total_cpu_elapsed) * 100.0;

				f64 gib_per_sec = 0;
				if (b->processed_byte_count > 0)
				{
				    f64 seconds = (f64)b->tsc_inclusive / (f64)cpu_freq;
					f64 bytes_per_sec = b->processed_byte_count / seconds;
					gib_per_sec = bytes_per_sec / (1024.0 * 1024.0 * 1024.0);
				}
				if (inc_percent == exc_percent)
				{
				    // Note: This block does not have any nested/children blocks
					str8fmt_write(fd, arena, STR8_LIT("%-20s: %-12u cycles [%5.2f%%] | %-19s | Hits %-8u"),
					b->name,
					b->tsc_inclusive, inc_percent,
					STR8_LIT("No children blocks"),
					b->hit_count);
				}
				else
				{
				    str8fmt_write(fd, arena, STR8_LIT("%-20s: %-12u cycles [%5.2f%%] | Exclusive: [%5.2f%%] | Hits %-8u"),
			        b->name,
				    b->tsc_inclusive, inc_percent,
			        exc_percent,
				    b->hit_count);
				}
				if (b->processed_byte_count > 0)
				{
				    str8fmt_write(fd, arena, STR8_LIT(" | %5.2f GiB/s\n"), gib_per_sec);
				}
				else
				{
                    str8fmt_write(fd, arena, STR8_LIT("\n"));
				}
			}
			i += 1;
		}
		#endif
		str8fmt_write(fd, arena, STR8_LIT("-------------------------\n"));
	}
}

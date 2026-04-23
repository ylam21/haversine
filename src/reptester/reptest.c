#pragma once
#include "../root.unity.h"

u64 estimate_cpu_freq(void)
{
	u64 wait_time_ms = 100;
	u64 os_freq = OS_TIMER_FREQUENCY;
	u64 os_ticks_during_wait_time = os_freq * wait_time_ms / 1000;
	u64 os_elapsed = 0;
	u64 os_start = os_get_timestamp();
	u64 cpu_start = __rdtsc();

	while (os_elapsed < os_ticks_during_wait_time) {
		os_elapsed = os_get_timestamp() - os_start;
	}

	u64 cpu_freq = (__rdtsc() - cpu_start) * os_freq / os_elapsed;

	return cpu_freq;
}

// https://man7.org/linux/man-pages/man2/perf_event_open.2.html
s32 perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags)
{
  int ret = syscall(SYS_perf_event_open, hw_event, pid, cpu,group_fd, flags);
  return ret;
}

void print_results(Arena *arena, RepetitionTestResults results, u64 cpu_timer_freq, u64 byte_count)
{
    print_single_result(arena, STR8_LIT("\tMIN"), results.min,  cpu_timer_freq, byte_count, 0);
    print_single_result(arena, STR8_LIT("\tMAX"), results.max,  cpu_timer_freq, byte_count, 0);

    if (results.test_count)
    {
        f64 test_count = results.test_count;
        print_single_result(arena, STR8_LIT("\tAVG"), results.total_time / test_count, cpu_timer_freq, byte_count, 0);
    }
}

f64 seconds_from_cpu_time(f64 cpu_time, u64 cpu_timer_freq)
{
	f64 seconds = 0.0;
	if (cpu_timer_freq) {
		seconds = cpu_time / (f64)cpu_timer_freq;
	}
	return seconds;
}

void print_single_result(Arena *arena, String8 label, f64 cpu_time, u64 cpu_timer_freq, u64 byte_count, u64 page_fault_count)
{
    Temp temp = temp_begin(arena);
    str8fmt_write(STDOUT_FILENO, arena, STR8_LIT("%s: %.0f"), label, cpu_time);
    if (cpu_timer_freq)
    {
        f64 seconds = seconds_from_cpu_time(cpu_time, cpu_timer_freq);
        str8fmt_write(STDOUT_FILENO, arena, STR8_LIT(" (%fms)"), 1000.0f * seconds);

        if (byte_count)
        {
            f64 best_bandwidth = byte_count / (GIBIBYTE(1) * seconds);
            f64 mib_count = (f64)byte_count / (f64)MEBIBYTE(1);
            String8 page_fault_str = STR8_LIT("");
            if (page_fault_count)
            {
                page_fault_str = str8fmt(arena, STR8_LIT("\t| %u bytes / page fault (total: %u)"), byte_count / page_fault_count, page_fault_count);
            }
            str8fmt_write(STDOUT_FILENO, arena, STR8_LIT("   %5.3fMib at %fgb/s%s\n"), mib_count, best_bandwidth, page_fault_str);
        }
    }
    temp_end(temp);
}

u8 *handle_allocation(AllocKind kind, ReadParams *params)
{
    switch (kind)
    {
        case ALLOC_NONE:
            return params->filedata.str;
        case ALLOC_MALLOC:
            return  malloc(params->filedata.size);
        case ALLOC_MMAP:
            return mmap(NULL, params->filedata.size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        default:
            return 0;
    }
}

void handle_release(AllocKind kind, ReadParams *params, u8 *buffer)
{
    switch (kind)
    {
        case ALLOC_NONE:
            return;
        case ALLOC_MALLOC:
            free(buffer);
            return;
        case ALLOC_MMAP:
            munmap(buffer, params->filedata.size);
        default:
            return;
    }
}

void error(Arena *arena, RepetitionTester *tester, String8 msg)
{
    tester->mode = TEST_MODE_ERROR;
    str8fmt_write(STDERR_FILENO, arena, STR8_LIT("Error: %s\n"), msg);
    exit(1);
}

u8 is_testing(Arena *arena, RepetitionTester *tester)
{
    if (tester->mode != TEST_MODE_TESTING) return 0;

    u64 current_cpu_time = __rdtsc();

    if (tester->open_block_count)
    {
        if (tester->open_block_count != tester->close_block_count)
        {
            error(arena, tester, STR8_LIT("Unbalanced begin_time/end_time"));
        }
        if (tester->bytes_accumulated_this_test != tester->target_processed_byte_count)
        {
            error(arena, tester, STR8_LIT("Processed byte count mismatch"));
        }
        if (tester->mode == TEST_MODE_TESTING)
        {
            RepetitionTestResults *results = &tester->results;
            results->test_count += 1;
            u64 elapsed = tester->time_accumulated_this_test;
            results->total_time += elapsed;
            if (elapsed > results->max)
            {
                results->max = elapsed;
            }
            if (elapsed < results->min)
            {
                results->min = elapsed;
                tester->test_started_at = current_cpu_time;
                if (tester->print_new_min)
                {
                    print_single_result(arena,
                        STR8_LIT("MIN"),
                        results->min,
                        tester->cpu_timer_frequency,
                        tester->bytes_accumulated_this_test,
                        tester->page_fault_count_this_test);
                }
            }

            tester->open_block_count = 0;
            tester->close_block_count = 0;
            tester->time_accumulated_this_test = 0;
            tester->bytes_accumulated_this_test = 0;
            tester->page_fault_count_this_test = 0;
        }
    }

    if ((current_cpu_time - tester->test_started_at) > tester->try_for_time)
    {
        tester->mode = TEST_MODE_COMPLETED;
        print_results(arena, tester->results, tester->cpu_timer_frequency, tester->target_processed_byte_count);
        return 0;
    }

    return 1;
}

void new_test_wave(Arena *arena, RepetitionTester *tester, u64 target_byte_count, u64 cpu_timer_freq, u32 seconds_to_try)
{
    // reset state in tester
    if (tester->mode == TEST_MODE_UNINITIALIZED)
    {
        tester->mode = TEST_MODE_TESTING;
        tester->target_processed_byte_count = target_byte_count;
        tester->cpu_timer_frequency = cpu_timer_freq;
        tester->print_new_min = 1;
        tester->results.min = UINT64_MAX;
    }
    else if (tester->mode == TEST_MODE_COMPLETED)
    {
        tester->mode = TEST_MODE_TESTING;
        if (tester->target_processed_byte_count != target_byte_count)
        {
            error(arena, tester, STR8_LIT("target_processed_byte_count changed"));
        }
        if (tester->cpu_timer_frequency != cpu_timer_freq)
        {
            error(arena, tester, STR8_LIT("cpu_timer_frequency changed"));
        }

        tester->results.test_count = 0;
        tester->results.total_time = 0;
        tester->results.max = 0;
        tester->results.min = UINT64_MAX;
        tester->open_block_count = 0;
        tester->close_block_count = 0;
        tester->time_accumulated_this_test = 0;
        tester->bytes_accumulated_this_test= 0;
        tester->page_fault_count_this_test = 0;
    }

    tester->try_for_time = seconds_to_try * cpu_timer_freq;
    tester->test_started_at = __rdtsc();
}

void begin_time(RepetitionTester *tester)
{
    tester->open_block_count += 1;
    tester->time_accumulated_this_test -= __rdtsc();
}

void end_time(RepetitionTester *tester)
{
    tester->close_block_count += 1;
    tester->time_accumulated_this_test += __rdtsc();
}

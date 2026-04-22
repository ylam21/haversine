#include "../root.unity.h"
#include <sys/mman.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>

#define READ_BUFFER_SIZE KIBIBYTE(1)

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

u64 estimate_cpu_freq(void);

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

void error(Arena *arena, RepetitionTester *tester, String8 msg);
u8 is_testing(Arena *arena, RepetitionTester *tester);
void new_test_wave(Arena *arena, RepetitionTester *tester, u64 target_byte_count, u64 cpu_timer_freq, u32 seconds_to_try);
void begin_time(RepetitionTester *tester);
void end_time(RepetitionTester *tester);

void read_via_read(Arena *arena, RepetitionTester *tester, ReadParams *params, AllocKind kind);
void read_via_fread(Arena *arena, RepetitionTester *tester, ReadParams *params, AllocKind kind);
void write_to_all_bytes(Arena *arena, RepetitionTester *tester, ReadParams *params, AllocKind kind);

void print_single_result(Arena *arena, String8 label, f64 cpu_time, u64 cpu_timer_freq, u64 byte_count, u64 page_fault_count);
void print_results(Arena *arena, RepetitionTestResults results, u64 cpu_timer_freq, u64 byte_count);
f64 seconds_from_cpu_time(f64 cpu_time, u64 cpu_timer_freq);
s32 perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags);

u8 *handle_allocation(AllocKind kind, ReadParams *params);
void handle_release(AllocKind kind, ReadParams *params, u8 *buffer);

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
            str8fmt_write(STDOUT_FILENO, arena, STR8_LIT("   %5.3fMib %fgb/s%s\n"), mib_count, best_bandwidth, page_fault_str);
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

void write_to_all_bytes(Arena *arena, RepetitionTester *tester, ReadParams *params, AllocKind kind)
{
    while (is_testing(arena, tester))
    {
        u8 *backing_buffer = handle_allocation(kind, params);
        if (!backing_buffer)
        {
            error(arena, tester, STR8_LIT("fread failed alloc"));
            break;
        }

        struct perf_event_attr pe;
        memset(&pe, 0, sizeof(pe));
        pe.type = PERF_TYPE_SOFTWARE;
        pe.config = PERF_COUNT_SW_PAGE_FAULTS;
        pe.disabled = 1;
        pe.exclude_kernel = 0;
        s32 pffd = perf_event_open(&pe, 0, -1, -1, 0);
        if (pffd == -1)
        {
            perror("perf_event_open");
            error(arena, tester, STR8_LIT("read failed perf_event_open"));
            break;
        }
        ioctl(pffd, PERF_EVENT_IOC_RESET, 0);
        ioctl(pffd, PERF_EVENT_IOC_ENABLE, 0);

        begin_time(tester);
        u64 i = 0;
        while (i < params->filedata.size)
        {
            backing_buffer[i] = 1;
            i += 1;
        }
        end_time(tester);

        ioctl(pffd, PERF_EVENT_IOC_DISABLE, 0);
        u64 page_faults_count = 0;
        read(pffd, &page_faults_count, sizeof(page_faults_count));
        close(pffd);

        handle_release(kind, params, backing_buffer);

        tester->bytes_accumulated_this_test += params->filedata.size;
        tester->page_fault_count_this_test += page_faults_count;
    }
}


void read_via_fread(Arena *arena, RepetitionTester *tester, ReadParams *params, AllocKind kind)
{
    while (is_testing(arena, tester))
    {
        FILE *file = fopen(params->filepath, "r");
        if (!file)
        {
            error(arena, tester, STR8_LIT("fread failed 1"));
            break;
        }

        u8 *backing_buffer = handle_allocation(kind, params);
        if (!backing_buffer)
        {
            error(arena, tester, STR8_LIT("fread failed alloc"));
            break;
        }

        struct perf_event_attr pe;
        memset(&pe, 0, sizeof(pe));
        pe.type = PERF_TYPE_SOFTWARE;
        pe.config = PERF_COUNT_SW_PAGE_FAULTS;
        pe.disabled = 1;
        pe.exclude_kernel = 0;
        s32 pffd = perf_event_open(&pe, 0, -1, -1, 0);
        if (pffd == -1)
        {
            perror("perf_event_open");
            error(arena, tester, STR8_LIT("read failed perf_event_open"));
            break;
        }
        ioctl(pffd, PERF_EVENT_IOC_RESET, 0);
        ioctl(pffd, PERF_EVENT_IOC_ENABLE, 0);

        begin_time(tester);
        u64 read_bytes = fread(backing_buffer, 1, params->filedata.size, file);
        end_time(tester);

        ioctl(pffd, PERF_EVENT_IOC_DISABLE, 0);
        u64 page_faults_count = 0;
        read(pffd, &page_faults_count, sizeof(page_faults_count));
        close(pffd);

        fclose(file);
        handle_release(kind, params, backing_buffer);

        if (read_bytes != params->filedata.size)
        {
            error(arena, tester, STR8_LIT("fread failed 2"));
            break;

        }

        tester->bytes_accumulated_this_test += params->filedata.size;
        tester->page_fault_count_this_test += page_faults_count;
    }
}

void read_via_read(Arena *arena, RepetitionTester *tester, ReadParams *params, AllocKind kind)
{
    while (is_testing(arena, tester))
    {
        s32 fd = open(params->filepath, O_RDONLY, 0777);
        if (fd == -1)
        {
            error(arena, tester, STR8_LIT("read failed 1"));
            break;
        }

        u8 *backing_buffer = handle_allocation(kind, params);
        if (!backing_buffer)
        {
            error(arena, tester, STR8_LIT("read failed alloc"));
            break;
        }

        struct perf_event_attr pe;
        memset(&pe, 0, sizeof(pe));
        pe.type = PERF_TYPE_SOFTWARE;
        pe.config = PERF_COUNT_SW_PAGE_FAULTS;
        pe.disabled = 1;
        pe.exclude_kernel = 0;
        s32 pffd = perf_event_open(&pe, 0, -1, -1, 0);
        if (pffd == -1)
        {
          perror("perf_event_open");
          error(arena, tester, STR8_LIT("read failed perf_event_open"));
          break;
        }
        ioctl(pffd, PERF_EVENT_IOC_RESET, 0);
        ioctl(pffd, PERF_EVENT_IOC_ENABLE, 0);

        begin_time(tester);
        s64 read_bytes = read(fd, backing_buffer, params->filedata.size);
        end_time(tester);


        ioctl(pffd, PERF_EVENT_IOC_DISABLE, 0);
        u64 page_faults_count = 0;
        read(pffd, &page_faults_count, sizeof(page_faults_count));
        close(pffd);

        close(fd);
        handle_release(kind, params, backing_buffer);

        if ((u64)read_bytes != params->filedata.size)
        {
            error(arena, tester, STR8_LIT("read failed 2"));
            break;
        }

        tester->bytes_accumulated_this_test += params->filedata.size;
        tester->page_fault_count_this_test += page_faults_count;
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

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s [existing filename]\n", argv[0]);
        return 1;
    }

    char *file_name = argv[1];

    TestFunc test_functions[] = {
        {STR8_LIT("write"), write_to_all_bytes},
        {STR8_LIT("read"),  read_via_read},
        {STR8_LIT("fread"), read_via_fread},
    };

    u64 file_size = os_file_size(file_name);

    Arena *arena = arena_alloc(MEBIBYTE(1) + file_size);
    if (!arena) return 1;

    u8 *buffer = malloc(file_size);
    if (!buffer) return 1;

    ReadParams params = {
        .filepath = file_name,
        .filedata = {
            .str = buffer,
            .size = file_size,
        },
    };

    RepetitionTester tester = {0};

    u32 seconds_to_try = 10;
    u64 cpu_timer_freq = estimate_cpu_freq();

    String8 kind_names[3] = {
        STR8_LIT(""),
        STR8_LIT("+ malloc"),
        STR8_LIT("+ mmap"),
    };

    Temp temp = temp_begin(arena);
    while (1)
    {
        temp_end(temp);
        u32 i = 0;
        u32 end = ArrayCount(test_functions);
        while (i < end)
        {
            u8 kind = 0;
            while (kind < ALLOC_KIND_COUNT)
            {
                TestFunc test_func = test_functions[i];
                str8fmt_write(STDOUT_FILENO, arena, STR8_LIT("\n---New Test Wave: %s %s---\n"), test_func.name, kind_names[kind]);
                new_test_wave(arena, &tester, params.filedata.size, cpu_timer_freq, seconds_to_try);
                test_func.func(arena, &tester, &params, kind);

                kind += 1;
            }

            i += 1;
        }
    }

    return 0;
}

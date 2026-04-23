#include "../root.unity.h"
#include "reptest.h"

#include "reptest.c"

void read_via_read(Arena *arena, RepetitionTester *tester, ReadParams *params, AllocKind kind);
void read_via_fread(Arena *arena, RepetitionTester *tester, ReadParams *params, AllocKind kind);
void write_to_all_bytes(Arena *arena, RepetitionTester *tester, ReadParams *params, AllocKind kind);
void write_to_all_bytes_back(Arena *arena, RepetitionTester *tester, ReadParams *params, AllocKind kind);

void write_to_all_bytes_back(Arena *arena, RepetitionTester *tester, ReadParams *params, AllocKind kind)
{
    while (is_testing(arena, tester))
    {
        u8 *backing_buffer = handle_allocation(kind, params);
        if (!backing_buffer)
        {
            error(arena, tester, STR8_LIT("writeback failed alloc"));
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
            error(arena, tester, STR8_LIT("writeback failed perf_event_open"));
            break;
        }
        ioctl(pffd, PERF_EVENT_IOC_RESET, 0);
        ioctl(pffd, PERF_EVENT_IOC_ENABLE, 0);

        begin_time(tester);
        u64 i = 0;
        u64 size = params->filedata.size;
        while (i < size)
        {
            backing_buffer[size - 1 - i] = i;
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

void write_to_all_bytes(Arena *arena, RepetitionTester *tester, ReadParams *params, AllocKind kind)
{
    while (is_testing(arena, tester))
    {
        u8 *backing_buffer = handle_allocation(kind, params);
        if (!backing_buffer)
        {
            error(arena, tester, STR8_LIT("write failed alloc"));
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
            error(arena, tester, STR8_LIT("write failed perf_event_open"));
            break;
        }
        ioctl(pffd, PERF_EVENT_IOC_RESET, 0);
        ioctl(pffd, PERF_EVENT_IOC_ENABLE, 0);

        begin_time(tester);
        u64 i = 0;
        u64 size = params->filedata.size;
        while (i < size)
        {
            backing_buffer[i] = i;
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
        {STR8_LIT("write_back"), write_to_all_bytes_back},
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

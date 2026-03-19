#include "base/base_inc.h"
#include "parser/parser_inc.h"
#include "timer/timer_inc.h"
#include "generator/generator_inc.h"

#include "base/base_inc.c"
#include "parser/parser_inc.c"
#include "timer/timer_inc.c"
#include "generator/generator_inc.c"

#define PROGRAM_NAME_HAVERSINE "haversine"

void print_usage(void)
{
	fprintf(stdout,"Usage: ./%s <filename.json>\n", PROGRAM_NAME_HAVERSINE);
}

int main(int argc, char **argv)
{
	cpu_freq cf;
	cpu_freq_start(&cf);

	// TIMESTAMP: STARTUP START
	u64 cfts_startup_start = __rdtsc();

	if (argc != 2)
	{
		print_usage();
		return 0;
	}

	s32 fd = open(argv[1], O_RDONLY, 0777);
	if (fd == -1)
	{
		fprintf(stderr, "Error: cannot open %s\n", argv[1]);
		return 1;
	}
	
	u64 buffer_size = GB(1);
	u8 *buffer = (u8 *)malloc(buffer_size);
	if (buffer == NULL)
	{
		perror("malloc");
		return 1;
	}
	u64 cfts_startup_end = __rdtsc();
	// TIMESTAMP: STARTUP END

	// TIMESTAMP: READ START
	u64 cfts_read_start = __rdtsc();

	s32 read_bytes = read(fd, buffer, buffer_size);

	u64 cfts_read_end = __rdtsc();
	// TIMESTAMP: READ END 

	// TIMESTAMP: MISC SETUP START 
	u64 cfts_misc_setup_start = __rdtsc();

	if (read_bytes == -1)
	{
		fprintf(stderr, "Error: cannot read %s\n", argv[1]);
		return 1;
	}

	String8 json = (String8){.str = buffer, .size = (u64)read_bytes};
	Arena *arena = arena_alloc(GB(1));
	if (arena == 0)
	{
		return 1;
	}

	u64 cfts_misc_setup_end = __rdtsc();
	// TIMESTAMP: MISC SETUP END

	// TIMESTAMP: PARSE START
	u64 cfts_parse_start = __rdtsc();

	f64_array arr = json_parse_to_buffer(arena, json);

	u64 cfts_parse_end = __rdtsc();
	// TIMESTAMP: PARSE END

	// TIMESTAMP: SUM START 
	u64 cfts_sum_start = __rdtsc();

	f64 x0, y0, x1, y1;

	f64 haversine_distance;
	f64 sum = 0;
	u64 n_pairs = arr.count / 4;
	f64 sum_coef = 1.0 / (f64)n_pairs;

	u64 count = 0;
	while (count < arr.count)
	{
		x0 = arr.arr[0];
		y0 = arr.arr[1];
		x1 = arr.arr[2];
		y1 = arr.arr[3];

		haversine_distance = ReferenceHaversine(x0, y0, x1, y1, 6372.8);
		sum += sum_coef * haversine_distance;

		arr.arr += 4;
		count += 4;
	}

	u64 cfts_sum_end = __rdtsc();
	// TIMESTAMP: SUM END 


	cpu_freq_end(&cf);
	u64 cpu_freq = cpu_freq_get(&cf);

	u64 startup_freq = cpu_freq_from_ts(&cf, cfts_startup_start, cfts_startup_end);
	u64 read_freq = cpu_freq_from_ts(&cf, cfts_read_start, cfts_read_end);
	u64 misc_setup_freq = cpu_freq_from_ts(&cf, cfts_misc_setup_start, cfts_misc_setup_end);
	u64 parse_freq = cpu_freq_from_ts(&cf, cfts_parse_start, cfts_parse_end);
	u64 sum_freq = cpu_freq_from_ts(&cf, cfts_sum_start, cfts_sum_end);


	printf("Input size: %u\n", read_bytes);
	printf("Pair count: %lu\n", n_pairs);
	printf("Haversine sum: %.16f\n\n", sum);

	printf("Total time: %lu (CPU freq: %lu)\n", cf.os_time_start - cf.os_time_end, cpu_freq);
	printf(" Startup: %lu (%.2f%%)\n", startup_freq, (f64)startup_freq / (f64)cpu_freq * 100);
	printf(" Read: %lu (%.2f%%)\n", read_freq, (f64)read_freq / (f64)cpu_freq * 100);
	printf(" MiscSetup: %lu (%.2f%%)\n", misc_setup_freq, (f64)misc_setup_freq / (f64)cpu_freq * 100);
	printf(" Parse: %lu (%.2f%%)\n", parse_freq, (f64)parse_freq / (f64)cpu_freq * 100);
	printf(" Sum: %lu (%.2f%%)\n", sum_freq, (f64)sum_freq / (f64)cpu_freq * 100);

	return 0;
}
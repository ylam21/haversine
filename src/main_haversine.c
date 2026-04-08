#include "base/base_inc.h"
#include "parser/parser_inc.h"
#include "generator/generator_inc.h"

#include "base/base_inc.c"
#include "parser/parser_inc.c"
#include "generator/generator_inc.c"

#define PROGRAM_NAME_HAVERSINE "haversine"

void print_usage(void)
{
    fprintf(stdout,"Usage: ./%s <filename.json>\n", PROGRAM_NAME_HAVERSINE);
}

int main(int argc, char **argv)
{
	Arena *life_arena = arena_alloc(KIBIBYTE(1));
	if (!life_arena) return 0;

	profiler_init();

	PROFILER_BEGIN(startup);

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

	u64 buffer_size = GIBIBYTE(1);
	u8 *buffer = (u8 *)malloc(buffer_size);
	if (buffer == NULL)
	{
		perror("malloc");
		return 1;
	}

    PROFILER_END(startup);

    PROFILER_BEGIN(read);

	s32 read_bytes = read(fd, buffer, buffer_size);
	if (read_bytes == -1) return 0;

	PROFILER_END(read);

	PROFILER_BEGIN(misc);

	if (read_bytes == -1)
	{
		fprintf(stderr, "Error: cannot read %s\n", argv[1]);
		return 1;
	}

	String8 json = (String8){.str = buffer, .size = (u64)read_bytes};
	Arena *arena = arena_alloc(GIBIBYTE(1));
	if (arena == 0)
	{
		return 1;
	}

	PROFILER_END(misc);

	PROFILER_BEGIN(parse);

	f64_array arr = json_parse_to_buffer(arena, json);
	assert((arr.count & 0x3) == 0);  // Exit the program if count is not divisible by 4, since 4 floats form 2 pairs

	PROFILER_END(parse);

	PROFILER_BEGIN(sum);

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
		// Note: What if I would be accessing 8 floats at one loop-cycle?
		// Would this increase performance? Since size of one cache line is 64 bytes...

		haversine_distance = ReferenceHaversine(x0, y0, x1, y1, 6372.8);
		sum += sum_coef * haversine_distance;

		arr.arr += 4;
		count += 4;
	}

	PROFILER_END(sum);
	profiler_end_and_dump(arena, STDOUT_FILENO);

	return 0;
}

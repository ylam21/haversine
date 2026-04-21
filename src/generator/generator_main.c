#include "../root.unity.h"

#define PROGRAM_NAME_GEN "haversine_generator"

void print_usage(void)
{
	fprintf(stdout,"Usage: ./%s <uniform/cluster> <seed> <number_of_pairs>\n", PROGRAM_NAME_GEN);
}

int main(int argc, char **argv)
{
	if (argc != 4)
	{
		print_usage();
		return 0;
	}

	u64 seed = atoll(argv[2]);
	u64 n_pairs = atoll(argv[3]);

	u8 generatorTypeFlag = 0;
	if (strcmp(argv[1], "uniform") == 0)
	{
		generatorTypeFlag |= typeUniform;

	}
	else if (strcmp(argv[1], "cluster") == 0)
	{
		generatorTypeFlag |= typeCluster;
	}
	else
	{
		print_usage();
		return 0;
	}

	s32 fd = open("data.json", O_CREAT | O_TRUNC | O_WRONLY, 0777);
	if (fd == -1)
	{
		fprintf(stderr, "Error: cannot create a file\n");
		return 1;
	}

	s32 fd_ans= open("answers.f64", O_CREAT | O_TRUNC | O_WRONLY, 0777);
	if (fd == -1)
	{
		fprintf(stderr, "Error: cannot create a file answers.f64\n");
		return 1;
	}

	generate_and_write_data(fd, fd_ans, seed, n_pairs, generatorTypeFlag);

	return 0;
}

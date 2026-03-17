#include "../base/base_inc.h"
#include "parser_inc.h"

#include "../base/base_inc.c"
#include "parser_inc.c"

#define PROGRAM_NAME_PARSER "json_parse"

void print_usage(void)
{
	fprintf(stdout,"Usage: ./%s <filename.json>\n", PROGRAM_NAME_PARSER);
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		print_usage();
		return 0;
	}

	s32 fd_in = open(argv[1], O_RDONLY, 0777);
	if (fd_in == -1)
	{
		fprintf(stderr, "Error: cannot open %s\n", argv[1]);
		return 1;
	}

	s32 fd_out = open("parsed.f64", O_CREAT | O_TRUNC | O_WRONLY, 0777);
	if (fd_out == -1)
	{
		fprintf(stderr, "Error: cannot open %s\n", "parsed.f64");
		return 1;
	}

	u64 buffer_size = GB(1);
	u8 *buffer = (u8 *)malloc(buffer_size);
	if (buffer == NULL)
	{
		perror("malloc");
		return 1;
	}

	s32 read_bytes = read(fd_in, buffer, buffer_size);
	if (read_bytes == -1)
	{
		fprintf(stderr, "Error: cannot read %s\n", argv[1]);
		return 1;
	}

	String8 json = (String8){.str = buffer, .size = (u64)read_bytes};
	json_parse(fd_out, json);

	return 0;
}
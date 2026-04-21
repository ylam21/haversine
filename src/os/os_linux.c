u64 os_file_size(char *filepath)
{
    struct stat filestat;
    stat(filepath, &filestat);
    return filestat.st_size;
}

u64 os_get_timestamp(void)
{
	struct timeval val;
	gettimeofday(&val, 0);
	return (OS_TIMER_FREQUENCY * (u64)val.tv_sec) + (u64)val.tv_usec;
}

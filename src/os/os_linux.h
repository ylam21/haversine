#ifndef OS_LINUX_H
#define OS_LINUX_H

#define OS_TIMER_FREQUENCY ((u64)1000000)

#include <sys/stat.h>

u64 os_file_size(char *filepath);
u64 os_get_timestamp(void);

#endif

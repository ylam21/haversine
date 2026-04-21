#pragma once
#include "../root.unity.h"

u32 str8ncmp(String8 s1, String8 s2, u32 n)
{
    u32 i = 0;
    while (i < s1.size && i < s2.size && i < n && s1.str[i] == s2.str[i])
    {
        i += 1;
    }
    if (i == n)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

String8 str8fmt(Arena *arena, String8 fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String8 result = str8fmt_builder(arena, fmt, args);
    va_end(args);
    return result;
}

s32 str8fmt_write(s32 fd, Arena *arena, String8 fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String8 result = str8fmt_builder(arena, fmt, args);
    va_end(args);
    return write(fd, result.str, result.size);
}

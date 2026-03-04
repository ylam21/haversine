#ifndef STRJOIN_FMT_H
#define STRJOIN_FMT_H

typedef struct t_fmt_opt t_fmt_opt;

struct t_fmt_opt
{
    u8 padding_char;
    s32 width;
    u8 left_align;
    u8 is_conditional; // special handling for '?' prefix specifier
};

typedef void (*fmt_handler_t)(Arena *arena, va_list args, t_fmt_opt opt);

String8 str8_fmt(Arena *arena, String8 fmt, ...);
t_fmt_opt parse_options(String8 fmt, u64 *pos);

#endif
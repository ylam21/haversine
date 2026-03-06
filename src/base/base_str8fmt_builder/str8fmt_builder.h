#ifndef STR8FMT_BUILDER_H
#define STR8FMT_BUILDER_H

typedef struct t_fmt_opt t_fmt_opt;

struct t_fmt_opt
{
    u8 padding_char;
    s32 width;
    u8 left_align;
    s32 precision;
};

typedef void (*fmt_handler_t)(Arena *arena, va_list args, t_fmt_opt *opt);

String8 str8fmt_builder(Arena *arena, String8 fmt, va_list args);
t_fmt_opt parse_options(String8 fmt, u64 *pos);

#endif
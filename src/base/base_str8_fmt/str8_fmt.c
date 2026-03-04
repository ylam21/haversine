#include "specifier_table.c"

t_fmt_opt parse_options(String8 fmt, u64 *pos)
{
    t_fmt_opt opt = { .padding_char = CHAR_SPACE, .width = 0, .left_align = 0 , .is_conditional = 0};
    u8 *f = fmt.str;
    u64 p = *pos;

    if (f[p] == '?')
    {
        opt.is_conditional = 1;
        p += 1;
    }

    while (p < fmt.size && (f[p] == '-' || f [p]== '0'))
    {
        if (f[p] == '-')
        {
            opt.left_align = 1;
        }
        else if (f[p] == '0')
        {
            opt.padding_char = '0';
        }
        p += 1;
    }

    while (p < fmt.size && f[p] >= '0' && f[p] <= '9')
    {
        opt.width = ((opt.width << 3) + (opt.width << 1)) + f[p] - '0';
        p += 1;
    }

    *pos = p;

    return opt;
}

String8 str8_fmt(Arena *arena, String8 fmt, ...)
{ 
    u8 *start_ptr = (u8 *)arena->buffer + arena->pos;
    // NOTE:  Everything allocated onto the arena from this point
    //        is going to be included in the returned string!

    va_list args;
    va_start(args, fmt);

    u64 pos = 0;
    while (pos < fmt.size)
    {
        if (fmt.str[pos] == '%')
        {
            pos++;
            t_fmt_opt opt = parse_options(fmt, &pos);

            fmt_handler_t handler = specifier_table[fmt.str[pos]];
            if (handler)
            {
                handler(arena, args, opt);
            }
            else if (pos < fmt.size)
            {
                u8 *c = arena_push_packed(arena, 1);
                if (!c)
                {
                    return (String8){0};
                }
                *c = fmt.str[pos];
            }
        }
        else
        {
            u8 *c = arena_push_packed(arena, 1);
            if (!c)
            {
                return (String8){0};
            }
            *c = fmt.str[pos];
        }
        pos++;
    }
    va_end(args);

    u64 size = (u64)((u8 *)arena->buffer + arena->pos  - start_ptr);

    return (String8){ .str = start_ptr, .size = size};
}

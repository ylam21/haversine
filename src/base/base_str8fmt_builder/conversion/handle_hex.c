void handle_hex(Arena *arena, va_list args, t_fmt_opt *opt, u8 table[16])
{
    u32 val = va_arg(args, u32);

    u8 temp[32];
    u8 result[32];
    u32 len = 0;

    if (val == 0)
    {
        temp[len++] = '0';
    }
    else
    {
        while (val)
        {
            temp[len++] = table[val & 0xF];
            val >>= 4;
        }
    }

    u32 i = 0;
    while (i < len)
    {
        result[i] = temp[len - 1 - i];
        i += 1;
    }

    String8 s;
    s.str = result;
    s.size = len;

    apply_padding(arena, s, opt);
}

void handle_upx(Arena *arena, va_list args, t_fmt_opt *opt)
{
    handle_hex(arena, args, opt, (u8*)"0123456789ABCDEF");
}   

void handle_x(Arena *arena, va_list args, t_fmt_opt *opt)
{
    handle_hex(arena, args, opt, (u8*)"0123456789abcdef");
}
s32 my_stoa_buf(u8 *buf, s32 x)
{
    s32 i = 0;
    s64 nb = x;
    
    if (nb == 0)
    {
        buf[0] = '0';
        return 1;
    }

    if (nb < 0) nb = -nb;

    u8 temp[32];
    s32 temp_len = 0;
    while (nb)
    {
        temp[temp_len++] = (nb % 10) + '0';
        nb /= 10;
    }

    if (x < 0) buf[i++] = '-';
    
    while (temp_len > 0)
    {
        buf[i++] = temp[--temp_len];
    }
    
    return i;
}

void handle_int(Arena *arena, va_list args, t_fmt_opt opt)
{
    s32 val = va_arg(args, s32);
    u8 buffer[32]; 

    s32 len = my_stoa_buf(buffer, val);
    
    String8 s;
    s.str = buffer;
    s.size = len;

    apply_padding(arena, s, opt);
}
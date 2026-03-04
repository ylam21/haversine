u32 my_utoa_buf(u8 *buf, u32 x)
{
    u32 i = 0;
    s64 nb = x;
    
    if (nb == 0)
    {
        buf[0] = '0';
        return 1;
    }

    u8 temp[32];
    u32 temp_len = 0;
    while (nb)
    {
        temp[temp_len++] = (nb % 10) + '0';
        nb /= 10;
    }
    
    while (temp_len > 0)
    {
        buf[i++] = temp[--temp_len];
    }
    
    return i;
}

void handle_uint(Arena *arena, va_list args, t_fmt_opt opt)
{
    u32 val = va_arg(args, u32);
    u8 buffer[32]; 

    u32 len = my_utoa_buf(buffer, val);
    
    String8 s;
    s.str = buffer;
    s.size = len;

    apply_padding(arena, s, opt);
}
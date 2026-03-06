u32 my_u64toa_buf(u8 *buf, u64 x)
{
    u32 i = 0;
    
    if (x == 0)
    {
        buf[0] = '0';
        return 1;
    }

    u8 temp[24]; 
    u32 temp_len = 0;
    
    while (x > 0)
    {
        temp[temp_len++] = (x % 10) + '0';
        x /= 10;
    }
    
    while (temp_len > 0)
    {
        buf[i++] = temp[--temp_len];
    }
    
    return i;
}

void handle_u64(Arena *arena, va_list args, t_fmt_opt *opt)
{
    u64 val = va_arg(args, u64);
    u8 buffer[32];

    u32 len = my_u64toa_buf(buffer, val);
    
    String8 s = { .str = buffer, .size = len };
    apply_padding(arena, s, opt);
}
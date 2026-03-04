void handle_hex_ptr(Arena *arena, va_list args, t_fmt_opt opt, u8 table[16])
{
    void *ptr = va_arg(args, void *);
    u64 val = (u64)ptr;
    
    u8 buffer[32]; 
    u64 len = 0;

    if (ptr == NULL)
    {
        u8 *nil_str = (u8 *)"(nil)";
        while (*nil_str)
        {
            buffer[len++] = *nil_str++;
        }
    }
    else
    {
        buffer[len++] = '0';
        buffer[len++] = 'x';

        u8 temp[20];
        s32 temp_idx = 0;
        
        while (val)
        {
            temp[temp_idx++] = table[val & 0xF];
            val >>= 4;
        }

        while (temp_idx > 0)
        {
            buffer[len++] = temp[--temp_idx];
        }
    }

    String8 s;
    s.str = buffer;
    s.size = len;

    apply_padding(arena, s, opt);
}

void handle_ptr(Arena *arena, va_list args, t_fmt_opt opt)
{
    handle_hex_ptr(arena, args, opt, (u8 *)"0123456789abcdef");
}
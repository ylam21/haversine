void handle_string8(Arena *arena, va_list args, t_fmt_opt opt)
{
    String8 s = va_arg(args, String8);

    if (!s.str)
    {
        s.size = 0;
    }

    u8 buffer[16]; // NOTE: In theory only 3 bytes of memory is needed to allocate 
    if (opt.is_conditional && s.size > 0)
    {
        memcpy(buffer, s.str, s.size);
        buffer[s.size] = ':';

        s.str = buffer;
        s.size += 1;
    }

    apply_padding(arena, s, opt);
}

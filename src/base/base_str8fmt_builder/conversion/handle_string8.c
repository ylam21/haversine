void handle_string8(Arena *arena, va_list args, t_fmt_opt *opt)
{
    String8 s = va_arg(args, String8);

    if (!s.str)
    {
        s.size = 0;
    }

    apply_padding(arena, s, opt);
}

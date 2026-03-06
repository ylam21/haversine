void handle_percent_literal(Arena *arena, va_list args, t_fmt_opt *opt)
{
    (void)args;

    u8 c = '%';
    String8 s;
    s.str = &c;
    s.size = 1;

    apply_padding(arena, s, opt);
}
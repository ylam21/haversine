void handle_char(Arena *arena, va_list args, t_fmt_opt opt)
{
    u8 c = (u8)va_arg(args, s32);

    String8 s;
    s.str = &c;
    s.size = 1;

    apply_padding(arena, s, opt);
}
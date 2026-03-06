void apply_padding(Arena *arena, String8 s, t_fmt_opt *opt)
{
    if (s.size >= (u64)opt->width)
    {
        u8 *result = arena_push_packed(arena, s.size);
        if (!result) return;
        memcpy(result, s.str, s.size);
        return ;
    }

    s32 total_padding = opt->width - s.size;
    
    u8 *padded = arena_push_packed(arena, opt->width); 
    if (!padded) return;

    if (opt->left_align)
    {   
        /* <TEXT;SPACES> */
        memcpy(padded, s.str, s.size);
        memset(padded + s.size, CHAR_SPACE, total_padding);
    }
    else
    {
        /* <SPACES/ZEROS;TEXT> */
        memset(padded, opt->padding_char, total_padding);
        memcpy(padded + total_padding, s.str, s.size);
    }
}
fmt_handler_t specifier_table[256] =
{
    ['d'] = handle_int,
    ['i'] = handle_int,
    ['u'] = handle_uint,
    ['s'] = handle_string8,
    ['c'] = handle_char,
    ['x'] = handle_x,
    ['X'] = handle_upx,
    ['p'] = handle_ptr,
    ['%'] = handle_percent_literal
};
#pragma once
#include "../../../root.unity.h"

u8 my_ftoa_buf(u8 *buffer, f64 val, t_fmt_opt *opt)
{
    u8 i = 0;
    if (val < 0) { buffer[i++] = '-'; val = -val; }

    s32 precision = (opt->precision >= 0) ? opt->precision : 6;

    f64 rounding = 0.5;
    for (s32 p = 0; p < precision; ++p) rounding /= 10.0;
    val += rounding;

    u64 whole = (u64)val;
    f64 fraction = val - (f64)whole;
    i += my_u64toa_buf(buffer + i, whole);

    if (precision > 0) {
        buffer[i++] = '.';
        while (precision--) {
            fraction *= 10.0;
            u32 digit = (u32)fraction;
            buffer[i++] = (u8)digit + '0';
            fraction -= (f64)digit;
        }
    }
    return i;
}

void handle_float(Arena *arena, va_list args, t_fmt_opt *opt)
{
	f64 val = va_arg(args, f64);
	u8 buf[32] = {0};
	u8 len = my_ftoa_buf(buf, val, opt);
	String8 s = (String8){.str = buf, .size = len};
	apply_padding(arena, s, opt);
}

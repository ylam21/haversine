#ifndef CONVERSION_H
#define CONVERSION_H

void apply_padding(Arena *arena, String8 s, t_fmt_opt *opt);
void handle_int(Arena *arena, va_list args, t_fmt_opt *opt);
void handle_u64(Arena *arena, va_list args, t_fmt_opt *opt);
void handle_x(Arena *arena, va_list args, t_fmt_opt *opt);
void handle_hex(Arena *arena, va_list args, t_fmt_opt *opt, u8 table[16]);
void handle_upx(Arena *arena, va_list args, t_fmt_opt *opt);
void handle_ptr(Arena *arena, va_list args, t_fmt_opt *opt);
void handle_float(Arena *arena, va_list args, t_fmt_opt *opt);
void handle_string8(Arena *arena, va_list args, t_fmt_opt *opt);
void handle_char(Arena *arena, va_list args, t_fmt_opt *opt);
void handle_percent_literal(Arena *arena, va_list args, t_fmt_opt *opt);
void handle_hex_ptr(Arena *arena, va_list args, t_fmt_opt *opt, u8 table[16]);
s32 my_stoa_buf(u8 *buf, s32 x);
u32 my_u64toa_buf(u8 *buf, u64 x);
u8 my_ftoa_buf(u8 *buf, f64 val, t_fmt_opt *opt);

#endif
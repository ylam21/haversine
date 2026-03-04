#ifndef CONVERSION_H
#define CONVERSION_H

void apply_padding(Arena *arena, String8 s, t_fmt_opt opt);
void handle_int(Arena *arena, va_list args, t_fmt_opt opt);
void handle_uint(Arena *arena, va_list args, t_fmt_opt opt);
void handle_x(Arena *arena, va_list args, t_fmt_opt opt);
void handle_hex(Arena *arena, va_list args, t_fmt_opt opt, u8 table[16]);
void handle_upx(Arena *arena, va_list args, t_fmt_opt opt);
void handle_ptr(Arena *arena, va_list args, t_fmt_opt opt);
void handle_string8(Arena *arena, va_list args, t_fmt_opt opt);
void handle_char(Arena *arena, va_list args, t_fmt_opt opt);
void handle_percent_literal(Arena *arena, va_list args, t_fmt_opt opt);
void handle_hex_ptr(Arena *arena, va_list args, t_fmt_opt opt, u8 table[16]);
s32 my_stoa_buf(u8 *buf, s32 x);
u32 my_utoa_buf(u8 *buf, u32 x);

#endif
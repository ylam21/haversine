#ifndef PARSER_H
#define PARSER_H

typedef struct f64_array f64_array;
struct f64_array
{
	f64 *arr;
	u64 count;
};

u8 *locate_str8_in_str8(String8 src, String8 target);
u64 parse_point_from_json_to_fd(s32 fd, String8 point, String8 data, f64 *acc);
void json_parse_to_fd(s32 fd, String8 json);
f64_array json_parse_to_buffer(Arena *arena, String8 json);
u64 parse_point_from_json_to_buffer(Arena *arena, f64_array *arr, String8 point, String8 data);

#endif

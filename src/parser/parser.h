#ifndef PARSER_H
#define PARSER_H

u8 *locate_str8_in_str8(String8 src, String8 target);
u64 parse_point_from_json(s32 fd, String8 point, String8 data);
void json_parse(s32 fd, String8 json);

#endif
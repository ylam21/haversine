u8 *locate_str8_in_str8(String8 src, String8 target)
{
	if (target.size == 0)
	{
		return src.str;
	}
	if (target.size > src.size || src.size == 0)
	{
		return 0;
	}
	u64 pos = 0;
	while (pos < src.size)
	{
		if (str8ncmp((String8){.str = src.str + pos, .size = src.size - pos}, target, target.size) == 0)
		{
			return src.str + pos;
		}
		pos += 1;
	}
	return 0;
}

u64 parse_point_from_json(s32 fd, String8 point, String8 data)
{
	u8 *ptr = locate_str8_in_str8(data, point);
	if (ptr)
	{
		u64 offset = point.size + 2;
		if (offset > data.size) return 0;

		char *endptr;
		f64 val = strtod((char *)(ptr + offset), &endptr);
		write(fd, &val, sizeof(f64));

		return (u64)((u8*)endptr - data.str);
	}
	return 0;
}

// This is a simple parser and it is not generic at all.
void json_parse(s32 fd, String8 json)
{
	u64 pos = 0;
	u64 end = json.size;
	u64 consumed = 0;
	while (pos < end)
	{
		consumed = parse_point_from_json(fd, STR8_LIT("x0"), json);
		if (consumed == 0) return;
		json.str += consumed;
		json.size -= consumed;
		consumed = parse_point_from_json(fd, STR8_LIT("y0"), json);
		if (consumed == 0) return;
		json.str += consumed;
		json.size -= consumed;
		consumed = parse_point_from_json(fd, STR8_LIT("x1"), json);
		if (consumed == 0) return;
		json.str += consumed;
		json.size -= consumed;
		consumed = parse_point_from_json(fd, STR8_LIT("y1"), json);
		if (consumed == 0) return;
		json.str += consumed;
		json.size -= consumed;

	}
}
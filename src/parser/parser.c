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

u64 parse_point_from_json_to_buffer(Arena *arena, f64_array *arr, String8 point, String8 data, f64 *acc)
{
    PROFILER_BLOCK_BEGIN(parse_locate_str);
	u8 *ptr = locate_str8_in_str8(data, point);
	PROFILER_BLOCK_END(parse_locate_str);
	if (ptr)
	{
		u64 offset = point.size + 3; // fixed calcution to the float since point is just single char
		if (offset > data.size) return 0;

		PROFILER_BLOCK_BEGIN(parse_strtod_conv);
		char *endptr;
		f64 val = strtod((char *)(ptr + offset), &endptr);
		PROFILER_BLOCK_END(parse_strtod_conv);

		*acc += val;

		PROFILER_BLOCK_BEGIN(parse_arena_push);
		f64 *dest = arena_push_packed(arena, sizeof(f64));
		if (dest)
		{
			*dest = val;
			arr->count += 1;
		}
		else
		{
			fprintf(stderr, "Error: not enough capacity on the arena\n");
		}
		PROFILER_BLOCK_END(parse_arena_push);

		return (u64)((u8*)endptr - data.str);
	}
	return 0;
}

f64_array json_parse_to_buffer(Arena *arena, String8 json)
{
	f64_array arr =
	{
		.arr = arena->buffer + arena->pos,
		.count = 0,
	};


	f64 acc = 0;
	u64 consumed = 0;
	while (json.size > 0)
	{
		consumed = parse_point_from_json_to_buffer(arena, &arr, STR8_LIT("x"), json, &acc);
		if (consumed == 0) break;
		json.str += consumed;
		json.size -= consumed;
		consumed = parse_point_from_json_to_buffer(arena, &arr, STR8_LIT("y"), json, &acc);
		if (consumed == 0) break;
		json.str += consumed;
		json.size -= consumed;
		consumed = parse_point_from_json_to_buffer(arena, &arr, STR8_LIT("x"), json, &acc);
		if (consumed == 0) break;
		json.str += consumed;
		json.size -= consumed;
		consumed = parse_point_from_json_to_buffer(arena, &arr, STR8_LIT("y"), json, &acc);
		if (consumed == 0) break;
		json.str += consumed;
		json.size -= consumed;
	}
	fprintf(stdout, "Accumulator value: %.16f\n", acc);

	return arr;
}

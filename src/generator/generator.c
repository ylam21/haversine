////////////////////////////////////////////////////////////
// COPIED FROM examples
f64 Square(f64 A)
{
    f64 Result = (A*A);
    return Result;
}

f64 RadiansFromDegrees(f64 Degrees)
{
    f64 Result = 0.01745329251994329577 * Degrees;
    return Result;
}

// NOTE(casey): EarthRadius is generally expected to be 6372.8
f64 ReferenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius)
{
    /* NOTE(casey): This is not meant to be a "good" way to calculate the Haversine distance.
       Instead, it attempts to follow, as closely as possible, the formula used in the real-world
       question on which these homework exercises are loosely based.
    */
    
    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;
    
    f64 dLat = RadiansFromDegrees(lat2 - lat1);
    f64 dLon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);
    
    f64 a = Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));
    
    f64 Result = EarthRadius * c;
    
    return Result;
}
// END OF: COPIED FROM examples
////////////////////////////////////////////////////////////

u64 ranval(ranctx *x)
{
	u64 e = x->a - rot(x->b, 7);
	x->a = x->b ^ rot(x->c, 13);
	x->b = x->c + rot(x->d, 37);
	x->c = x->d + e;
	x->d = e + x->a;
	return x->d;
}

void raninit(ranctx *x, u64 seed)
{
	x->a = 0xf1ea5eed;
	x->b = seed;
	x->c = seed;
	x->d = seed;

	u8 i = 0;
	while (i < 20)
	{
		(void)ranval(x);
		i += 1;
	}
}

f64 rand_in_range(ranctx *ctx, f64 min, f64 max)
{
	f64 t = (f64)ranval(ctx) / (f64)UINT64_MAX;
	return (1.0 - t)*min + t*max;
}

void generate_and_write_data(s32 fd, u64 seed, u64 n_pairs, u8 flag)
{
	Arena arena = {0};
	u8 buf[KB(1)];
	arena_init(&arena, buf, KB(1));
	
	ranctx ctx = {0};
	raninit(&ctx, seed);
	
	f64 x0, y0, x1, y1 = 0;
	
	f64 sum = 0;
	f64 sum_coef  = 1.0 / (f64)n_pairs;
	f64 haversine_distance = 0;
	u64 n = 0;
	String8 sep = {0};
	s32 written = str8fmt_write(fd, &arena, STR8_LIT("{\"pairs\":[\n"));
	if (written == -1) return;

	if (flag & typeUniform)
	{
		while (n < n_pairs)
		{
			x0 = rand_in_range(&ctx, -MAX_ALLOWED_X, MAX_ALLOWED_X);
			y0 = rand_in_range(&ctx, -MAX_ALLOWED_Y, MAX_ALLOWED_Y);
			x1 = rand_in_range(&ctx, -MAX_ALLOWED_X, MAX_ALLOWED_X);
			y1 = rand_in_range(&ctx, -MAX_ALLOWED_Y, MAX_ALLOWED_Y);
			
			sep = n == (n_pairs - 1) ? STR8_LIT("\n") : STR8_LIT(",\n");
			written = str8fmt_write(fd, &arena, STR8_LIT("    {\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f}%s"), x0, y0, x1, y1, sep);
			if (written == -1) return;
			arena_pop_to(&arena, 0);

			haversine_distance = ReferenceHaversine(x0, y0, x1, y1, 6372.8);
			sum += sum_coef*haversine_distance;
			n += 1;
		}
	}
	else if (flag & typeCluster)
	{
		f64 x_center = 0;
		f64 y_center = 0;
		f64 x_radius = 0;
		f64 y_radius = 0;

		u64 points_per_cluster = 50;
		u64 current_cluster_points = 0;
		while (n < n_pairs)
		{
			if (current_cluster_points == 0)
			{
				x_center = rand_in_range(&ctx, -MAX_ALLOWED_X, MAX_ALLOWED_X);
				x_radius = rand_in_range(&ctx, 5, MAX_ALLOWED_X / 4);
				y_center = rand_in_range(&ctx, -MAX_ALLOWED_Y, MAX_ALLOWED_Y);
				y_radius = rand_in_range(&ctx, 5, MAX_ALLOWED_Y / 4);
			}
			
			x0 = rand_in_range(&ctx, x_center - x_radius, x_center + x_radius);
			y0 = rand_in_range(&ctx, y_center - y_radius, y_center + y_radius);
			x1 = rand_in_range(&ctx, x_center - x_radius, x_center + x_radius);
			y1 = rand_in_range(&ctx, y_center - y_radius, y_center + y_radius);

			sep = n == (n_pairs - 1) ? STR8_LIT("\n") : STR8_LIT(",\n");
			written = str8fmt_write(fd, &arena, STR8_LIT("    {\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f}%s"), x0, y0, x1, y1, sep);
			if (written == -1) return;
			arena_pop_to(&arena, 0);

			haversine_distance = ReferenceHaversine(x0, y0, x1, y1, 6372.8);
			sum += sum_coef*haversine_distance;
			n += 1;
			current_cluster_points += 1;
			if (current_cluster_points >= points_per_cluster)
			{
				current_cluster_points = 0;
			}
		}
	}
	written = str8fmt_write(fd, &arena, STR8_LIT("]}\n"));
	if (written == -1) return;

	char *method_name = flag == typeUniform ? "uniform" : "cluster";
	fprintf(stdout, "Method: %s\n", method_name);
	fprintf(stdout, "Seed: %lu\n", seed);
	fprintf(stdout, "Pair count: %lu\n", n_pairs);
	fprintf(stdout, "Expected sum: %.16f\n", sum);
}
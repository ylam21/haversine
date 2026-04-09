#ifndef GENERATOR_H
#define GENERATOR_H

#define MAX_ALLOWED_X 180
#define MAX_ALLOWED_Y 90

#define rot(x, k) (((x) << (k)) | ((x) >> (64 - (k))))

enum generatorTypeFlag
{
	typeUniform = 0x1,
	typeCluster = 0x2,
};

typedef struct ranctx
{
	u64 a;
	u64 b;
	u64 c;
	u64 d;
} ranctx;

f64 ReferenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius);
f64 RadiansFromDegrees(f64 Degrees);
f64 Square(f64 A);
void generate_and_write_data(s32 fd, s32 fd_ans, u64 seed, u64 n_pairs, u8 flag);
f64 rand_in_range(ranctx *ctx, f64 min, f64 max);
void raninit(ranctx *x, u64 seed);
u64 ranval(ranctx *x);

#endif

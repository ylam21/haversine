#ifndef BASE_CORE_H
#define BASE_CORE_H

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#define KB(n)  (((U64)(n)) << 10)
#define MB(n)  (((U64)(n)) << 20)
#define GB(n)  (((U64)(n)) << 30)
#define TB(n)  (((U64)(n)) << 40)
#define Thousand(n)   ((n)*1000)
#define Million(n)    ((n)*1000000)
#define Billion(n)    ((n)*1000000000)

#define CHAR_SPACE ' '

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef float f32;
typedef double f64;

#endif
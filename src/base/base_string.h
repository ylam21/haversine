#ifndef BASE_STRING_H
#define BASE_STRING_H

typedef struct String8 String8;
struct String8
{
    u64 size;
    u8 *str;
};

#define STR8_LIT(s) (String8){sizeof(s) - 1, (u8*)(s)}

u32 str8ncmp(String8 s1, String8 s2, u32 n);

#endif
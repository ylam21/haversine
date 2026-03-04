#ifndef BASE_ARENA_H
#define BASE_ARENA_H

#define DEFAULT_ALIGNMENT (u64)sizeof(void *)

typedef struct Arena Arena;
struct Arena
{
    void *buffer;
    u64 cap;
    u64 pos;
};

typedef struct Temp Temp;
struct Temp
{
    Arena *arena;
    u64 pos;
};

Arena *arena_create(u64 size);
void *arena_push(Arena *arena, u64 size);
void *arena_push_packed(Arena *arena, u64 size);
u64 arena_pos(Arena *arena);
void arena_pop_to(Arena *arena, u64 pos);
void arena_destroy(Arena *arena);

Temp temp_begin(Arena *arena);
void temp_end(Temp temp);

#endif

Arena *arena_create(u64 size)
{
    Arena *arena;
    arena = malloc(sizeof(Arena));
    if (arena == 0)
    {
        perror("malloc");
        return 0;
    }
    arena->buffer = malloc(size);
    if (arena->buffer == 0)
    {
        perror("malloc");
        free(arena);
        return 0;
    }
    arena->cap = size;
    arena->pos = 0;
    return arena;
}

u64 arena_pos(Arena *arena)
{
    return arena->pos;
}

static u64 align_forward(u64 ptr, u64 align)
{
    return (ptr + align - 1) & ~(align - 1);
}

void  *arena_push(Arena *arena, u64 size)
{
    u64 current_ptr = (u64)arena->buffer + arena->pos;
    u64 offset = align_forward(current_ptr, DEFAULT_ALIGNMENT);
    offset -= (u64)arena->buffer;

    if (offset + size <= arena->cap)
    {
        void *ptr = (u8*)arena->buffer + offset;
        arena->pos  = offset + size;
        memset(ptr, 0, size);
        return ptr;
    }

    return 0;
}

void *arena_push_packed(Arena *arena, u64 size)
{
    if (arena->pos + size <= arena->cap)
    {
        void *ptr = (u8*)arena->buffer + arena->pos;
        arena->pos += size; 
        memset(ptr, 0, size);
        return ptr;
    }
    return 0;
}

void arena_destroy(Arena *arena)
{
    if (arena)
    {
        free(arena->buffer);
        free(arena);
    }
}

void arena_pop_to(Arena *arena, u64 pos)
{
    arena->pos = pos;
}

Temp temp_begin(Arena *arena)
{
    u64 pos = arena_pos(arena);
    return (Temp){.arena = arena, .pos = pos};
}

void temp_end(Temp temp)
{
    arena_pop_to(temp.arena, temp.pos);
}
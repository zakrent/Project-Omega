#ifndef MEMORY_ARENA_H
#define MEMORY_ARENA_H

typedef struct{
	void *base;
	u64 used;
	u64 usedBySubArenas;
	u64 size;
} MemoryArena;

MemoryArena arena_init(void* base, u64 size);
MemoryArena arena_sub_arena(MemoryArena *arena, u64 size);
void *arena_alloca(MemoryArena *a, u64 size, u8 alignment);
void arena_clear(MemoryArena *a);

#define arena_alloc_type(arena, type) arena_alloca(arena, sizeof(type), __alignof__(type))
#define arena_alloc(arena, size) arena_alloca(arena, size, 1)

#endif

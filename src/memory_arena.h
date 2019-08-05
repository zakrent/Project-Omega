#ifndef MEMORY_ARENA_H
#define MEMORY_ARENA_H

typedef struct{
	void *base;
	u64 used;
	u64 reallyUsed;
	u64 size;
} MemoryArena;

MemoryArena arena_init(void* base, u64 size);
MemoryArena arena_sub_arena(MemoryArena *arena, u64 size);
void *arena_alloc(MemoryArena *a, u64 size);
void arena_clear(MemoryArena *a);

#define arena_alloc_type(arena, type) arena_alloc(arena, sizeof(type))

#endif

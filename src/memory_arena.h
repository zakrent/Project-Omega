#ifndef MEMORY_ARENA_H
#define MEMORY_ARENA_H

typedef struct{
	void *base;
	u64 used;
	u64 size;
} MemoryArena;

MemoryArena arena_init(void* base, u64 size);
MemoryArena arena_sub_arena(MemoryArena *arena, u64 size);
void *arena_push(MemoryArena *arena, u64 size);
void *arena_push_no_align(MemoryArena *arena, u64 size);
void arena_clear(MemoryArena *arena);

#define arena_push_struct(arena, s) arena_push(arena, sizeof(s))
#define arena_push_struct_no_align(arena, s) arena_push_no_align(arena, sizeof(s))

#endif

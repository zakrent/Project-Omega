#include <stdio.h>
#include "memory_arena.h"
#include "common.h"

MemoryArena arena_init(void* base, u64 size){
#ifndef SYSTEM_LAYER
	systemAPI.system_log(LOG_DEBUG, "New memory arena of size %u bytes created", size);
#endif
	MemoryArena a = (MemoryArena){.base = base, .size = size, .used = 0};
	return a;
}

MemoryArena arena_sub_arena(MemoryArena *a, u64 size){
#ifndef SYSTEM_LAYER
	systemAPI.system_log(LOG_DEBUG, "New memory subarena of size %u bytes created", size);
#endif
	void *newBase = arena_alloc(a, size);
	MemoryArena sub = (MemoryArena){.base = newBase, .size = size, .used = 0};
	return sub;
}

void *arena_alloc(MemoryArena *a, u64 size){
	void *firstFreeByte = a->base + a->used;
	a->used += size;
	assert(a->size > a->used);	
	return firstFreeByte;
}

void arena_clear(MemoryArena *a){
	a->used = 0;
}

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
	u64 usedOld = a->used;
	void *newBase = arena_alloc(a, size);
	a->usedBySubArenas += a->used-usedOld;
	MemoryArena sub = (MemoryArena){.base = newBase, .size = size, .used = 0};
	return sub;
}

void *arena_alloca(MemoryArena *a, u64 size, u8 alignment){
	void *firstFreeByte = a->base + a->used;
	void *firstAlignedByte;
	if((u64)firstFreeByte % alignment == 0){
		firstAlignedByte = firstFreeByte;
	}
	else{
		firstAlignedByte = firstFreeByte - (u64)firstFreeByte % alignment + alignment;
	}
	a->used += size+(firstAlignedByte-firstFreeByte);
	assert(a->size > a->used);	
	return firstAlignedByte;
}

void arena_clear(MemoryArena *a){
	a->used = 0;
}

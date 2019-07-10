#include <stdio.h>
#include "memory_arena.h"
#include "common.h"

MemoryArena arena_init(void* base, u64 size){
	MemoryArena a = (MemoryArena){.base = base, .size = size, .used = 0};
	return a;
}

MemoryArena arena_sub_arena(MemoryArena *a, u64 size){
	void *newBase = arena_push(a, size);
	MemoryArena sub = (MemoryArena){.base = newBase, .size = size, .used = 0};
	return sub;
}

void *arena_push(MemoryArena *a, u64 size){
	void *firstFreeByte = a->base + a->used;
	//Align byte to 32 bit boundry
	void *alignedByte = firstFreeByte;
	if((u64)alignedByte % 4 != 0){
		alignedByte += 4-((u64)alignedByte % 4);
	}
	u64 realSize = size+alignedByte-firstFreeByte;
	a->used += realSize;
	assert(a->size > a->used);	
	return alignedByte;
}

void *arena_push_no_align(MemoryArena *a, u64 size){
	void *firstFreeByte = a->base + a->used;
	a->used += size;
	assert(a->size > a->used);	
	return firstFreeByte;
}

void arena_clear(MemoryArena *a){
	a->used = 0;
}

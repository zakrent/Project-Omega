#include <stdint.h>
#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"
#include "common.h"
#include "system.h"

System systemAPI;

#include "memory_arena.c"
#include "resources.c"
#include "game.h"
#include "render_list.h"

typedef struct{
	b32 initialized;
	MemoryArena masterArena;
	MemoryArena transientArena;
	MemoryArena renderList;
} GameState;


FRAME(frame){
	GameState *gs = memory.permanentMemory;
	systemAPI = _systemAPI;
	*renderList = &(gs->renderList);
	if(!gs->initialized){
		gs->masterArena    = arena_init(memory.permanentMemory+sizeof(GameState), memory.permanentMemorySize);
		gs->transientArena = arena_init(memory.transientMemory, memory.transientMemorySize);
		gs->renderList     = arena_sub_arena(&(gs->transientArena), MEGABYTES(8));
		gs->initialized = true;
		systemAPI.system_log(LOG_DEBUG, "GameState initialized");
	}

	File file = systemAPI.system_open_file("./test");
	systemAPI.system_close_file(file);

	arena_clear(&(gs->renderList));
	*(u32*)(arena_push(&(gs->renderList), sizeof(u32)))                   = RL_COLOR_CLEAR;
	*(u32*)(arena_push(&(gs->renderList), sizeof(u32)))                   = RL_DRAW_SPRITE;
	*(RLDrawSprite*)(arena_push(&(gs->renderList), sizeof(RLDrawSprite))) = (RLDrawSprite){.pos = HMM_Vec2(0.0, 0.0)};

	ResourceStatus resStatus = (ResourceStatus){0};
	resources_get_sprite_sheet(SS_BASIC, resStatus, gs->renderList);
}

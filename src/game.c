#include <stdint.h>
#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"
#include "common.h"
#include "system.h"

System systemAPI;

#include "memory_arena.c"
#include "resources.c"
#include "game.h"
#include "render_list.c"

typedef struct{
	b32 initialized;
	ResourceStatus resStatus;
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
		gs->resStatus      = (ResourceStatus){0};
		gs->initialized = true;
		systemAPI.system_log(LOG_DEBUG, "GameState initialized");
	}

	SpriteSheet basicSheet = resources_get_sprite_sheet(SS_BASIC, &(gs->resStatus), gs->renderList);

	arena_clear(&(gs->renderList));
	rl_color_clear(&(gs->renderList));
	rl_set_camera(&(gs->renderList), HMM_Vec2(0.0, 0.0), HMM_Vec2(5.0, 5.0));
	rl_use_texture(&(gs->renderList), basicSheet);
	for(int y = -1; y <= 1; y++){
		for(int x = -1; x <= 1; x++){
			rl_draw_sprite(&(gs->renderList), HMM_Vec2(x, y), HMM_Vec2(1.0, 1.0), HMM_Vec2(x+1, y+1), HMM_Vec2(1.0, 1.0));
		}
	}
}

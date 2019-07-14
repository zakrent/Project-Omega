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
	LIST_PUSH(&(gs->renderList), u32, RL_COLOR_CLEAR);
	LIST_PUSH(&(gs->renderList), u32, RL_SET_CAMERA);
	LIST_PUSH(&(gs->renderList), RLSetCamera, ((RLSetCamera){.pos = HMM_Vec2(0.0, 0.0), .size = HMM_Vec2(5.0, 5.0)}));
	LIST_PUSH(&(gs->renderList), u32, RL_USE_TEXTURE);
	LIST_PUSH(&(gs->renderList), RLUseTexture, ((RLUseTexture){.handle = basicSheet.handle, .xMul = basicSheet.xMul, .yMul = basicSheet.yMul}));
	for(int y = -1; y <= 1; y++){
		for(int x = -1; x <= 1; x++){
			LIST_PUSH(&(gs->renderList), u32, RL_DRAW_SPRITE);
			LIST_PUSH(&(gs->renderList), RLDrawSprite, ((RLDrawSprite){.pos = HMM_Vec2(x, y),
				.size = HMM_Vec2(1.0, 1.0), .spritePos = HMM_Vec2(1.0, 1.0), .spriteSize = HMM_Vec2(1.0, 1.0)}));
		}
	}
}

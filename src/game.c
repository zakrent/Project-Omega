#include <stdint.h>
#include <stdarg.h>
//TODO: remove this:
#include <string.h>
#include <x86intrin.h>
#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"
#include "common.c"
#include "system.h"

System systemAPI;
struct DebugContext;
struct DebugContext *debugCtx;

#include "memory_arena.c"
#include "resources.c"
#include "render_list.c"
#include "ui.c"
#include "debug.c"
#include "game.h"
#include "map.c"
#include "entity.h"
#include "entity.c"

typedef struct{
	b32 initialized;
	EntitiesData *entities;
	Map *map;
	Resources *resources;
	UIContext *uiCtx;
	DebugContext *debugCtx;
	MemoryArena masterArena;
	MemoryArena transientArena;
	MemoryArena frameArena;
} GameState;


FRAME(frame){
	GameState *gs = memory.permanentMemory;
	systemAPI = _systemAPI;
	debugCtx = gs->debugCtx;
	DEBUG_TIMER_START();

	if(!gs->initialized){
		gs->masterArena    = arena_init(memory.permanentMemory+sizeof(GameState), memory.permanentMemorySize-sizeof(GameState));
		gs->transientArena = arena_init(memory.transientMemory, memory.transientMemorySize);
		gs->frameArena     = arena_sub_arena(&gs->transientArena, MEGABYTES(8));

		gs->map = arena_alloc_type(&gs->masterArena, Map);
		map_generate(gs->map, 0);

		gs->entities = arena_alloc_type(&gs->masterArena, EntitiesData);

		gs->resources = arena_alloc_type(&gs->transientArena, Resources);

		gs->debugCtx = arena_alloc_type(&gs->transientArena, DebugContext);

		gs->uiCtx = arena_alloc_type(&gs->masterArena, UIContext);

		gs->initialized = true;
		systemAPI.system_log(LOG_DEBUG, "GameState initialized");
	}


	static u64 counter = 0;
	counter++;
	if(counter % 120 == 0 || counter == 0){
		entity_spawn(gs->entities, gs->map);
	}

	arena_clear(&gs->frameArena);

	entity_update(gs->entities, gs->map);

	SpriteSheet basicSheet = resources_get_sprite_sheet(SS_BASIC, gs->resources);
	SpriteSheet fontSheet  = resources_get_sprite_sheet(SS_FONT,  gs->resources);

	RenderList *list = arena_alloc_type(&gs->frameArena, RenderList);
	*list = (RenderList){0};

	rl_color_clear(&gs->frameArena, list);

	rl_set_camera( &gs->frameArena, list, HMM_Vec2(0.0, 0.0), HMM_Vec2(16.0, 16.0));
	rl_use_texture(&gs->frameArena, list, basicSheet);
	map_draw(gs->map, &gs->frameArena, list);
	entity_draw(gs->entities, &gs->frameArena, list);
	
	rl_set_camera( &gs->frameArena, list, HMM_Vec2(0.0, 0.0), HMM_Vec2(1.0, 1.0));
	rl_use_texture(&gs->frameArena, list, fontSheet);

	static r32 lastTime;
	static r32 highestTime;
	r32 currentTime = systemAPI.time-lastTime;
	if(currentTime > highestTime && counter > 2)
		highestTime = currentTime;
	lastTime = systemAPI.time;

	ui_move(gs->uiCtx, -1.75, -0.96);
	ui_draw_string(gs->uiCtx, &gs->frameArena, list, 4, "Frame time               %10.2f ms", currentTime*1000.0);
	ui_draw_string(gs->uiCtx, &gs->frameArena, list, 4, "Highest frame time       %10.2f ms", highestTime*1000.0);
	ui_draw_string(gs->uiCtx, &gs->frameArena, list, 4, "Master arena             %10u bytes used", gs->masterArena.reallyUsed);
	ui_draw_string(gs->uiCtx, &gs->frameArena, list, 4, "Transient arena          %10u bytes used", gs->transientArena.reallyUsed);
	ui_draw_string(gs->uiCtx, &gs->frameArena, list, 4, "Frame arena              %10u bytes used", gs->frameArena.reallyUsed);
	debug_draw(gs->debugCtx, gs->uiCtx, list, &gs->frameArena);

	*_debugCtx = gs->debugCtx;
	*_renderList = list;
	DEBUG_TIMER_STOP();
}

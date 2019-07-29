#include <stdint.h>
#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"
#include "common.c"
#include "system.h"

System systemAPI;

#include "memory_arena.c"
#include "resources.c"
#include "game.h"
#include "render_list.c"
#include "entity.c"
#include "map.c"

typedef struct{
	b32 initialized;
	EntitiesData entities;
	Map map;
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

		entity_spawn_prefab(&(gs->entities), EPI_TANK,   HMM_Vec2(0.0, 0.0), 0.0);
		entity_spawn_prefab(&(gs->entities), EPI_TURRET, HMM_Vec2(0.0, 0.0), 0.0);
		entity_spawn_prefab(&(gs->entities), EPI_TANK, HMM_Vec2(-6.0, -1.0), 0.0);
		entity_spawn_prefab(&(gs->entities), EPI_PROJECTILE, HMM_Vec2(1.0, 1.0), 1.0);

		map_generate(&(gs->map), 0);

		gs->initialized = true;
		systemAPI.system_log(LOG_DEBUG, "GameState initialized");
	}

	static u64 counter = 0;
	counter++;
	if(counter % 60 == 0){
		map_generate(&(gs->map), 0);
	}

	entity_update(&(gs->entities));

	SpriteSheet basicSheet = resources_get_sprite_sheet(SS_BASIC, &(gs->resStatus), gs->renderList);

	arena_clear(&(gs->renderList));
	rl_color_clear(&(gs->renderList));
	rl_set_camera(&(gs->renderList), HMM_Vec2(0.0, 0.0), HMM_Vec2(16.0, 16.0));
	rl_use_texture(&(gs->renderList), basicSheet);
	map_draw(&(gs->map), &(gs->renderList));
	entity_draw(&(gs->entities), &(gs->renderList));
}

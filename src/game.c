#include <stdint.h>
#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"
#include "common.c"
#include "system.h"

System systemAPI;

#include "memory_arena.c"
#include "resources.c"
#include "render_list.c"
#include "game.h"
#include "entity.c"
#include "map.c"

typedef struct{
	b32 initialized;
	EntitiesData *entities;
	Map *map;
	Resources *resources;
	MemoryArena masterArena;
	MemoryArena transientArena;
	MemoryArena frameArena;
} GameState;

FRAME(frame){
	GameState *gs = memory.permanentMemory;
	systemAPI = _systemAPI;

	if(!gs->initialized){
		gs->masterArena    = arena_init(memory.permanentMemory+sizeof(GameState), memory.permanentMemorySize-sizeof(GameState));
		gs->transientArena = arena_init(memory.transientMemory, memory.transientMemorySize);
		gs->frameArena     = arena_sub_arena(&gs->transientArena, MEGABYTES(8));

		gs->map = arena_alloc_type(&gs->masterArena, Map);
		map_generate(gs->map, 0);

		gs->entities = arena_alloc_type(&gs->masterArena, Map);

		entity_spawn_prefab(gs->entities, EPI_TANK,       HMM_Vec2(0.0, 0.0),   0.0);
		entity_spawn_prefab(gs->entities, EPI_TURRET,     HMM_Vec2(0.0, 0.0),   0.0);
		entity_spawn_prefab(gs->entities, EPI_TANK,       HMM_Vec2(-6.0, -1.0), 0.0);
		entity_spawn_prefab(gs->entities, EPI_PROJECTILE, HMM_Vec2(1.0, 1.0),   1.0);

		gs->resources = arena_alloc_type(&gs->transientArena, Resources);

		gs->initialized = true;
		systemAPI.system_log(LOG_DEBUG, "GameState initialized");
	}

	arena_clear(&gs->frameArena);

	static u64 counter = 0;
	counter++;
	if(counter % 60 == 0){
		map_generate(gs->map, 0);
	}

	entity_update(gs->entities);

	SpriteSheet basicSheet = resources_get_sprite_sheet(SS_BASIC, gs->resources);

	RenderList *list = arena_alloc_type(&gs->frameArena, RenderList);
	*list = (RenderList){0};

	rl_color_clear(&gs->frameArena, list);
	rl_set_camera( &gs->frameArena, list, HMM_Vec2(0.0, 0.0), HMM_Vec2(16.0, 16.0));
	rl_use_texture(&gs->frameArena, list, basicSheet);
	map_draw(gs->map, &gs->frameArena, list);
	entity_draw(gs->entities, &gs->frameArena, list);

	*_renderList = list;
}

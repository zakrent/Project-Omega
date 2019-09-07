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

enum GameMode{
	MODE_MENU,
	MODE_GAME,
	COUNT_MODE
};

typedef struct{
	b32 initialized;
	u32 mode;
	EntitiesData *entities;
	Map *map;
	Resources *resources;
	UIContext *uiCtx;
	DebugContext *debugCtx;
	MemoryArena masterArena;
	MemoryArena modeArena;
	MemoryArena transientArena;
	MemoryArena frameArena;
} GameState;

void game_start_game_mode(GameState *gs){
	arena_clear(&gs->modeArena);
	gs->map = arena_alloc_type(&gs->modeArena, Map);
	map_generate(gs->map, 0);

	gs->entities = arena_alloc_type(&gs->modeArena, EntitiesData);
	entity_populate_prefabs(gs->entities);

	gs->mode = MODE_GAME;
}

FRAME(frame){
	GameState *gs = memory.permanentMemory;
	systemAPI = _systemAPI;
	debugCtx = gs->debugCtx;
	DEBUG_TIMER_START();

	if(!gs->initialized){
		gs->masterArena    = arena_init(memory.permanentMemory+sizeof(GameState), memory.permanentMemorySize-sizeof(GameState));
		gs->modeArena      = arena_sub_arena(&gs->masterArena, 0.8*gs->masterArena.size);
		gs->transientArena = arena_init(memory.transientMemory, memory.transientMemorySize);
		gs->frameArena     = arena_sub_arena(&gs->transientArena, MEGABYTES(8));

		gs->resources = arena_alloc_type(&gs->transientArena, Resources);

		gs->debugCtx = arena_alloc_type(&gs->transientArena, DebugContext);

		gs->uiCtx = arena_alloc_type(&gs->masterArena, UIContext);

		gs->initialized = true;
		systemAPI.system_log(LOG_DEBUG, "GameState initialized");
	}

	arena_clear(&gs->frameArena);

	SoundBuffer *soundBuffer = systemAPI.soundBuffer;
	for(int i = 0; i < 2048; i++){
		soundBuffer->data[soundBuffer->writePosition] = 0.0;
		soundBuffer->writePosition++;
		if(soundBuffer->writePosition >= SOUND_BUFFER_SIZE){
			soundBuffer->writePosition = 0;
		}
		soundBuffer->soundTime += 1.0/soundBuffer->sampleRate;
		u32 writeDistance = 0;
		if(soundBuffer->writePosition > soundBuffer->readPosition){
			writeDistance = soundBuffer->writePosition - soundBuffer->readPosition;
		}
		else{
			writeDistance = (SOUND_BUFFER_SIZE - soundBuffer->readPosition) + soundBuffer->writePosition;
		}
		//TODO: fix sound system bugs
		//assert(soundBuffer->readPosition != soundBuffer->writePosition);
		if(writeDistance > 2048)
			break;
	}
	if(!soundBuffer->soundStarted){
		soundBuffer->soundStarted = true;
	}

	RenderList *list = arena_alloc_type(&gs->frameArena, RenderList);
	*list = (RenderList){0};
	rl_color_clear(&gs->frameArena, list);

	Shader spriteShader = resources_get_shader(SHADER_SPRITE, gs->resources);
	rl_use_shader(&gs->frameArena, list, spriteShader);

	static u64 counter = 0;
	counter++;

	switch(gs->mode){
		case MODE_MENU:
			{
				SpriteSheet basicSheet = resources_get_sprite_sheet(SS_UNITS, gs->resources);
				rl_use_texture(&gs->frameArena, list, basicSheet);
				rl_draw_simple_sprite(&gs->frameArena, list, HMM_Vec2(randf(-0.0, 0.0), randf(-0.0, 0.0)), HMM_Vec2(1.3, 0.6), HMM_Vec2(0.0, 17.0), HMM_Vec2(13.0,6.0));
#if 1
				SpriteSheet fontSheet  = resources_get_sprite_sheet(SS_FONT,  gs->resources);
				rl_use_texture(&gs->frameArena, list, fontSheet);
				ui_move(gs->uiCtx, -1.0, 0.0);
				ui_draw_string(gs->uiCtx, &gs->frameArena, list, 16, "STARTING GAME IN: %f", 2-counter/60.0);
				if((i32)(counter/60.0) == 2 || input.LMBDown)
					game_start_game_mode(gs);
#endif

			}
			break;
		case MODE_GAME:
			{
				if(counter % 240 == 0 || counter == 1)
					entity_spawn(gs->entities, EP_TANK, gs->map->waypoints[0]);

				if(input.LMBChanged && !input.LMBDown){
					r32 posX = input.mouseX*9.0;
					posX += 0.5*posX/HMM_ABS(posX);
					posX = (i32)posX;
					r32 posY = input.mouseY*9.0;
					posY += 0.5*posY/HMM_ABS(posY);
					posY = (i32)posY;
					entity_spawn(gs->entities, EP_TURRET, HMM_Vec2(posX, posY));
				}

				entity_update(gs->entities, gs->map);

				SpriteSheet basicSheet = resources_get_sprite_sheet(SS_BASIC, gs->resources);
				SpriteSheet unitsSheet = resources_get_sprite_sheet(SS_UNITS, gs->resources);

				rl_set_camera( &gs->frameArena, list, HMM_Vec2(0.0, 0.0), HMM_Vec2(9.0, 9.0));

				rl_use_texture(&gs->frameArena, list, basicSheet);
				map_draw(gs->map, &gs->frameArena, list);

				rl_use_texture(&gs->frameArena, list, unitsSheet);
				entity_draw(gs->entities, &gs->frameArena, list);
				
				rl_set_camera( &gs->frameArena, list, HMM_Vec2(0.0, 0.0), HMM_Vec2(1.0, 1.0));

				rl_use_texture(&gs->frameArena, list, basicSheet);
				rl_draw_simple_sprite(&gs->frameArena, list, HMM_Vec2(input.mouseX, input.mouseY), HMM_MultiplyVec2f(HMM_Vec2(0.1, 0.1), 1.0+input.LMBDown), HMM_Vec2(22.0, -0.01), HMM_Vec2(1.0,1.0));

				break;
			}
	}

	SpriteSheet fontSheet  = resources_get_sprite_sheet(SS_FONT,  gs->resources);
	rl_use_texture(&gs->frameArena, list, fontSheet);

	static r32 lastTime;
	static r32 highestTime;
	r32 currentTime = systemAPI.time-lastTime;
	if(currentTime > highestTime)
		highestTime = currentTime;
	lastTime = systemAPI.time;

	ui_move(gs->uiCtx, -1.75, -0.96);
	ui_draw_string(gs->uiCtx, &gs->frameArena, list, 4, "Frame time               %10.2f ms", currentTime*1000.0);
	ui_draw_string(gs->uiCtx, &gs->frameArena, list, 4, "Highest frame time       %10.2f ms", highestTime*1000.0);
	ui_draw_string(gs->uiCtx, &gs->frameArena, list, 4, "Master arena             %10u bytes used", gs->masterArena.used-gs->masterArena.usedBySubArenas);
	ui_draw_string(gs->uiCtx, &gs->frameArena, list, 4, "Mode arena               %10u bytes used", gs->modeArena.used-gs->modeArena.usedBySubArenas);
	ui_draw_string(gs->uiCtx, &gs->frameArena, list, 4, "Transient arena          %10u bytes used", gs->transientArena.used-gs->transientArena.usedBySubArenas);
	ui_draw_string(gs->uiCtx, &gs->frameArena, list, 4, "Frame arena              %10u bytes used", gs->frameArena.used-gs->frameArena.usedBySubArenas);
	ui_draw_string(gs->uiCtx, &gs->frameArena, list, 4, "Mouse X                  %10.2f", input.mouseX);
	ui_draw_string(gs->uiCtx, &gs->frameArena, list, 4, "Mouse Y                  %10.2f", input.mouseY);
	debug_draw(gs->debugCtx, gs->uiCtx, list, &gs->frameArena);

	*_debugCtx = gs->debugCtx;
	*_renderList = list;
	DEBUG_TIMER_STOP();
}

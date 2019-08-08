#include "render_list.h"

RLEntryHeader *rl_new_entry(MemoryArena *frameArena, RenderList *list, u32 type){
	RLEntryHeader *newLast = arena_alloc_type(frameArena, RLEntryHeader);
	*newLast = (RLEntryHeader){.type = type, .data = NULL, .nextEntry = NULL};
	if(list->last){
		list->last->nextEntry = newLast;
		list->last = newLast;
	}
	else{
		list->first = newLast;
		list->last = newLast;
	}
	return newLast;
}

void rl_color_clear(MemoryArena* frameArena, RenderList *list){
	rl_new_entry(frameArena, list, RL_COLOR_CLEAR);
}

void rl_use_texture(MemoryArena* frameArena, RenderList *list, SpriteSheet sheet){
	RLEntryHeader *header = rl_new_entry(frameArena, list, RL_USE_TEXTURE);
	RLUseTexture *data = arena_alloc_type(frameArena, RLUseTexture);
	*data = (RLUseTexture){.handle = sheet.handle, .xMul = sheet.xMul, .yMul = sheet.yMul};
	header->data = (void *)data;
}

void rl_set_camera(MemoryArena* frameArena, RenderList *list, hmm_vec2 pos, hmm_vec2 size){
	RLEntryHeader *header = rl_new_entry(frameArena, list, RL_SET_CAMERA);
	RLSetCamera *data = arena_alloc_type(frameArena, RLSetCamera);
	*data = (RLSetCamera){.pos = pos, .size = size};
	header->data = (void *)data;
}

//Optimization breaks this code
#pragma GCC push_options
#pragma GCC optimize ("O0")
void rl_draw_sprite(MemoryArena* frameArena, RenderList *list, hmm_vec2 pos, r32 rotation, hmm_vec2 rotationOffset, hmm_vec2 size, hmm_vec2 spritePos, hmm_vec2 spriteSize){
	RLEntryHeader *header = rl_new_entry(frameArena, list, RL_DRAW_SPRITE);
	RLDrawSprite *data = arena_alloc_type(frameArena, RLDrawSprite);

	hmm_m4 m = HMM_MultiplyMat4( HMM_Translate(HMM_Vec3(pos.X, pos.Y, 0.0)),
			   HMM_MultiplyMat4( HMM_Rotate(rotation, HMM_Vec3(0.0,0.0,1.0)),
			   HMM_MultiplyMat4( HMM_Translate(HMM_Vec3(rotationOffset.X, rotationOffset.Y, 0.0)),
								 HMM_Scale(HMM_Vec3(size.Width*0.5, size.Height*0.5, 1.0)))));

	*data = (RLDrawSprite){.model = m, .spritePos = spritePos, .spriteSize = spriteSize};
	header->data = (void *)data;
}

void rl_draw_simple_sprite(MemoryArena* frameArena, RenderList *list, hmm_vec2 pos, hmm_vec2 size, hmm_vec2 spritePos, hmm_vec2 spriteSize){
	RLEntryHeader *header = rl_new_entry(frameArena, list, RL_DRAW_SPRITE);
	RLDrawSprite *data = arena_alloc_type(frameArena, RLDrawSprite);

	hmm_m4 m = HMM_MultiplyMat4( HMM_Translate(HMM_Vec3(pos.X, pos.Y, 0.0)), 
								 HMM_Scale(HMM_Vec3(size.Width*0.5, size.Height*0.5, 1.0)));

	*data = (RLDrawSprite){.model = m, .spritePos = spritePos, .spriteSize = spriteSize};
	header->data = (void *)data;
}

void rl_draw_cached_simple_sprite(MemoryArena* frameArena, RenderList *list, RLDrawSprite *data){
	RLEntryHeader *header = rl_new_entry(frameArena, list, RL_DRAW_SPRITE);
	header->data = (void *)data;
}

void rl_cache_simple_sprite(RLDrawSprite *data, hmm_vec2 pos, hmm_vec2 size, hmm_vec2 spritePos, hmm_vec2 spriteSize){
	hmm_m4 m = HMM_MultiplyMat4( HMM_Translate(HMM_Vec3(pos.X, pos.Y, 0.0)), 
								 HMM_Scale(HMM_Vec3(size.Width*0.5, size.Height*0.5, 1.0)));
	*data = (RLDrawSprite){.model = m, .spritePos = spritePos, .spriteSize = spriteSize};
}
#pragma GCC pop_options

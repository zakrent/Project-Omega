#include "render_list.h"

void rl_color_clear(MemoryArena* list){
	LIST_PUSH(list, u32, RL_COLOR_CLEAR);
}

void rl_use_texture(MemoryArena* list, SpriteSheet sheet){
	LIST_PUSH(list, u32, RL_USE_TEXTURE);
	LIST_PUSH(list, RLUseTexture, ((RLUseTexture){.handle = sheet.handle, .xMul = sheet.xMul, .yMul = sheet.yMul}));
}

void rl_set_camera(MemoryArena* list, hmm_vec2 pos, hmm_vec2 size){
	LIST_PUSH(list, u32, RL_SET_CAMERA);
	LIST_PUSH(list, RLSetCamera, ((RLSetCamera){.pos = pos, .size = size}));
}

void rl_draw_sprite(MemoryArena* list, hmm_vec2 pos, r32 rotation, hmm_vec2 rotationOffset, hmm_vec2 size, hmm_vec2 spritePos, hmm_vec2 spriteSize){
	LIST_PUSH(list, u32, RL_DRAW_SPRITE);
	LIST_PUSH(list, RLDrawSprite, ((RLDrawSprite){.pos = pos, .rotation = rotation, .rotationOffset = rotationOffset,
		.size = size, .spritePos = spritePos, .spriteSize = spriteSize}));
}

#ifndef Z_RENDER_LIST_H
#define Z_RENDER_LIST_H

#define LIST_PUSH(list, type, var) *(type*)(arena_push_no_align(list, sizeof(type))) = var;

enum RLEntryType{
	RL_INVALID,

	RL_COLOR_CLEAR,

	RL_USE_TEXTURE,
//	RL_USE_SHADER,

	RL_SET_CAMERA,

	RL_DRAW_SPRITE,

	COUNT_RL_TYPE
};

typedef struct{
	u32 handle;
	r32 xMul;
	r32 yMul;
}RLUseTexture;

typedef struct{
	hmm_vec2 pos;
	hmm_vec2 size;
}RLSetCamera;

typedef struct{
	hmm_vec2 pos;
	r32 rotation;
	hmm_vec2 rotationOffset;
	hmm_vec2 size;
	hmm_vec2 spritePos;
	hmm_vec2 spriteSize;
}RLDrawSprite;

#ifndef SYSTEM_LAYER

void rl_color_clear(MemoryArena* list);
void rl_use_texture(MemoryArena* list, SpriteSheet sheet);
void rl_set_camera(MemoryArena* list, hmm_vec2 pos, hmm_vec2 size);
void rl_draw_sprite(MemoryArena* list, hmm_vec2 pos, r32 rotation, hmm_vec2 rotationOffset, hmm_vec2 size, hmm_vec2 spritePos, hmm_vec2 spriteSize);

#endif

#endif

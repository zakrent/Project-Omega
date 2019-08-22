#ifndef Z_RENDER_LIST_H
#define Z_RENDER_LIST_H

enum RLEntryType{
	RL_INVALID,

	RL_COLOR_CLEAR,

	RL_USE_TEXTURE,

	RL_SET_CAMERA,

	RL_DRAW_SPRITE,

	COUNT_RL_TYPE
};

struct RLEntryHeader;
typedef struct RLEntryHeader RLEntryHeader;

struct RLEntryHeader{
	u32 type;
	RLEntryHeader *nextEntry;
	void *data;
};

typedef struct{
	RLEntryHeader *first;
	RLEntryHeader *last;
}RenderList;

typedef struct{
	u32 handle;
	r32 xMul;
	r32 yMul;
	r32 xOffset;
	r32 yOffset;
}RLUseTexture;

typedef struct{
	hmm_vec2 pos;
	hmm_vec2 size;
}RLSetCamera;

typedef struct{
	hmm_v2 spritePos;
	hmm_v2 spriteSize;
	hmm_m4 model;
}RLDrawSprite;

#ifndef SYSTEM_LAYER

void rl_color_clear(MemoryArena* frameArena, RenderList *list);
void rl_use_texture(MemoryArena* frameArena, RenderList *list, SpriteSheet sheet);
void rl_set_camera(MemoryArena*  frameArena, RenderList *list, hmm_vec2 pos, hmm_vec2 size);
void rl_draw_sprite(MemoryArena* frameArena, RenderList *list, hmm_vec2 pos, r32 rotation, hmm_vec2 rotationOffset, hmm_vec2 size, hmm_vec2 spritePos, hmm_vec2 spriteSize);
void rl_draw_simple_sprite(MemoryArena* frameArena, RenderList *list, hmm_vec2 pos, hmm_vec2 size, hmm_vec2 spritePos, hmm_vec2 spriteSize);
void rl_draw_cached_simple_sprite(MemoryArena* frameArena, RenderList *list, RLDrawSprite *data);
void rl_cache_simple_sprite(RLDrawSprite *data, hmm_vec2 pos, hmm_vec2 size, hmm_vec2 spritePos, hmm_vec2 spriteSize);

#endif

#endif

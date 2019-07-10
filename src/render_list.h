#ifndef Z_RENDER_LIST_H
#define Z_RENDER_LIST_H

enum RLEntryType{
	RL_INVALID,

	RL_COLOR_CLEAR,

	RL_USE_TEXTURE,
	RL_USE_SHADER,

	RL_SET_SHADER,

	RL_DRAW_SPRITE,

	COUNT_RL_TYPE
};

typedef struct{
	hmm_vec2 pos;
}RLDrawSprite;

#endif

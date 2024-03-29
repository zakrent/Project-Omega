#include "ui.h"

void ui_move(UIContext *ctx, r32 x, r32 y){
	ctx->x = x;
	ctx->y = y;
}

void ui_draw_string(UIContext *ctx, MemoryArena *frameArena, RenderList *list, r32 size, const char *fmt, ...){
	char *s = arena_alloc(frameArena, 128);
	va_list ap;
	va_start(ap, fmt);
	vsprintf(s, fmt, ap);
	va_end(ap);
	r32 sizeY = size/100.0;
	r32 sizeX = sizeY*0.5;
	r32 xOffset = 0.0;
	for(char *c = s; *c != 0; c++){
		u32 sx = (*c - 32) % 32;
		u32 sy = (*c - 32 - sx)/32.0;
		rl_draw_simple_sprite(frameArena, list, HMM_Vec2(ctx->x+xOffset, ctx->y), HMM_Vec2(sizeX, sizeY), HMM_Vec2(sx, sy), HMM_Vec2(1.0,1.0));
		xOffset += sizeX;
	}
	ctx->y += sizeY;
}

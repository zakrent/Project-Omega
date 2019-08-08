#ifndef UI_H
#define UI_H

//Very naive ui implementation, for now
//only used for debug
typedef struct{
	r32 x;
	r32 y;
} UIContext;

void ui_move(UIContext *ctx, r32 x, r32 y);
void ui_draw_string(UIContext *ctx, MemoryArena *frameArena, RenderList *list, r32 size, const char *fmt, ...);

#endif

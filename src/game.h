#ifndef Z_GAME_H
#define Z_GAME_H

typedef struct{
	r64 mouseX;
	r64 mouseY;
	b32 LMBDown;
	b32 LMBChanged;
} GameInput;

typedef struct{
	void *permanentMemory;
	void *transientMemory;
	u64 permanentMemorySize;
	u64 transientMemorySize;
} GameMemory;

#define FRAME(name) void name(GameMemory memory, GameInput input, System _systemAPI, RenderList **_renderList, DebugContext **_debugCtx)
typedef FRAME(Frame);

#endif

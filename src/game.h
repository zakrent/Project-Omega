#ifndef Z_GAME_H
#define Z_GAME_H

typedef struct{
	void *permanentMemory;
	void *transientMemory;
	u64 permanentMemorySize;
	u64 transientMemorySize;
} GameMemory;

#define FRAME(name) void name(GameMemory memory, System _systemAPI, RenderList **_renderList, DebugContext **_debugCtx)
typedef FRAME(Frame);

#endif

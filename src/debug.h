#ifndef DEBUG_H
#define DEBUG_H

#define MAX_DEBUG_TIMERS 16

//Only use one timer per block
#ifndef SYSTEM_LAYER
#define DEBUG_TIMER_START() u8 dTimerIdx = debug_timer_start(__COUNTER__, debugCtx, (char*)__PRETTY_FUNCTION__)
#define DEBUG_TIMER_STOP() debug_timer_stop(debugCtx, dTimerIdx)
#else
#define DEBUG_TIMER_START() u8 dTimerIdx = debug_timer_start(__COUNTER__+MAX_DEBUG_TIMERS, debugCtx, (char*)__PRETTY_FUNCTION__)
#define DEBUG_TIMER_STOP() debug_timer_stop(debugCtx, dTimerIdx)
#endif

typedef struct{
	u64 id;
	u64 start;
	u64 value;
	u64 avg;
	char name[64];
} DebugTimer; 

typedef struct DebugContext{
	u8 nTimers;
	DebugTimer timers[MAX_DEBUG_TIMERS];
} DebugContext;

u8 debug_timer_start(u64 id, DebugContext *ctx, char *name);
void debug_timer_stop(DebugContext *ctx, u8 idx);
#ifndef SYSTEM_LAYER
void debug_draw(DebugContext *ctx, UIContext *uiCtx, RenderList *list, MemoryArena *frameArena);
#endif

#endif

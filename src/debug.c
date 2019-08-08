#include "debug.h"

u8 debug_timer_start(u64 id, DebugContext *ctx, char *name){
	if(!ctx)
		return MAX_DEBUG_TIMERS;		
	u64 startVal = systemAPI.system_get_perf_time();

	//Slow but done only few times a frame
	for(int i = 0; i < ctx->nTimers; i++){
		DebugTimer *timer = ctx->timers+i;
		if(timer->id == id){
			timer->start = startVal;
			return i;
		}
	}
	
	DebugTimer *timer = ctx->timers+ctx->nTimers;
	*timer = (DebugTimer){.id = id, .start = startVal};
	//TODO:Ugly hack fix this
	strcpy(timer->name, name);
	ctx->nTimers++;
	return ctx->nTimers-1;
}

void debug_timer_stop(DebugContext *ctx, u8 idx){
	if(!ctx || idx >= MAX_DEBUG_TIMERS)
		return;
	DebugTimer *timer = ctx->timers+idx;
	timer->value = systemAPI.system_get_perf_time() - timer->start;
}

void debug_draw(DebugContext *ctx, UIContext *uiCtx, RenderList *list, MemoryArena *frameArena){
	ui_draw_string(uiCtx, frameArena, list, 4, "Debug timers:");
	for(int i = 0; i < ctx->nTimers; i++){
		DebugTimer *timer = ctx->timers+i;
		u64 usecTime = timer->value*1.0/1000.0;
		ui_draw_string(uiCtx, frameArena, list, 4, "Function %s: %u usec", timer->name, usecTime);
	}
}

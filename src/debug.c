#include "debug.h"

u8 debug_timer_start(u64 id, DebugContext *ctx, char *name){
	if(!ctx)
		return MAX_DEBUG_TIMERS;		
#ifndef SYSTEM_LAYER
	u64 startVal = systemAPI.system_get_perf_time();
#else
	u64 startVal = system_get_perf_time();
#endif

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
#ifndef SYSTEM_LAYER
	timer->value = systemAPI.system_get_perf_time() - timer->start;
#else
	timer->value = system_get_perf_time() - timer->start;
#endif
	timer->avg = timer->avg*0.99+timer->value*0.01;
}

#ifndef SYSTEM_LAYER
void debug_draw(DebugContext *ctx, UIContext *uiCtx, RenderList *list, MemoryArena *frameArena){
	DEBUG_TIMER_START();
	ui_draw_string(uiCtx, frameArena, list, 4, "Debug timers:");
	for(int i = 0; i < ctx->nTimers; i++){
		DebugTimer *timer = ctx->timers+i;
		u64 usecAvg = timer->avg*1.0/1000.0;
		u64 usecValue = timer->value*1.0/1000.0;
		ui_draw_string(uiCtx, frameArena, list, 4, "Function %s: %u(%u) usec", timer->name, usecAvg, usecValue);
	}
	DEBUG_TIMER_STOP();
}
#endif

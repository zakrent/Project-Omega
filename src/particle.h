#ifndef Z_PARTICLE_H
#define Z_PARTICLE_H

//max per engine
#define MAX_PARTICLES 4096

typedef struct{
	u32 particleCount;
	hmm_v2 position[MAX_PARTICLES];
	hmm_v2 velocity[MAX_PARTICLES];
	r32 time[MAX_PARTICLES];
} Particles;

typedef struct{
	Particles SmokeParticles;
} ParticleSystem;

void particle_update(ParticleSystem *system);
void particle_draw(ParticleSystem *system, MemoryArena *frameArena, RenderList *list);

#endif

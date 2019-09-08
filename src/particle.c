#include "particle.h"

void particle_update(ParticleSystem *system){
	Particles *particles = &system->SmokeParticles;

	for(int i = 0; i < particles->particleCount; i++){
		if(particles->time[i] >= 480.0){
			particles->particleCount--;
			particles->position[i] = particles->position[particles->particleCount];
			particles->velocity[i] = particles->velocity[particles->particleCount];
			particles->time[i]     = particles->time[particles->particleCount];
		}
	}

	for(int i = 0; i < particles->particleCount; i++){
		particles->position[i] = HMM_AddVec2(particles->position[i], particles->velocity[i]);
		particles->velocity[i].X += 0.0001;
		particles->time[i] += 1.0;
	}

	particles->velocity[particles->particleCount] = HMM_Vec2(randf(-0.005, 0.005), randf(-0.015, -0.020));
	particles->time[particles->particleCount++] = 0.0;
	if(particles->particleCount == MAX_PARTICLES)
		particles->particleCount = 0;
}

void particle_draw(ParticleSystem *system, MemoryArena *frameArena, RenderList *list){
	Particles *particles = &system->SmokeParticles;
	for(int i = 0; i < particles->particleCount; i++){
		rl_draw_particle(frameArena, list, particles->position[i], HMM_Vec2(particles->time[i]/100.0, particles->time[i]/100.0), HMM_Vec2(0.0, 0.0), HMM_Vec2(1.0, 1.0), particles->time[i]);
	}
}

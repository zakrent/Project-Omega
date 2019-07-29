#ifndef ENTITY_STATS_H
#define ENTITY_STATS_H

typedef struct{
	u32 type;
	u32 controller;
	r32 spriteRotation;
	r32 initialHealth;
	hmm_vec2 rotationOffset;
	hmm_vec2 size;
	hmm_vec2 spritePos;
	hmm_vec2 spriteSize;
	union{
		struct{
			b32 friendly;
			r32 firingDelay;
		} shooterStats;

		struct{
			r32 damage;
		} projectileStats;
	};
} EntityStats;

enum EntityStats{
	STATS_TANK_1_HULL,
	STATS_TANK_1_TURRET,

	STATS_TURRET_1_BASE,
	STATS_TURRET_1_TURRET,

	STATS_PROJECTILE_1,

	COUNT_ENTITY_STATS
};

EntityStats entityStats[] = {
	{.type = ENTITY_TANK_HULL,   .controller = CONTROLLER_PATH_FOLLOWER, .size = {1.0, 1.0}, .spritePos = {16.0, 11.0}, .spriteSize = {1.0, 1.0}, //TANK_1_HULL
		.initialHealth = 10.0},
	{.type = ENTITY_TURRET,      .controller = CONTROLLER_SHOOTER,       .size = {1.0, 1.0}, .spritePos = {16.0, 12.0}, .spriteSize = {1.0, 1.0}, //TANK_1_TURRET
	   	.rotationOffset = {0.08, 0.0}, .shooterStats.friendly = false, .shooterStats.firingDelay = 30.0, .initialHealth = 1.0},

	{.type = ENTITY_TURRET_BASE, .controller = CONTROLLER_NONE,          .size = {1.0, 1.0}, .spritePos = {20.0, 7.0},  .spriteSize = {1.0, 1.0}, //TURRET_1_BASE
		.initialHealth = 100.0},
	{.type = ENTITY_TURRET,      .controller = CONTROLLER_SHOOTER,       .size = {1.0, 1.0}, .spritePos = {20.0, 10.0}, .spriteSize = {1.0, 1.0}, //TURRET_1_TURRET
		.spriteRotation = HMM_PI32*0.5, .rotationOffset = {0.0, -0.2}, .shooterStats.friendly = true, .shooterStats.firingDelay = 30.0, .initialHealth = 1.0},

	{.type = ENTITY_PROJECTILE,  .controller = CONTROLLER_PROJECTILE,    .size = {1.0, 1.0}, .spritePos = {19.0, 11.0}, .spriteSize = {1.0, 1.0}, //PROJECTILE_1
		.projectileStats.damage = 1.0, .initialHealth = 1.0},
};

#endif

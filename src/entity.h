#ifndef Z_ENTITY_H
#define Z_ENTITY_H

#define MAX_ENTITIES 1024

enum EntityType{
	ENTITY_TURRET,
	ENTITY_TANK,
	ENTITY_PROJECTILE,

	COUNT_ENTITY_TYPE
};

typedef struct{
	u16 index;
	u32 generation;
} EntityHandle;

typedef struct{
	u8 state;
	u8 waypointNumber;
	hmm_v2 waypoint;
} PathFollowerData;

typedef struct{
	b32 hasTarget;
	u32 firingDelayCounter;
	EntityHandle target;
} ShooterData;

typedef struct{
	b16 valid;
	u16 type;
	r32 health;
	r32 rotation;
	hmm_vec2 pos;
	union{
		struct{
			r32 turretRotation;
			ShooterData shooterData;
			PathFollowerData pathFollowerData;
		} tankData;

		struct{
			ShooterData shooterData;
		} turretData;
		
		struct{
			EntityHandle target;
		} projectileData;
	};
} Entity;

typedef struct{
	u32 generation;
	union{
		b16 valid;
		Entity entity;
	};
} EntityEntry;

typedef struct{
	EntityEntry entities[MAX_ENTITIES];
	u16 lastEntityIndex;
} EntitiesData;

EntityHandle entity_new(EntitiesData *data, Entity newEntity);
Entity *entity_get(EntitiesData *data, EntityHandle handle);
void entity_update(EntitiesData *data, Map *map);
void entity_draw(EntitiesData *data, MemoryArena *frameArena, RenderList *list);
void entity_spawn(EntitiesData *data, Map *map);

#endif

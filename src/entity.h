#ifndef Z_ENTITY_H
#define Z_ENTITY_H

#define MAX_ENTITIES 1024

enum EntityController{
	CONTROLLER_NONE,

	CONTROLLER_PATH_FOLLOWER,
	CONTROLLER_SHOOTER,
	CONTROLLER_PROJECTILE,

	COUNT_CONTROLLER
};

typedef struct{
	u16 index;
	u32 generation;
} EntityHandle;

typedef struct{
	b32 valid;
	u32 stats;
	b32 isMaster;
	r32 health;
	EntityHandle master;
	hmm_vec2 pos;
	r32 rotation;
	union{
		struct{
			u8 state;
			u8 waypointNumber;
			hmm_vec2 waypoint;
		} pathFollowerData;

		struct{
			b32 hasTarget;
			u32 firingDelayCounter;
			EntityHandle target;
		} shooterData;
		
		struct{
			EntityHandle target;
		} projectileData;
	};
} Entity;

typedef struct{
	u32 generation;
	union{
		b32 valid;
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

enum EntityPrefabId{
	EPI_INVALID,

	EPI_PROJECTILE,

	EPI_TURRET,
	EPI_TANK,

	COUNT_EPI
};

EntityHandle entity_spawn_prefab(EntitiesData *data, u32 prefabId, hmm_vec2 pos, r32 rotation);

#endif

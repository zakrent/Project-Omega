#ifndef Z_ENTITY_H
#define Z_ENTITY_H

#define MAX_ENTITIES 1024

enum EntityType{
	ENTITY_INVALID,

	ENTITY_MAIN,
	ENTITY_UNIT,
	ENTITY_TURRET,

	COUNT_ENTITY_TYPE
};

typedef struct{
	u16 index;
	u32 generation;
} EntityHandle;

typedef struct{
	b32 valid;
	u32 generation;
	u32 type;
	b32 isMaster;
	EntityHandle master;
	r32 rotation;
	r32 spriteRotation;
	hmm_vec2 pos;
	hmm_vec2 size;
	hmm_vec2 spritePos;
	hmm_vec2 spriteSize;
	union{
		struct{
			r32 health;
		} unitData;

		struct{
			b32 friendly;
		} turretData;
	};
} Entity;

typedef struct{
	Entity entities[MAX_ENTITIES];
	u16 lastEntityIndex;
} EntitiesData;

EntityHandle entity_new(EntitiesData *data, Entity newEntity);
void entity_update(EntitiesData *data);
void entity_draw(EntitiesData *data, MemoryArena *renderList);

enum EntityPrefabId{
	EPI_INVALID,

	EPI_TANK,

	COUNT_EPI
};

EntityHandle entity_spawn_prefab(EntitiesData *data, u32 prefabId, hmm_vec2 pos, r32 rotation);

#endif

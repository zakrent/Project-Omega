#include "entity.h"

EntityHandle entity_new(EntitiesData *data, Entity newEntity){
	for(int i = 0; i < MAX_ENTITIES; i++){
		Entity *e = data->entities+i;
		if(!e->valid){
			*e = newEntity;
			e->generation++;
			e->valid = true;
			return (EntityHandle){.index = i, .generation = e->generation};
		}
	}
	assert(0);
}

void entity_update(EntitiesData *data){
	for(int i = 0; i < MAX_ENTITIES; i++){
		Entity *e = data->entities+i;
		if(e->valid){
			switch(e->type){
				case ENTITY_UNIT:
					{
					hmm_v2 vel = HMM_MultiplyMat4ByVec4(HMM_Rotate(e->rotation, HMM_Vec3(0.0, 0.0, 1.0)), HMM_Vec4(0.05, 0.0, 0.0, 0.0)).XY;
					e->pos = HMM_AddVec2(e->pos, vel);
					e->rotation -= 0.05;
					break;
					}
				case ENTITY_TURRET:
					e->rotation += 0.1;
					break;
				default:
					break;
			}
		}
	}
}

void entity_draw(EntitiesData *data, MemoryArena *renderList){
	for(int i = 0; i < MAX_ENTITIES; i++){
		Entity *e = data->entities+i;
		if(e->valid){
			hmm_v2 absolutePos = e->pos;
			r32 absoluteRotation = e->rotation;
			if(!e->isMaster){
				absolutePos = HMM_AddVec2(absolutePos, data->entities[e->master.index].pos);
				absoluteRotation += data->entities[e->master.index].rotation;
			}
			rl_draw_sprite(renderList, absolutePos, absoluteRotation, e->size, e->spritePos, e->spriteSize);
		}
	}
}

EntityHandle entity_spawn_prefab(EntitiesData *data, u32 prefabId, hmm_vec2 pos, r32 rotation){
	switch(prefabId){
		case EPI_TANK:
			{
			EntityHandle master = entity_new(data, (Entity){.type = ENTITY_UNIT, .isMaster = true, .pos = pos, .spriteRotation = 0.0, .rotation = rotation,
				   	.size = HMM_Vec2(1.0,1.0), .spritePos = HMM_Vec2(16.0, 11.0), .spriteSize = HMM_Vec2(1.0, 1.0), .unitData.health = 100.0});
			entity_new(data, (Entity){.type = ENTITY_TURRET, .master = master, .pos = HMM_Vec2(0.0,0.0), .spriteRotation = 0.0, .rotation = 3.14*0.25,
				   	.size = HMM_Vec2(1.0,1.0), .spritePos = HMM_Vec2(16.0, 12.0), .spriteSize = HMM_Vec2(1.0, 1.0)});
			return master;
			}
		default:
			assert(0);
			break;
	}
}

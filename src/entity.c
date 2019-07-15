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
				case ENTITY_TANK_HULL:
					{
					hmm_v2 vel = HMM_MultiplyMat4ByVec4(HMM_Rotate(e->rotation, HMM_Vec3(0.0, 0.0, 1.0)), HMM_Vec4(0.01, 0.0, 0.0, 1.0)).XY;
					e->pos = HMM_AddVec2(e->pos, vel);
					e->rotation += 0.005;
					break;
					}
				case ENTITY_TURRET:
					e->rotation += 0.05;
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
				Entity *master = data->entities+e->master.index;
				//TODO: include master rotation offset
				hmm_m4 masterMatrix = HMM_Rotate(master->rotation, HMM_Vec3(0.0, 0.0, 1.0));
				absolutePos = HMM_MultiplyMat4ByVec4(masterMatrix, HMM_Vec4(absolutePos.X, absolutePos.Y, 0.0, 1.0)).XY;
				absolutePos = HMM_AddVec2(absolutePos, master->pos);
				absoluteRotation += data->entities[e->master.index].rotation;
			}
			rl_draw_sprite(renderList, absolutePos, absoluteRotation, e->rotationOffset, e->size, e->spritePos, e->spriteSize);
		}
	}
}

EntityHandle entity_spawn_prefab(EntitiesData *data, u32 prefabId, hmm_vec2 pos, r32 rotation){
	switch(prefabId){
		case EPI_TURRET:
			{
			EntityHandle master = entity_new(data, (Entity){.type = ENTITY_TURRET_BASE, .isMaster = true, .pos = pos, .spriteRotation = 0.0, .rotation = rotation,
				   	.size = HMM_Vec2(1.0,1.0), .spritePos = HMM_Vec2(20.0, 7.0), .spriteSize = HMM_Vec2(1.0, 1.0)});
			entity_new(data, (Entity){.type = ENTITY_TURRET, .master = master, .pos = HMM_Vec2(0.0,0.0), .spriteRotation = 0.0, .rotation = 0.0,
				   	.rotationOffset = HMM_Vec2(0.0, -0.2), .size = HMM_Vec2(1.0,1.0), .spritePos = HMM_Vec2(20.0, 10.0), .spriteSize = HMM_Vec2(1.0, 1.0)});
			return master;
			break;
			}
		case EPI_TANK:
			{
			EntityHandle master = entity_new(data, (Entity){.type = ENTITY_TANK_HULL, .isMaster = true, .pos = pos, .spriteRotation = 0.0, .rotation = rotation,
				   	.size = HMM_Vec2(1.0,1.0), .spritePos = HMM_Vec2(16.0, 11.0), .spriteSize = HMM_Vec2(1.0, 1.0), .unitData.health = 100.0});
			entity_new(data, (Entity){.type = ENTITY_TURRET, .master = master, .pos = HMM_Vec2(0.0,0.0), .spriteRotation = 0.0, .rotation = 3.14*0.25,
				   	.rotationOffset = HMM_Vec2(0.08, 0.0), .size = HMM_Vec2(1.0,1.0), .spritePos = HMM_Vec2(16.0, 12.0), .spriteSize = HMM_Vec2(1.0, 1.0)});
			return master;
			break;
			}
		default:
			assert(0);
			break;
	}
}

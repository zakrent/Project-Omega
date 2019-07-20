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

Entity *entity_get(EntitiesData *data, EntityHandle handle){
	assert(MAX_ENTITIES > handle.index);
	Entity* e = data->entities+handle.index;
	if(e->valid && e->generation == handle.generation){
		return e;
	}
	else{
		return NULL;
	}
}

hmm_v2 child_absolute_pos(Entity* master, hmm_v2 childPos){
	hmm_v2 absolutePos = childPos;
	hmm_m4 masterMatrix = HMM_Rotate(master->rotation, HMM_Vec3(0.0, 0.0, 1.0));
	absolutePos = HMM_MultiplyMat4ByVec4(masterMatrix, HMM_Vec4(absolutePos.X, absolutePos.Y, 0.0, 1.0)).XY;
	absolutePos = HMM_AddVec2(absolutePos, master->pos);
	return absolutePos;
}

r32 child_absolute_rotation(Entity* master, r32 childRotation){
	return childRotation + master->rotation;
}

void entity_update(EntitiesData *data){
	for(int i = 0; i < MAX_ENTITIES; i++){
		Entity *e = data->entities+i;
		if(e->valid){
			switch(e->type){
				case ENTITY_TANK_HULL:
					{
						hmm_v2 vel = HMM_MultiplyMat4ByVec4(HMM_Rotate(e->rotation, HMM_Vec3(0.0, 0.0, 1.0)), HMM_Vec4(0.05, 0.0, 0.0, 1.0)).XY;
						e->pos = HMM_AddVec2(e->pos, vel);
						e->rotation += 0.05;
						break;
					}
				case ENTITY_TURRET:
					{
						hmm_v2 absolutePos = e->pos;
						r32 absoluteRotation = e->rotation;
						if(!e->isMaster){
							Entity *master = data->entities+e->master.index;
							absolutePos = child_absolute_pos(master, absolutePos);
							absoluteRotation = child_absolute_rotation(master, absoluteRotation);
						}

						if(!e->turretData.hasTarget){
							b32 foundTarget = false;
							EntityHandle target;
							r32 smallestDistance = 0.0;
							for(int j = 0; j < MAX_ENTITIES; j++){
								Entity *t = data->entities+j;
								//Target types are always masters
								if(t->valid && j != i && ((t->type == ENTITY_TANK_HULL && e->turretData.friendly) 
										|| (t->type == ENTITY_TURRET_BASE && !e->turretData.friendly))){
									r32 distance = HMM_LengthVec2(HMM_SubtractVec2(e->pos, t->pos));
									if(!foundTarget || smallestDistance > distance){
										smallestDistance = distance;
										target.index = j;
										target.generation = t->generation;
										foundTarget = true;
									}
								}
							}
							if(foundTarget){
								e->turretData.hasTarget = true;
								e->turretData.target = target;
							}
						}
						else{
							Entity *t = entity_get(data, e->turretData.target);
							if(t){
								hmm_v2 direction = HMM_SubtractVec2(t->pos, absolutePos);
								e->rotation += HMM_ATan2F(direction.Y, direction.X)-absoluteRotation;
								if(e->turretData.firingDelay <= 0.0){
									entity_spawn_prefab(data, EPI_PROJECTILE, absolutePos, absoluteRotation);
									e->turretData.firingDelay = 10.0;
								}
								else{
									e->turretData.firingDelay -= 1.0;
								}
							}
							else{
								e->turretData.hasTarget = false;
							}
						}
					}
					break;
				case ENTITY_PROJECTILE:
					{
						hmm_v2 vel = HMM_MultiplyMat4ByVec4(HMM_Rotate(e->rotation, HMM_Vec3(0.0, 0.0, 1.0)), HMM_Vec4(0.2, 0.0, 0.0, 1.0)).XY;
						e->pos = HMM_AddVec2(e->pos, vel);
					}
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
			r32 absoluteRotation = e->rotation+e->spriteRotation;
			if(!e->isMaster){
				Entity *master = data->entities+e->master.index;
				absolutePos = child_absolute_pos(master, absolutePos);
				absoluteRotation = child_absolute_rotation(master, absoluteRotation);
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
			entity_new(data, (Entity){.type = ENTITY_TURRET, .master = master, .pos = HMM_Vec2(0.0,0.0), .spriteRotation = HMM_PI32*0.5, .rotation = 0.0,
				   	.rotationOffset = HMM_Vec2(0.0, -0.2), .size = HMM_Vec2(1.0,1.0), .spritePos = HMM_Vec2(20.0, 10.0), .spriteSize = HMM_Vec2(1.0, 1.0), .turretData.friendly = true});
			return master;
			break;
			}
		case EPI_TANK:
			{
			EntityHandle master = entity_new(data, (Entity){.type = ENTITY_TANK_HULL, .isMaster = true, .pos = pos, .spriteRotation = 0.0, .rotation = rotation,
				   	.size = HMM_Vec2(1.0,1.0), .spritePos = HMM_Vec2(16.0, 11.0), .spriteSize = HMM_Vec2(1.0, 1.0), .unitData.health = 100.0});
			entity_new(data, (Entity){.type = ENTITY_TURRET, .master = master, .pos = HMM_Vec2(0.0,0.0), .spriteRotation = 0.0, .rotation = 3.14*0.25,
				   	.rotationOffset = HMM_Vec2(0.08, 0.0), .size = HMM_Vec2(1.0,1.0), .spritePos = HMM_Vec2(16.0, 12.0), .spriteSize = HMM_Vec2(1.0, 1.0), .turretData.friendly = false});
			return master;
			break;
			}
		case EPI_PROJECTILE:
			{
				return entity_new(data, (Entity){.type = ENTITY_PROJECTILE, .isMaster = true, .pos = pos, .spriteRotation = 0.0, .rotation = rotation,
					.size = HMM_Vec2(0.5, 0.5), .spritePos = HMM_Vec2(19.0, 11.0), .spriteSize = HMM_Vec2(1.0, 1.0)});
			}
		default:
			assert(0);
			break;
	}
}

#include "entity.h"

EntityHandle entity_new(EntitiesData *data, Entity newEntity){
	for(int i = 0; i < MAX_ENTITIES; i++){
		EntityEntry *entry = data->entities+i;
		if(!entry->valid){
			entry->entity = newEntity;
			entry->generation++;
			entry->valid = true;
			return (EntityHandle){.index = i, .generation = entry->generation};
		}
	}
	assert(0);
	return (EntityHandle){0};
}

Entity *entity_get(EntitiesData *data, EntityHandle handle){
	assert(MAX_ENTITIES > handle.index);
	EntityEntry* entry = data->entities+handle.index;
	if(entry->valid && entry->generation == handle.generation){
		return &entry->entity;
	}
	return NULL;
}

void entity_follow_path(Entity *e, PathFollowerData *data, Map *map, r32 maxSpeed, r32 acceleration){
	switch(data->state){
		//find waypoint
		case 0:
			{
				if(data->waypointNumber > map->waypointCount - 1){
					e->valid = false; 
				}
				else{
					data->waypoint = map->waypoints[data->waypointNumber];
					data->waypointNumber++;
					data->state = 1;
				}
				break;
			}
		//rotate to waypoint
		case 1:
			{
				hmm_v2 direction = HMM_SubtractVec2(data->waypoint, e->pos);
				//r32 distance = HMM_LengthVec2(direction);
				r32 angleDelta = HMM_Clamp(-0.02, HMM_ATan2F(direction.Y, direction.X) - e->rotation, 0.02);
				if(HMM_ABS(angleDelta) <= 0.001){
					data->state = 2;
				}
				else{
					e->rotation += angleDelta;
				}
				break;
			}
		//go to waypoint
		case 2:
			{

				r32 distance = HMM_LengthVec2(HMM_SubtractVec2(data->waypoint, e->pos));

				r32 breakingDistance = (data->speed*data->speed)/(2*acceleration);

				if(breakingDistance > distance){
					data->speed -= acceleration;
					if(data->speed < 0.001)
						data->speed = 0.001;
				}
				else{
					data->speed += acceleration;
					if(data->speed > maxSpeed)
						data->speed = maxSpeed;
				}

				hmm_v2 vel = HMM_MultiplyMat4ByVec4(HMM_Rotate(e->rotation, HMM_Vec3(0.0, 0.0, 1.0)), HMM_Vec4(data->speed, 0.0, 0.0, 1.0)).XY;
				hmm_v2 newPos = HMM_AddVec2(e->pos, vel);
				r32 newDistance = HMM_LengthVec2(HMM_SubtractVec2(data->waypoint, newPos));
				if(newDistance > distance){
					data->speed = 0.0;
					data->state = 0;
				}
				else{
					e->pos = newPos;
				}
				break;
			}
		default:
			assert(0);
			break;
	}
}

r32 entity_target_bearing(Entity *e, EntitiesData *data, ShooterData *sdata, b32 isFriendly, b32 *hasTarget){
	if(!sdata->hasTarget){
		b32 foundTarget = false;
		EntityHandle target;
		r32 smallestDistance = 0.0;
		for(int j = 0; j < MAX_ENTITIES; j++){
			EntityEntry *te = data->entities+j;
			if(te->valid){
				Entity *t = &te->entity;
				if((t->type == ENTITY_TURRET && !isFriendly) || (t->type == ENTITY_TANK && isFriendly)){
					r32 distance = HMM_LengthVec2(HMM_SubtractVec2(e->pos, t->pos));
					if(!foundTarget || smallestDistance > distance){
						smallestDistance = distance;
						target.index = j;
						target.generation = te->generation;
						foundTarget = true;
					}
				}
			}
		}
		if(foundTarget){
			sdata->hasTarget = true;
			sdata->target = target;
		}
	}

	Entity *t = entity_get(data, sdata->target);
	if(t){
		hmm_v2 direction = HMM_SubtractVec2(t->pos, e->pos);
		*hasTarget = true;
		return HMM_ATan2F(direction.Y, direction.X);
	}
	else{
		sdata->hasTarget = false;
		*hasTarget = false;
	}
	return 0.0;
}

void entity_fire(Entity *e, EntitiesData *data, ShooterData *sdata, u32 firingDelay){
	if(sdata->firingDelayCounter <= 0.0){
		entity_new(data, (Entity){.type = ENTITY_PROJECTILE, .pos = e->pos, .projectileData.target = sdata->target});
		sdata->firingDelayCounter = firingDelay;
	}
	else{
		sdata->firingDelayCounter--;
	}
}

void entity_update(EntitiesData *data, Map *map){
	DEBUG_TIMER_START();
	for(int i = 0; i < MAX_ENTITIES; i++){
		EntityEntry *entry = data->entities+i;
		if(entry->valid){
			Entity *e = &entry->entity;
			switch(e->type){
				case ENTITY_TANK:
					{
						if(e->health <= 0.0){
							e->valid = false;
						}
						entity_follow_path(e, &e->tankData.pathFollowerData, map, 0.03, 0.0003);
						b32 hasTarget;
						r32 targetBearing = entity_target_bearing(e, data, &e->tankData.shooterData, false, &hasTarget);
						if(hasTarget){
							e->tankData.turretRotation = targetBearing - e->rotation;
							entity_fire(e, data, &e->tankData.shooterData, 60);
						}
						break;
					}
				case ENTITY_TURRET:
					{
						b32 hasTarget;
						r32 targetBearing = entity_target_bearing(e, data, &e->turretData.shooterData, true, &hasTarget);
						if(hasTarget){
							e->rotation = targetBearing;
							entity_fire(e, data, &e->turretData.shooterData, 60);
						}
						break;
					}
				case ENTITY_PROJECTILE:
					{
						Entity *t = entity_get(data, e->projectileData.target);
						if(!t){
							e->valid = false;
							break;
						}

						const r32 speed = 0.2;

						if(!e->projectileData.hasOldTargetPos)
							e->projectileData.oldTargetPos = t->pos;

						hmm_v2 inverseRawDirection = HMM_SubtractVec2(e->pos, t->pos);
						hmm_v2 targetVelocity = HMM_SubtractVec2(e->projectileData.oldTargetPos, t->pos);

						r32 cosOmega = HMM_DotVec2(HMM_NormalizeVec2(targetVelocity), HMM_NormalizeVec2(inverseRawDirection));
						r32 sinOmega = HMM_SquareRootF(1.0-cosOmega*cosOmega);
						r32 beta = asinf(sinOmega*(HMM_LengthVec2(targetVelocity)/speed));

						if(targetVelocity.Y < 0){
							beta *= -1;
						}

						e->rotation = HMM_ATan2F(-inverseRawDirection.Y, -inverseRawDirection.X)+beta;
						
						e->projectileData.oldTargetPos = t->pos;
						e->projectileData.hasOldTargetPos = true;

						r32 distance = HMM_LengthVec2(inverseRawDirection);
						hmm_v2 vel = HMM_MultiplyMat4ByVec4(HMM_Rotate(e->rotation, HMM_Vec3(0.0, 0.0, 1.0)), HMM_Vec4(0.2, 0.0, 0.0, 1.0)).XY;
						hmm_v2 newPos = HMM_AddVec2(e->pos, vel);
						r32 newDistance = HMM_LengthVec2(HMM_SubtractVec2(t->pos, newPos));
						if(newDistance > distance){
							t->health -= 1.0;
							e->valid = false;
						}
						else{
							e->pos = newPos;
						}
					}
				default:
					break;
			}
		}
	}
	DEBUG_TIMER_STOP();
}

void entity_draw(EntitiesData *data, MemoryArena *frameArena, RenderList *list){
	DEBUG_TIMER_START();
	for(int i = 0; i < MAX_ENTITIES; i++){
		EntityEntry *entry = data->entities+i;
		if(entry->valid){
			Entity *e = &entry->entity;
			const static float scalingFactor = 0.10;
			switch(e->type){
				case ENTITY_TANK:
					rl_draw_sprite(frameArena, list, e->pos, e->rotation, HMM_Vec2(0.0, 0.0), HMM_Vec2(13.0*scalingFactor, 5.0*scalingFactor), HMM_Vec2(0.0, 34.0), HMM_Vec2(13.0, 5.0));
					rl_draw_sprite(frameArena, list, e->pos, e->rotation+e->tankData.turretRotation, HMM_Vec2(1.0*scalingFactor, 0.0), HMM_Vec2(7.0*scalingFactor, 5.0*scalingFactor), HMM_Vec2(13.0, 34.0), HMM_Vec2(7.0, 5.0));
					break;
				case ENTITY_TURRET:
					rl_draw_sprite(frameArena, list, e->pos, e->rotation, HMM_Vec2(1.0*scalingFactor, 0.0), HMM_Vec2(7.0*scalingFactor, 5.0*scalingFactor), HMM_Vec2(0.0, 0.0), HMM_Vec2(7.0, 5.0));
					//rl_draw_sprite(frameArena, list, e->pos, e->rotation+HMM_PI32*0.5, HMM_Vec2(0.0, -0.2), HMM_Vec2(1.0, 1.0), HMM_Vec2(20.0, 10.0), HMM_Vec2(1.0, 1.0));
					break;
				case ENTITY_PROJECTILE:
					//rl_draw_sprite(frameArena, list, e->pos, e->rotation, HMM_Vec2(0.0, 0.0), HMM_Vec2(1.0, 1.0), HMM_Vec2(19.0, 11.0), HMM_Vec2(1.0, 1.0));
					break;
				default:
					break;
			}
		}
	}
	DEBUG_TIMER_STOP();
}

void entity_populate_prefabs(EntitiesData *data){
	EntityPrefab *prefabs = data->entityPrefabs;
	//Tank
	prefabs[EP_TANK].entity    = (Entity){.type = ENTITY_TANK, .health = 3.0};
	prefabs[EP_TANK].buildable = false;
	//Turret
	prefabs[EP_TURRET].entity    = (Entity){.type = ENTITY_TURRET};
	prefabs[EP_TURRET].buildable = true;
}

void entity_spawn(EntitiesData *data, u32 prefabId, hmm_v2 pos){
	Entity newEntity = data->entityPrefabs[prefabId].entity;
	newEntity.pos = pos;
	entity_new(data, newEntity);
}

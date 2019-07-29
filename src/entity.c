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
}

Entity *entity_get(EntitiesData *data, EntityHandle handle){
	assert(MAX_ENTITIES > handle.index);
	EntityEntry* entry = data->entities+handle.index;
	if(entry->valid && entry->generation == handle.generation){
		return &entry->entity;
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

void entity_update(EntitiesData *data, Map *map){
	for(int i = 0; i < MAX_ENTITIES; i++){
		EntityEntry *entry = data->entities+i;
		if(entry->valid){
			Entity *e = &entry->entity;
			Entity *m = NULL;
			EntityStats *s = entityStats+e->stats;
			if(!e->isMaster){
				m = entity_get(data, e->master);
				if(!m){
					e->valid = false;
					continue;
				}
			}

			if(e->health <= 0.0){
				e->valid = false;
			}
			
			switch(s->controller){
				case CONTROLLER_PATH_FOLLOWER:
					{
						switch(e->pathFollowerData.state){
							//find waypoint
							case 0:
								{
									if(e->pathFollowerData.waypointNumber > map->waypointCount - 1){
										e->valid = false; 
									}
									else{
										e->pathFollowerData.waypoint = map->waypoints[e->pathFollowerData.waypointNumber];
										e->pathFollowerData.waypointNumber++;
										e->pathFollowerData.state = 1;
									}
									break;
								}
							//rotate to waypoint
							case 1:
								{
									hmm_v2 direction = HMM_SubtractVec2(e->pathFollowerData.waypoint, e->pos);
									r32 distance = HMM_LengthVec2(direction);
									r32 angleDelta = HMM_Clamp(-0.08, HMM_ATan2F(direction.Y, direction.X) - e->rotation, 0.08);
									if(HMM_ABS(angleDelta) <= 0.001){
										e->pathFollowerData.state = 2;
									}
									else{
										e->rotation += angleDelta;
									}
									break;
								}
							//go to waypoint
							case 2:
								{
									r32 distance = HMM_LengthVec2(HMM_SubtractVec2(e->pathFollowerData.waypoint, e->pos));
									hmm_v2 vel = HMM_MultiplyMat4ByVec4(HMM_Rotate(e->rotation, HMM_Vec3(0.0, 0.0, 1.0)), HMM_Vec4(0.05, 0.0, 0.0, 1.0)).XY;
									hmm_v2 newPos = HMM_AddVec2(e->pos, vel);
									r32 newDistance = HMM_LengthVec2(HMM_SubtractVec2(e->pathFollowerData.waypoint, newPos));
									if(newDistance > distance){
										e->pathFollowerData.state = 0;
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
						break;
					}
				case CONTROLLER_SHOOTER:
					{
						hmm_v2 absolutePos = e->pos;
						r32 absoluteRotation = e->rotation;
						if(!e->isMaster){
							absolutePos = child_absolute_pos(m, absolutePos);
							absoluteRotation = child_absolute_rotation(m, absoluteRotation);
						}
						if(!e->shooterData.hasTarget){
							b32 foundTarget = false;
							EntityHandle target;
							r32 smallestDistance = 0.0;
							for(int j = 0; j < MAX_ENTITIES; j++){
								EntityEntry *te = data->entities+j;
								if(te->valid){
									Entity *t = &te->entity;
									EntityStats *ts = entityStats+t->stats;
									//Target types are always masters
									if(j != i && ((ts->type == ENTITY_TANK_HULL && s->shooterStats.friendly) 
											|| (ts->type == ENTITY_TURRET_BASE && !s->shooterStats.friendly))){
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
								e->shooterData.hasTarget = true;
								e->shooterData.target = target;
							}
						}
						else{
							Entity *t = entity_get(data, e->shooterData.target);
							if(t){
								hmm_v2 direction = HMM_SubtractVec2(t->pos, absolutePos);
								r32 distance = HMM_LengthVec2(direction);
								e->rotation += HMM_ATan2F(direction.Y, direction.X)-absoluteRotation;
								if(e->shooterData.firingDelayCounter <= 0.0){
									EntityHandle ph = entity_spawn_prefab(data, EPI_PROJECTILE, absolutePos, absoluteRotation);
									Entity* p = entity_get(data, ph);
									p->projectileData.target = e->shooterData.target;
									e->shooterData.firingDelayCounter = s->shooterStats.firingDelay;
								}
								else{
									e->shooterData.firingDelayCounter -= 1.0;
								}
							}
							else{
								e->shooterData.hasTarget = false;
							}
						}
						break;
					}
				case CONTROLLER_PROJECTILE:
					{
						Entity *t = entity_get(data, e->projectileData.target);
						if(!t){
							e->valid = false;
							continue;
						}

						hmm_v2 direction = HMM_SubtractVec2(t->pos, e->pos);
						e->rotation = HMM_ATan2F(direction.Y, direction.X);

						r32 distance = HMM_LengthVec2(direction);
						hmm_v2 vel = HMM_MultiplyMat4ByVec4(HMM_Rotate(e->rotation, HMM_Vec3(0.0, 0.0, 1.0)), HMM_Vec4(0.8, 0.0, 0.0, 1.0)).XY;
						hmm_v2 newPos = HMM_AddVec2(e->pos, vel);
						r32 newDistance = HMM_LengthVec2(HMM_SubtractVec2(t->pos, newPos));
						if(newDistance > distance){
							t->health -= s->projectileStats.damage;
							e->valid = false;
						}
						else{
							e->pos = newPos;
						}
						break;
					}
				default:
					break;
			}
		}
	}
}

void entity_draw(EntitiesData *data, MemoryArena *frameArena, RenderList *list){
	for(int i = 0; i < MAX_ENTITIES; i++){
		EntityEntry *entry = data->entities+i;
		if(entry->valid){
			Entity *e = &entry->entity;
			EntityStats *s = entityStats+e->stats;
			hmm_v2 absolutePos = e->pos;
			r32 absoluteRotation = e->rotation+s->spriteRotation;
			if(!e->isMaster){
				Entity *master = entity_get(data, e->master);
				if(!master)
					continue;
				absolutePos = child_absolute_pos(master, absolutePos);
				absoluteRotation = child_absolute_rotation(master, absoluteRotation);
			}
			rl_draw_sprite(frameArena, list, absolutePos, absoluteRotation, s->rotationOffset, s->size, s->spritePos, s->spriteSize);
		}
	}
}

EntityHandle entity_spawn_prefab(EntitiesData *data, u32 prefabId, hmm_vec2 pos, r32 rotation){
	switch(prefabId){
		case EPI_TURRET:
			{
			EntityHandle master = entity_new(data, (Entity){.stats = STATS_TURRET_1_BASE, .isMaster = true, .health = 100.0, .pos = pos, .rotation = rotation});
			entity_new(data, (Entity){.stats = STATS_TURRET_1_TURRET, .master = master, .pos = HMM_Vec2(0.0, 0.0), .health = 1.0});
			return master;
			break;
			}
		case EPI_TANK:
			{
			//TODO: health from stats
			EntityHandle master = entity_new(data, (Entity){.stats = STATS_TANK_1_HULL, .isMaster = true, .health = 100.0, .pos = pos, .rotation = rotation});
			entity_new(data, (Entity){.stats = STATS_TANK_1_TURRET, .master = master, .pos = HMM_Vec2(0.0, 0.0), .health = 1.0});
			return master;
			break;
			}
		case EPI_PROJECTILE:
			{
				return entity_new(data, (Entity){.stats = STATS_PROJECTILE_1, .isMaster = true, .health = 1.0, .pos = pos, .rotation = rotation});
			}
		default:
			assert(0);
			break;
	}
}

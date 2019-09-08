#include "map.h"

void map_generate(Map* map, u16 seed){
	//Clear map
	*map = (Map){0};

	//Generate road and waypoints
	i16 headX = 0;
	i16 headY = MAP_SIZE_Y*0.5;

	map->waypoints[0] = HMM_Vec2(-MAP_SIZE_X*0.5, headY-MAP_SIZE_Y*0.5+0.5);
	map->waypointCount++;

	u8 direction = 1;
	u16 dirLength = 0;
	u8 nTurns = 0;
	do{
		switch(direction){
			case 0:
				if(headY <= 2 || nTurns == MAX_TURNS - 1 || (dirLength >= 2 && randfu() < 0.1)){
					map->tile[headX][headY] = TILE_ROAD_TNE;
					map->waypoints[nTurns+1] = HMM_Vec2(headX + 0.5 - MAP_SIZE_X*0.5, headY + 0.5 - MAP_SIZE_Y*0.5);
					map->waypointCount++;
					headX++;
					nTurns++;
					dirLength = 0;
					direction = 1;
				}
				else{
					map->tile[headX][headY] = TILE_ROAD_NS;
					headY--;
					dirLength++;
				}
				break;
			case 1:
				if(dirLength >= 2 && nTurns < MAX_TURNS - 1 && headX < MAP_SIZE_X - 3 && headY >= 4 && randfu() < 0.2){
					map->tile[headX][headY] = TILE_ROAD_TSW;
					map->waypoints[nTurns+1] = HMM_Vec2(headX + 0.5 - MAP_SIZE_X*0.5, headY + 0.5 - MAP_SIZE_Y*0.5);
					map->waypointCount++;
					headY--;
					nTurns++;
					dirLength = 0;
					direction = 0;
				}
				else if(dirLength >= 2 && nTurns < MAX_TURNS - 1 && headX < MAP_SIZE_X - 3 && headY <= MAP_SIZE_Y-5 && randfu() < 0.2){
					map->tile[headX][headY] = TILE_ROAD_TES;
					map->waypoints[nTurns+1] = HMM_Vec2(headX + 0.5 - MAP_SIZE_X*0.5, headY + 0.5 - MAP_SIZE_Y*0.5);
					map->waypointCount++;
					headY++;
					nTurns++;
					dirLength = 0;
					direction = 2;
				}
				else{
					map->tile[headX][headY] = TILE_ROAD_WE;
					headX++;
					dirLength++;
				}
				break;
			case 2:
				if(headY >= MAP_SIZE_Y - 3 || nTurns == MAX_TURNS - 1 || (dirLength >= 2 && randfu() < 0.1)){
					map->tile[headX][headY] = TILE_ROAD_TWN;
					map->waypoints[nTurns+1] = HMM_Vec2(headX + 0.5 - MAP_SIZE_X*0.5, headY + 0.5 - MAP_SIZE_Y*0.5);
					map->waypointCount++;
					headX++;
					nTurns++;
					dirLength = 0;
					direction = 1;
				}
				else{
					map->tile[headX][headY] = TILE_ROAD_NS;
					headY++;
					dirLength++;
				}
				break;
			default:
				direction = 0;
				dirLength = 0;
				break;
		}
	}while(headX != -1 && headX != MAP_SIZE_X && headY != -1 && headY != MAP_SIZE_Y);

	map->waypoints[map->waypointCount] = HMM_Vec2(MAP_SIZE_X*0.5, headY-MAP_SIZE_Y*0.5+0.5);
	map->waypointCount++;

	//TODO: decals?
}

void map_draw(Map *map, MemoryArena *frameArena, RenderList* list){
	DEBUG_TIMER_START();
	if(!map->spriteCacheValid){
		for(int x = 0; x < MAP_SIZE_X; x++){
			for(int y = 0; y < MAP_SIZE_Y; y++){
				u8 spriteX = 6.0;
				u8 spriteY = 1.0;
				u8 tile = map->tile[x][y];
				switch(tile){
					case TILE_ROAD_NS:
						spriteX = 7.0;
						spriteY = 2.0;
						break;
					case TILE_ROAD_WE:
						spriteX = 6.0;
						spriteY = 3.0;
						break;
					case TILE_ROAD_TNE:
						spriteX = 3.0;
						spriteY = 2.0;
						break;
					case TILE_ROAD_TSW:
						spriteX = 4.0;
						spriteY = 3.0;
						break;
					case TILE_ROAD_TES:
						spriteX = 4.0;
						spriteY = 2.0;
						break;
					case TILE_ROAD_TWN:
						spriteX = 3.0;
						spriteY = 3.0;
						break;
					default:
						break;
				}
				rl_cache_simple_sprite(&(map->spriteCache[x][y]), HMM_Vec2(x-MAP_SIZE_X*0.5+0.5, y-MAP_SIZE_Y*0.5+0.5), 
						HMM_Vec2(1.0, 1.0), HMM_Vec2(spriteX,spriteY), HMM_Vec2(1.0,1.0));
			}
		}
		map->spriteCacheValid = true;
	}

	for(int x = 0; x < MAP_SIZE_X; x++){
		for(int y = 0; y < MAP_SIZE_Y; y++){
			rl_draw_cached_simple_sprite(frameArena, list, &(map->spriteCache[x][y]));
		}
	}

	DEBUG_TIMER_STOP();
}

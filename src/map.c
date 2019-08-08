#include "map.h"

void map_generate(Map* map, u16 seed){
	//Clear map
	*map = (Map){0};

	//Generate road and waypoints

	i16 headX = 0;
	i16 headY = MAP_SIZE_Y*0.5;

	map->waypoints[0] = HMM_Vec2(-MAP_SIZE_X*0.5, headY-MAP_SIZE_Y*0.5+1.0);
	map->waypointCount++;

	u8 direction = 1;
	u16 dirLength = 0;
	u8 nTurns = 0;
	do{
		switch(direction){
			case 0:
				if(headY <= 2 || nTurns == MAX_TURNS - 1 || dirLength >= 2 && randfu() < 0.1){
					map->tile[headX][headY]     = TILE_ROAD_W;
					map->tile[headX][headY-1]   = TILE_ROAD_TNW;
					map->tile[headX+1][headY-1] = TILE_ROAD_N;
					map->tile[headX+1][headY]   = TILE_ROAD_SE;
					map->waypoints[nTurns+1] = HMM_Vec2(headX + 1.0 - MAP_SIZE_X*0.5, headY - MAP_SIZE_Y*0.5);
					map->waypointCount++;
					headX += 2;
					headY -= 1;
					nTurns++;
					dirLength = 0;
					direction = 1;
				}
				else{
					map->tile[headX][headY]   = TILE_ROAD_W;
					map->tile[headX+1][headY] = TILE_ROAD_E;
					headY--;
					dirLength++;
				}
				break;
			case 1:
				if(dirLength >= 2 && nTurns < MAX_TURNS - 1 && headX < MAP_SIZE_X - 3 && headY >= 4 && randfu() < 0.2){
					map->tile[headX][headY]     = TILE_ROAD_NW;
					map->tile[headX+1][headY]   = TILE_ROAD_E;
					map->tile[headX][headY+1]   = TILE_ROAD_S;
					map->tile[headX+1][headY+1] = TILE_ROAD_TSE;
					map->waypoints[nTurns+1] = HMM_Vec2(headX + 1.0 - MAP_SIZE_X*0.5, headY + 1.0 - MAP_SIZE_Y*0.5);
					map->waypointCount++;
					headY--;
					nTurns++;
					dirLength = 0;
					direction = 0;
				}
				else if(dirLength >= 2 && nTurns < MAX_TURNS - 1 && headX < MAP_SIZE_X - 3 && headY <= MAP_SIZE_Y-5 && randfu() < 0.2){
					map->tile[headX][headY]     = TILE_ROAD_N;
					map->tile[headX+1][headY]   = TILE_ROAD_TNE;
					map->tile[headX][headY+1]   = TILE_ROAD_SW;
					map->tile[headX+1][headY+1] = TILE_ROAD_E;
					map->waypoints[nTurns+1] = HMM_Vec2(headX + 1.0 - MAP_SIZE_X*0.5, headY + 1.0 - MAP_SIZE_Y*0.5);
					map->waypointCount++;
					headY += 2;
					nTurns++;
					dirLength = 0;
					direction = 2;
				}
				else{
					map->tile[headX][headY]   = TILE_ROAD_N;
					map->tile[headX][headY+1] = TILE_ROAD_S;
					headX++;
					dirLength++;
				}
				break;
			case 2:
				if(headY >= MAP_SIZE_Y - 3 || nTurns == MAX_TURNS - 1 || dirLength >= 2 && randfu() < 0.1){
					map->tile[headX][headY]     = TILE_ROAD_W;
					map->tile[headX+1][headY]   = TILE_ROAD_NE;
					map->tile[headX][headY+1]   = TILE_ROAD_TSW;
					map->tile[headX+1][headY+1] = TILE_ROAD_S;
					map->waypoints[nTurns+1] = HMM_Vec2(headX + 1.0 - MAP_SIZE_X*0.5, headY + 1.0 - MAP_SIZE_Y*0.5);
					map->waypointCount++;
					headX += 2;
					nTurns++;
					dirLength = 0;
					direction = 1;
				}
				else{
					map->tile[headX][headY]   = TILE_ROAD_W;
					map->tile[headX+1][headY] = TILE_ROAD_E;
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

	map->waypoints[map->waypointCount] = HMM_Vec2(MAP_SIZE_X*0.5, headY-MAP_SIZE_Y*0.5+1.0);
	map->waypointCount++;

	//TODO: decals?
}

void map_draw(Map *map, MemoryArena *frameArena, RenderList* list){
	DEBUG_TIMER_START();
	for(int x = 0; x < MAP_SIZE_X; x++){
		for(int y = 0; y < MAP_SIZE_Y; y++){
			u8 spriteX = 1.0;
			u8 spriteY = 1.0;
			u8 tile = map->tile[x][y];
			if(tile >= TILE_ROAD_NW && tile <= TILE_ROAD_TSE){
				if(tile == TILE_ROAD){
					spriteX = 11;
					spriteY = 1;
				}
				else if(tile < TILE_ROAD_TNW){
					u8 directionIdx = tile - TILE_ROAD_NW;
					u8 xDiff = 2 - directionIdx % 3;
					u8 yDiff = 2 - (directionIdx - (directionIdx % 3)) / 3;
					spriteX = 0  + xDiff;
					spriteY = 9 + yDiff;
				}
				else{
					u8 directionIdx = tile - TILE_ROAD_TNW;
					u8 xDiff = directionIdx % 2;
					u8 yDiff = (directionIdx - (directionIdx % 2)) / 2;
					spriteX = 3 + xDiff;
					spriteY = 9 + yDiff;
				}
			}
			rl_draw_simple_sprite(frameArena, list, HMM_Vec2(x-MAP_SIZE_X*0.5+0.5, y-MAP_SIZE_Y*0.5+0.5), 
					HMM_Vec2(1.0, 1.0), HMM_Vec2(spriteX, spriteY), HMM_Vec2(1.0,1.0));
		}
	}
	for(int i = 0; i < map->waypointCount; i++){
		rl_draw_simple_sprite(frameArena, list, map->waypoints[i], HMM_Vec2(1.0, 1.0), HMM_Vec2(22.0, 0.0), HMM_Vec2(1.0,1.0));
	}
	DEBUG_TIMER_STOP();
}

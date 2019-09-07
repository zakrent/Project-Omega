#ifndef Z_MAP_H
#define Z_MAP_H

enum TileType{
	TILE_GRASS,

	TILE_ROAD_NW,
	TILE_ROAD_N,
	TILE_ROAD_NE,
	TILE_ROAD_W,
	TILE_ROAD,
	TILE_ROAD_E,
	TILE_ROAD_SW,
	TILE_ROAD_S,
	TILE_ROAD_SE,
	TILE_ROAD_TNW,
	TILE_ROAD_TNE,
	TILE_ROAD_TSW,
	TILE_ROAD_TSE,

	COUNT_TILE
};

#define MAX_TURNS 30
#define MAP_SIZE_Y 18
#define MAP_SIZE_X 30

typedef struct{
	u8 tile[MAP_SIZE_X][MAP_SIZE_Y];
	u8 waypointCount;
	b32 spriteCacheValid;
	RLDrawSprite spriteCache[MAP_SIZE_X][MAP_SIZE_Y];
	hmm_vec2 waypoints[MAX_TURNS+2];
} Map;

void map_generate(Map* map, u16 seed);
void map_draw(Map *map, MemoryArena *frameArena, RenderList* list);

#endif

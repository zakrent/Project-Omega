#ifndef Z_MAP_H
#define Z_MAP_H

enum TileType{
	TILE_GRASS,

	TILE_ROAD_NS,
	TILE_ROAD_WE,
	TILE_ROAD_TNE,
	TILE_ROAD_TES,
	TILE_ROAD_TSW,
	TILE_ROAD_TWN,

	COUNT_TILE
};

#define MAX_TURNS 30
#define MAP_SIZE_Y 19
#define MAP_SIZE_X 31

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

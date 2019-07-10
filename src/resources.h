#ifndef Z_RESOURCES_H
#define Z_RESOURCES_H

enum SpriteSheets{
	SS_INVALID,

	SS_BASIC,

	COUNT_SS
};

typedef struct{
	b32 valid;
	u32 handle;
	u32 width;
	u32 height;
	u32 xMul;
	u32 yMul;
} SpriteSheet;

enum Shaders{
	SHADER_INVALID,

	SHADER_BASIC,

	COUNT_SHADERS
};

typedef struct{
	b32 valid;
	u32 handle;
} Shader;

typedef struct{
	Shader shaders[COUNT_SHADERS];
	SpriteSheet spriteSheets[COUNT_SS];
} ResourceStatus;

Shader  resources_get_shader(u16 type, ResourceStatus status, MemoryArena tempArena);
SpriteSheet resources_get_sprite_sheet(u16 type, ResourceStatus status, MemoryArena tempArena);

#endif

#ifndef Z_RESOURCES_H
#define Z_RESOURCES_H

enum SpriteSheets{
	SS_INVALID,

	SS_BASIC,
	SS_UNITS,
	SS_FONT,

	COUNT_SS
};

typedef struct{
	b32 valid;
	u32 handle;
	u32 width;
	u32 height;
	r32 xMul;
	r32 yMul;
	r32 xOffset;
	r32 yOffset;
} SpriteSheet;

enum Shaders{
	SHADER_INVALID,

	SHADER_SPRITE,

	COUNT_SHADERS
};

typedef struct{
	b32 valid;
	u32 handle;
} Shader;

typedef struct{
	Shader shaders[COUNT_SHADERS];
	SpriteSheet spriteSheets[COUNT_SS];
} Resources;

Shader  resources_get_shader(u16 type, Resources *res);
SpriteSheet resources_get_sprite_sheet(u16 type, Resources *res);

#endif

#include "resources.h"

typedef struct{
	u16 fileType;        /* File type, always 4D42h ("BM") */
	u32 fileSize;        /* Size of the file in bytes */
	u16 reserved1;       /* Always 0 */
	u16 reserved2;       /* Always 0 */
	u32 bitmapOffset;    /* Starting position of image data in bytes */
	u32 size;            /* Size of this header in bytes */
	i32 width;           /* Image width in pixels */
	i32 height;          /* Image height in pixels */
	u16 planes;          /* Number of color planes */
	u16 bitsPerPixel;    /* Number of bits per pixel */
	u32 compression;     /* Compression methods used */
	u32 sizeOfBitmap;    /* Size of bitmap in bytes */
	i32 horzResolution;  /* Horizontal resolution in pixels per meter */
	i32 vertResolution;  /* Vertical resolution in pixels per meter */
	u32 colorsUsed;      /* Number of colors in the image */
	u32 colorsImportant; /* Minimum number of important colors */
	u32 redMask;         /* Mask identifying bits of red component */
	u32 greenMask;       /* Mask identifying bits of green component */
	u32 blueMask;        /* Mask identifying bits of blue component */
	u32 alphaMask;       /* Mask identifying bits of alpha component */
	u32 csType;          /* Color space type */
	i32 redX;            /* X coordinate of red endpoint */
	i32 redY;            /* Y coordinate of red endpoint */
	i32 redZ;            /* Z coordinate of red endpoint */
	i32 greenX;          /* X coordinate of green endpoint */
	i32 greenY;          /* Y coordinate of green endpoint */
	i32 greenZ;          /* Z coordinate of green endpoint */
	i32 blueX;           /* X coordinate of blue endpoint */
	i32 blueY;           /* Y coordinate of blue endpoint */
	i32 blueZ;           /* Z coordinate of blue endpoint */
	u32 gammaRed;        /* Gamma red coordinate scale value */
	u32 gammaGreen;      /* Gamma green coordinate scale value */
	u32 gammaBlue;       /* Gamma blue coordinate scale value */
} __attribute__((packed)) BMPHeader;

Shader resources_get_shader(u16 type, ResourceStatus status, MemoryArena tempArena){
}

SpriteSheet resources_get_sprite_sheet(u16 type, ResourceStatus *status, MemoryArena tempArena){
	if(status->spriteSheets[type].valid){
		return status->spriteSheets[type];
	}
	systemAPI.system_log(LOG_DEBUG, "Loading sprite sheet id: %u", type);
	File file;
	switch(type){
		case SS_BASIC:
			file = systemAPI.system_open_file("./res/basic.bmp");
			break;
		default:
			break;
	}
	BMPHeader *header = file.content;
	if(!file.valid || file.size < sizeof(BMPHeader) || (header->fileType != 0x4D42)){
		systemAPI.system_log(LOG_ERROR, "Invalid BMP file");
		systemAPI.system_close_file(file);
	}
	else{
		status->spriteSheets[type] = (SpriteSheet){
			.valid = true,
			.handle = systemAPI.system_generate_texture(file.content + header->bitmapOffset, header->width, header->height),
			.width = header->width,
			.height = header->height,
			.xMul = 64.0/header->width,
			.yMul = 64.0/header->height,
		};
	}
	return status->spriteSheets[type];
}

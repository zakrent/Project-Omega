#ifndef Z_SYSTEM_H
#define Z_SYSTEM_H

enum LogLevel{
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	COUNT_LOG_LEVEL
};

#define SYSTEM_LOG(name) void name(u8 logLevel, const char* fmt, ...)
typedef SYSTEM_LOG(SystemLog);

typedef struct{
	b32 valid;
	u64 size;
	void *content;
} File;

#define SYSTEM_OPEN_FILE(name) File name(const char* filename)
#define SYSTEM_CLOSE_FILE(name) void name(File file)
typedef SYSTEM_OPEN_FILE(SystemOpenFile);
typedef SYSTEM_CLOSE_FILE(SystemCloseFile);

#define SYSTEM_GENERATE_TEXTURE(name) u32 name(void* data, u32 width, u32 height)
typedef SYSTEM_GENERATE_TEXTURE(SystemGenerateTexture);

typedef struct{
	r32 time;
	SystemLog             *system_log;
	SystemOpenFile        *system_open_file;
	SystemCloseFile       *system_close_file;
	SystemGenerateTexture *system_generate_texture;
} System;

#endif

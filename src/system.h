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
#define SYSTEM_GENERATE_SHADER(name) u32 name(const char *vss, const char *fss)
typedef SYSTEM_GENERATE_SHADER(SystemGenerateShader);

#define SYSTEM_GET_PERF_TIME(name) u64 name()
typedef SYSTEM_GET_PERF_TIME(SystemGetPerfTime);

#define SOUND_BUFFER_SIZE 4096

typedef struct{
	r64 soundTime;
	b32 soundStarted;
	u32 writePosition;
	u32 readPosition;
	u32 sampleRate;
	r32 data[SOUND_BUFFER_SIZE];
} SoundBuffer;

typedef struct{
	r32 time;
	SoundBuffer *soundBuffer;
	SystemLog             *system_log;
	SystemOpenFile        *system_open_file;
	SystemCloseFile       *system_close_file;
	SystemGenerateTexture *system_generate_texture;
	SystemGenerateShader  *system_generate_shader;
	SystemGetPerfTime     *system_get_perf_time;
} System;

#endif

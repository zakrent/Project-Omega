#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define SYSTEM_LAYER
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"
#include "common.c"
#include "system.h"
#include "memory_arena.c"

SYSTEM_GET_PERF_TIME(system_get_perf_time);

#include "debug.c"

DebugContext *debugCtx;

#include "render_list.h"
#include "opengl_renderer.c"
#include "game.h"

#define FPS 60
#define DT (1.0/(FPS))

u32 windowWidth, windowHeight;

void glfw_error_callback(int error, const char* description){
	fprintf(stderr, "GLFW error: %s\n", description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
	windowWidth = width;
	windowHeight = height;
}

void die(const char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputs("\n", stderr);
	exit(EXIT_FAILURE);
}

SoundBuffer soundBuffer;
void audio_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount){
	if(soundBuffer.soundStarted){
		for(int i = 0; i < frameCount; i++){
			((r32*)pOutput)[i] = soundBuffer.data[soundBuffer.readPosition++];
			if(soundBuffer.readPosition >= SOUND_BUFFER_SIZE){
				soundBuffer.readPosition = 0;
			}
			assert(soundBuffer.readPosition != soundBuffer.writePosition);
		}
	}
}

SYSTEM_LOG(system_log){
	time_t rawtime;
	struct tm *timeinfo;
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	fprintf(stderr, "%i:%i:%i ", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	switch(logLevel){
		case LOG_DEBUG:
				fprintf(stderr, "[DEBUG]: ");
		break;
		case LOG_INFO:
				fprintf(stderr, "[INFO]: ");
		break;
		case LOG_WARN:
				fprintf(stderr, "[WARN]: ");
		break;
		case LOG_ERROR:
				fprintf(stderr, "[ERROR]: ");
		break;
	}
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputs("\n", stderr);
}

SYSTEM_OPEN_FILE(system_open_file){
	File file = (File){0};
	i32 fd = open(filename, O_RDWR);
	if(fd < 0){
		return file;
	}
	file.size = lseek(fd, 0, SEEK_END);	
	lseek(fd, 0, SEEK_SET);
	if(file.size < 0){
		return file;
	}
	file.content = mmap(NULL, file.size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if(file.content == MAP_FAILED){
		return file;
	}
	close(fd);
	file.valid = true;
	return file;
}

SYSTEM_CLOSE_FILE(system_close_file){
	munmap(file.content, file.size);
}

SYSTEM_GET_PERF_TIME(system_get_perf_time){
	struct timespec time;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
	u64 unitedTime = time.tv_sec * (u64)1e9 + time.tv_nsec;
	return unitedTime;
}

int main(){
	//Create window with opengl context
	if(!glfwInit()){
		die("GLFW initialization failed.");
	}
	glfwSetErrorCallback(glfw_error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Project Omega", NULL, NULL);
	if (!window){
		die("Window creation failed. Minimum required OpenGL version may not be avaible.");
	}
	windowWidth = 1280;
	windowHeight = 720;

	glfwMakeContextCurrent(window);

	//Set GLFW callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Load OpenGL 4.5
	if(glewInit() != GLEW_OK || !GLEW_VERSION_4_5){
		die("Could not load OpenGL 4.5 function pointers.");
	}

	//Enable vsync
	glfwSwapInterval(1);

	//Allocate game memory
	GameMemory memory;
	memory.permanentMemory = mmap((void*)TERABYTES(10), MEGABYTES(16), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	memory.transientMemory = mmap((void*)TERABYTES(11), MEGABYTES(16), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	memory.permanentMemorySize = MEGABYTES(16);
	memory.transientMemorySize = MEGABYTES(16);
	if(!memory.permanentMemory || !memory.transientMemory){
		die("Could not allocate game memory.");
	}

	//Load game dl
	void *handle = dlopen ("./game.so", RTLD_LAZY);
	if(!handle){
		die("Could not load game.so");
	}

	//Load game's function pointers
	Frame *frame = dlsym(handle, "frame");
	if(!frame){
		die("Could not get game function pointers.");
	}

	//Init opengl system
	ZGLState glState = opengl_state_init();

	System systemAPI = (System){
		.soundBuffer = &soundBuffer,
		.system_log              = system_log,
		.system_open_file        = system_open_file,
		.system_close_file       = system_close_file,
		.system_generate_texture = opengl_generate_texture,
		.system_get_perf_time    = system_get_perf_time,
	};

	//Init audio (hopefully)

	ma_result result;
	ma_device device;
	soundBuffer.sampleRate = ma_get_best_sample_rate_within_range(40000,48000);

	ma_device_config config = ma_device_config_init(ma_device_type_playback);
	config.playback.pDeviceID = NULL;
	config.playback.format    = ma_format_f32;
	config.playback.channels  = 1;
	config.sampleRate         = soundBuffer.sampleRate;
	config.dataCallback       = audio_data_callback;
	//config.pUserData          = &myUserData;

	result = ma_device_init(NULL, &config, &device);
	if(result != MA_SUCCESS) {
		die("Failed to open playback device.");
	}

	result = ma_device_start(&device);
	if(result != MA_SUCCESS) {
		die("Failed to start playback device.");
	}


	//Main loop
	GameInput input;
	double frameStart = glfwGetTime();
	while (!glfwWindowShouldClose(window)){
		DEBUG_TIMER_START();

		glfwGetCursorPos(window, &input.mouseX, &input.mouseY);
		input.mouseX = (input.mouseX/(windowWidth*0.5)-1.0)*windowWidth/windowHeight;
		input.mouseY = input.mouseY/(windowHeight*0.5)-1.0;
		b32 LMBDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;
		if(LMBDown != input.LMBDown){
			input.LMBChanged = true;
		}
		else{
			input.LMBChanged = false;
		}
		input.LMBDown = LMBDown;

		systemAPI.time = frameStart;
		RenderList *renderList;
		frame(memory, input, systemAPI, &renderList, &debugCtx);
		
		opengl_render_list(renderList, glState);

        glfwSwapBuffers(window);
		glfwPollEvents();

		DEBUG_TIMER_STOP();
		double frameTime = glfwGetTime()-frameStart;
		if(DT > frameTime){
			//POSIX.1-2001 deprecated this function but linux still supports it
			usleep((DT-frameTime)*10e5);
		}
		else{
			//Frame took too long
			printf("FRAME TOO LONG!\n");
		}
		frameStart = glfwGetTime();
	}

    ma_device_uninit(&device);

	return 0;
}

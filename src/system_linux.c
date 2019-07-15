#include <stdlib.h>
#include <stdio.h>
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
#define HANDMADE_MATH_IMPLEMENTATION
#define SYSTEM_LAYER
#include "HandmadeMath.h"
#include "common.h"
#include "system.h"
#include "memory_arena.c"
#include "opengl_renderer.c"
#include "game.h"

#define FPS 60
#define DT (1.0/(FPS))

void glfw_error_callback(int error, const char* description){
	fprintf(stderr, "GLFW error: %s\n", description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
}

void die(const char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputs("\n", stderr);
	exit(EXIT_FAILURE);
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

int main(){
	//Create window with opengl context
	if(!glfwInit()){
		die("GLFW initialization failed.");
	}
	glfwSetErrorCallback(glfw_error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	GLFWwindow* window = glfwCreateWindow(1080, 720, "Project Omega", NULL, NULL);
	if (!window){
		die("Window creation failed. Minimum required OpenGL version may not be avaible.");
	}

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
	memory.permanentMemory = mmap((void*)TERABYTES(10), MEGABYTES(256), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	memory.transientMemory = mmap((void*)TERABYTES(11), MEGABYTES(512), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	memory.permanentMemorySize = MEGABYTES(256);
	memory.transientMemorySize = MEGABYTES(512);
	if(!memory.permanentMemory || !memory.transientMemory){
		die("Could not allocate game memory.");
	}

	//Load game dl
	void *handle = dlopen ("./game.so", RTLD_LAZY);
	if(!dlopen){
		die("Could not load game.so");
	}

	//Load game's function pointers
	Frame *frame = dlsym(handle, "frame");
	if(!frame){
		die("Could not get game function pointers.");
	}

	//Init opengl system
	GLState glState = opengl_state_init();

	System systemAPI = (System){
		.system_log              = system_log,
		.system_open_file        = system_open_file,
		.system_close_file       = system_close_file,
		.system_generate_texture = opengl_generate_texture,
	};

	//Main loop
	double frameStart = glfwGetTime();
	while (!glfwWindowShouldClose(window)){
		systemAPI.time = frameStart;

		MemoryArena *renderList;
		frame(memory, systemAPI, &renderList);
		
		opengl_render_list(renderList, glState);

        glfwSwapBuffers(window);
		glfwPollEvents();

		double frameTime = glfwGetTime()-frameStart;
		if(DT > frameTime){
			//POSIX.1-2001 deprecated this function but linux still supports it
			usleep((DT-frameTime)*10e5);
		}
		frameStart = glfwGetTime();
	}
	return 0;
}

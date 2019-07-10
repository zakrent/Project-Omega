#ifndef Z_OPENGL_RENDERER_H
#define Z_OPENGL_RENDERER_H

typedef struct{
	GLuint mainVBO;
	GLuint mainVAO;
	GLuint shader;
	GLint mvpLocation;
} GLState;

GLState opengl_state_init();
void opengl_render_list(MemoryArena *renderList, GLState state);

#endif

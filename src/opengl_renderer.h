#ifndef Z_OPENGL_RENDERER_H
#define Z_OPENGL_RENDERER_H

typedef struct{
	GLuint vertexVBO;
	GLuint textureVBO;
	r32 texXMul;
	r32 texYMul;
	r32 texXOffset;
	r32 texYOffset;
	GLuint mainVAO;
	GLuint basicShader;
	GLuint spriteShader;
	GLint mvpLocation;
	GLint sizeXLocation;
	GLint sizeYLocation;
	GLint posXLocation;
	GLint posYLocation;
	hmm_m4 v;
	hmm_m4 p;
} GLState;

GLState opengl_state_init();
void opengl_render_list(RenderList *renderList, GLState state);

#endif

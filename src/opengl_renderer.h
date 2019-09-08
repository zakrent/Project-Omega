#ifndef Z_OPENGL_RENDERER_H
#define Z_OPENGL_RENDERER_H

struct ZGLVertex{
	hmm_v2 position;
	hmm_v2 texture;
	r32 time; //Used for particles
} __attribute__((packed));
typedef struct ZGLVertex ZGLVertex;

struct ZGLSprite{
	ZGLVertex vertices[6];
} __attribute__((packed));
typedef struct ZGLSprite ZGLSprite;


#define SPRITE_BUFFER_SIZE 1024
typedef struct{
	GLuint VBO;
	GLuint msaaFBO;
	GLuint msaaTexture;
	r32 texXMul;
	r32 texYMul;
	r32 texXOffset;
	r32 texYOffset;
	hmm_m4 v;
	hmm_m4 p;
} ZGLState;

ZGLState opengl_state_init();
void opengl_render_list(RenderList *renderList, ZGLState state, r32 time);

#endif

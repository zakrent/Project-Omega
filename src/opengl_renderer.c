#include "opengl_renderer.h"
#include "render_list.h"

const float defaultPoints[] = {
	-1.0, -1.0,
	+1.0, -1.0,
	+1.0, +1.0,
	-1.0, -1.0,
	+1.0, +1.0,
	-1.0, +1.0
};
const float defaultTexturePoints[] = {
	0.0, 0.0,
	1.0, 0.0,
	1.0, 1.0,
	0.0, 0.0,
	1.0, 1.0,
	0.0, 1.0
};

const char* spriteVertexShader =
"#version 400\n"
"layout(location = 0) in vec2 vp;"
"layout(location = 1) in vec2 texCord;"
"out vec2 TexCord;"
"uniform mat4  mvp[256];"
"uniform float sizeX[256];"
"uniform float sizeY[256];"
"uniform float posX[256];"
"uniform float posY[256];"
"void main() {"
"  TexCord.x = texCord.x*sizeX[gl_InstanceID]+posX[gl_InstanceID];"
"  TexCord.y = texCord.y*sizeY[gl_InstanceID]+posY[gl_InstanceID];"
"  gl_Position = mvp[gl_InstanceID]*vec4(vp, 0.0, 1.0);"
"}";

const char* spriteFragmentShader =
"#version 400\n"
"in vec2 TexCord;"
"out vec4 frag_colour;"
"uniform sampler2D spriteSheet;"
"void main() {"
"  frag_colour = texture(spriteSheet, vec2(TexCord.x, 1.0-TexCord.y));"
"}";

const char* basicVertexShader =
"#version 400\n"
"layout(location = 0) in vec2 vp;"
"layout(location = 1) in vec2 texCord;"
"out vec2 TexCord;"
"void main() {"
"  TexCord = texCord;"
"  gl_Position = vec4(vp, 0.0, 1.0);"
"}";

const char* basicFragmentShader =
"#version 400\n"
"in vec2 TexCord;"
"out vec4 frag_colour;"
"uniform sampler2D tex;"
"void main() {"
"  vec4 texVal = texture(tex, TexCord);"
"  frag_colour = texVal;"
"}";

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam ){
	fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
	( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
	type, severity, message );
}

SYSTEM_GENERATE_TEXTURE(opengl_generate_texture){
	GLuint handle;
	glGenTextures(1, &handle);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA,
			              GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	return handle;
}

GLuint opengl_create_shader(const char *vss, const char *fss){
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vss, NULL);
	glCompileShader(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fss, NULL);
	glCompileShader(fs);

	GLuint sp = glCreateProgram();
	glAttachShader(sp, fs);
	glAttachShader(sp, vs);
	glLinkProgram(sp);
	return sp;
}

GLState opengl_state_init(){
	glEnable              ( GL_DEBUG_OUTPUT );
	glDebugMessageCallback( MessageCallback, 0 );

	GLState state;

	glGenVertexArrays(1, &(state.mainVAO));
	glBindVertexArray(state.mainVAO);

	glGenBuffers(1, &(state.vertexVBO));
	glBindBuffer(GL_ARRAY_BUFFER, state.vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), defaultPoints, GL_STATIC_DRAW);

	glGenBuffers(1, &(state.textureVBO));
	glBindBuffer(GL_ARRAY_BUFFER, state.textureVBO);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), defaultTexturePoints, GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, state.vertexVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, state.textureVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	//Create sprite shader
	state.spriteShader = opengl_create_shader(spriteVertexShader, spriteFragmentShader);
	state.mvpLocation = glGetUniformLocation(state.spriteShader, "mvp");
	state.sizeXLocation = glGetUniformLocation(state.spriteShader, "sizeX");
	state.sizeYLocation = glGetUniformLocation(state.spriteShader, "sizeY");
	state.posXLocation = glGetUniformLocation(state.spriteShader, "posX");
	state.posYLocation = glGetUniformLocation(state.spriteShader, "posY");

	//Create basic shader
	state.basicShader = opengl_create_shader(basicVertexShader, basicFragmentShader);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	state.texXMul = 1.0;
	state.texYMul = 1.0;

	state.p = HMM_Orthographic(-1.0*16.0/9.0, 1.0*16.0/9.0, 1.0, -1.0, 0.0, 100.0);
	state.v = HMM_Scale(HMM_Vec3(1.0, 1.0, 1.0));

	glClearColor(1.0, 0.0, 1.0, 0.0);

	return state;
}

void opengl_draw_buffered_sprites(GLState state, u32 n, hmm_m4 *mvp, float *sizeX, float *sizeY, float *posX, float *posY){
	glUniformMatrix4fv(state.mvpLocation, n, GL_FALSE, (float*)mvp);
	glUniform1fv(state.sizeXLocation, n, sizeX);
	glUniform1fv(state.sizeYLocation, n, sizeY);
	glUniform1fv(state.posXLocation,  n, posX);
	glUniform1fv(state.posYLocation,  n, posY);

	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, n);
}

void opengl_render_list(RenderList *renderList, GLState state){
	//Resize window
	r32 windowSizeData[4];
	glGetFloatv(GL_VIEWPORT, windowSizeData);
	r32 wWidth  = windowSizeData[2];
	r32 wHeight = windowSizeData[3];
	r32 aspectRatio = wWidth/wHeight;
	state.p = HMM_Orthographic(-1.0*aspectRatio, 1.0*aspectRatio, 1.0, -1.0, 0.0, 100.0);

	//Create framebuffer to render to
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	GLuint fboTexture;
	glGenTextures(1, &fboTexture);
	glBindTexture(GL_TEXTURE_2D, fboTexture);
	  
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wWidth*2.0, wHeight*2.0, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

	glViewport(0, 0, wWidth*2.0, wHeight*2.0);
	//Use correct shaders
	glUseProgram(state.spriteShader);

	//Create sprite buffer
#define SPRITE_BUFFER_SIZE 256
	hmm_m4 spriteMVP[SPRITE_BUFFER_SIZE];
	float spriteSizeX[SPRITE_BUFFER_SIZE];
	float spriteSizeY[SPRITE_BUFFER_SIZE];
	float spritePosX[SPRITE_BUFFER_SIZE];
	float spritePosY[SPRITE_BUFFER_SIZE];
	u32 bufferedSprites = 0;


	//Draw render list
	RLEntryHeader *header = renderList->first;
	while(header){
		if(bufferedSprites && (header->type != RL_DRAW_SPRITE || bufferedSprites == SPRITE_BUFFER_SIZE)){
			opengl_draw_buffered_sprites(state, bufferedSprites, spriteMVP, spriteSizeX, spriteSizeY, spritePosX, spritePosY);
			bufferedSprites = 0;
		}
		switch(header->type){
			case RL_COLOR_CLEAR:
				{
					glClear(GL_COLOR_BUFFER_BIT);
					break;
				}
			case RL_USE_TEXTURE:
				{
					RLUseTexture *rlUseTexture = header->data;
					glBindTexture(GL_TEXTURE_2D, rlUseTexture->handle);
					state.texXMul = rlUseTexture->xMul;
					state.texYMul = rlUseTexture->yMul;
					break;
				}
			case RL_SET_CAMERA:
				{
					RLSetCamera *rlSetCamera = header->data;
					state.v = HMM_MultiplyMat4(HMM_Translate(HMM_Vec3(rlSetCamera->pos.X, rlSetCamera->pos.Y, 0.0)),
							HMM_Scale(HMM_Vec3(1.0/rlSetCamera->size.Width, 1.0/rlSetCamera->size.Height, 1.0)));
					break;
				}
			case RL_DRAW_SPRITE:
				{
					RLDrawSprite *rlDrawSprite = header->data;
					
					hmm_m4 m = HMM_MultiplyMat4( HMM_Translate(HMM_Vec3(rlDrawSprite->pos.X, rlDrawSprite->pos.Y, 0.0)),
							   HMM_MultiplyMat4( HMM_Rotate(rlDrawSprite->rotation, HMM_Vec3(0.0,0.0,1.0)),
							   HMM_MultiplyMat4( HMM_Translate(HMM_Vec3(rlDrawSprite->rotationOffset.X, rlDrawSprite->rotationOffset.Y, 0.0)),
												 HMM_Scale(HMM_Vec3(rlDrawSprite->size.Width*0.5, rlDrawSprite->size.Height*0.5, 1.0)))));

					spriteMVP[bufferedSprites] = HMM_MultiplyMat4(state.p, HMM_MultiplyMat4(state.v, m));
					spriteSizeX[bufferedSprites] = rlDrawSprite->spriteSize.X*state.texXMul;
					spriteSizeY[bufferedSprites] = rlDrawSprite->spriteSize.Y*state.texYMul;
					spritePosX[bufferedSprites] = rlDrawSprite->spritePos.X*state.texXMul;
					spritePosY[bufferedSprites] = rlDrawSprite->spritePos.Y*state.texYMul;
					bufferedSprites++;
					break;
				}
			default:
				break;
		}
		header = header->nextEntry;
	}

	if(bufferedSprites)
		opengl_draw_buffered_sprites(state, bufferedSprites, spriteMVP, spriteSizeX, spriteSizeY, spritePosX, spritePosY);

	//Render and delete framebuffer
	glViewport(0, 0, wWidth*1.0, wHeight*1.0);
	glBindBuffer(GL_ARRAY_BUFFER, state.textureVBO);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), defaultTexturePoints, GL_STREAM_DRAW);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(state.basicShader);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, fboTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &fboTexture);
}

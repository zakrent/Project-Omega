#include "opengl_renderer.h"
#include "render_list.h"

const char* spriteVertexShader =
"#version 400\n"
"layout(location = 0) in vec2 position;"
"layout(location = 1) in vec2 texture;"
"out vec2 Texture;"
"void main() {"
"  Texture = vec2(texture.x, texture.y);"
"  gl_Position = vec4(position.x, position.y, 0.0, 1.0);"
"}";

const char* spriteFragmentShader =
"#version 400\n"
"in vec2 Texture;"
"out vec4 color;"
"uniform sampler2D spriteSheet;"
"void main() {"
"  color = texture(spriteSheet, vec2(Texture.x, Texture.y));"
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

	char buffer[256];
	glGetShaderInfoLog(vs, 256, NULL, buffer);
	puts(buffer);
	glGetShaderInfoLog(fs, 256, NULL, buffer);

	GLuint sp = glCreateProgram();
	glAttachShader(sp, fs);
	glAttachShader(sp, vs);
	glLinkProgram(sp);

	glGetProgramInfoLog(sp, 256, NULL, buffer);
	puts(buffer);

	return sp;
}

ZGLState opengl_state_init(){
	glEnable              ( GL_DEBUG_OUTPUT );
	glDebugMessageCallback( MessageCallback, 0 );

	ZGLState state;

	u32 VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &state.VBO); 
	glBindBuffer(GL_ARRAY_BUFFER, state.VBO);
	glBufferData(GL_ARRAY_BUFFER, SPRITE_BUFFER_SIZE*sizeof(ZGLSprite), 0, GL_STREAM_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ZGLVertex), (void*)offsetof(ZGLVertex, position));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ZGLVertex), (void*)offsetof(ZGLVertex, texture));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	state.spriteShader = opengl_create_shader(spriteVertexShader, spriteFragmentShader);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	state.texXMul = 1.0;
	state.texYMul = 1.0;

	state.p = HMM_Orthographic(-1.0*16.0/9.0, 1.0*16.0/9.0, 1.0, -1.0, 0.0, 100.0);
	state.v = HMM_Scale(HMM_Vec3(1.0, 1.0, 1.0));

	glClearColor(0.53, 0.81, 0.92, 0.0);

	return state;
}

void opengl_render_list(RenderList *renderList, ZGLState state){
	DEBUG_TIMER_START();
	//Resize window
	r32 windowSizeData[4];
	glGetFloatv(GL_VIEWPORT, windowSizeData);
	r32 wWidth  = windowSizeData[2];
	r32 wHeight = windowSizeData[3];
	r32 aspectRatio = wWidth/wHeight;
	state.p = HMM_Orthographic(-1.0*aspectRatio, 1.0*aspectRatio, 1.0, -1.0, 0.0, 100.0);

	//Use correct shader
	glUseProgram(state.spriteShader);

	//Draw render list
	u32 bufferedSprites = 0;
	ZGLSprite sprites[SPRITE_BUFFER_SIZE];

	RLEntryHeader *header = renderList->first;
	while(header){
		if(bufferedSprites && (header->type != RL_DRAW_SPRITE || bufferedSprites == SPRITE_BUFFER_SIZE)){
			glBufferSubData(GL_ARRAY_BUFFER, 0, bufferedSprites*sizeof(ZGLSprite), sprites);
			glDrawArrays(GL_TRIANGLES, 0, bufferedSprites*6);
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
					state.texXOffset = rlUseTexture->xOffset;
					state.texYOffset = rlUseTexture->yOffset;
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

					ZGLSprite sprite;

					hmm_m4 spriteMVP = HMM_MultiplyMat4(state.p, HMM_MultiplyMat4(state.v, rlDrawSprite->model));

					sprite.vertices[0].position = HMM_MultiplyMat4ByVec4(spriteMVP, HMM_Vec4(-1.0, -1.0, 0.0, 1.0)).XY;
					sprite.vertices[1].position = HMM_MultiplyMat4ByVec4(spriteMVP, HMM_Vec4(-1.0, +1.0, 0.0, 1.0)).XY;
					sprite.vertices[2].position = HMM_MultiplyMat4ByVec4(spriteMVP, HMM_Vec4(+1.0, +1.0, 0.0, 1.0)).XY;
					sprite.vertices[3].position = HMM_MultiplyMat4ByVec4(spriteMVP, HMM_Vec4(+1.0, -1.0, 0.0, 1.0)).XY;
					sprite.vertices[4].position = sprite.vertices[0].position;
					sprite.vertices[5].position = sprite.vertices[2].position;

					r32 spriteSizeX = rlDrawSprite->spriteSize.X*state.texXMul-2.0*state.texXOffset;
					r32 spriteSizeY = rlDrawSprite->spriteSize.Y*state.texYMul-2.0*state.texYOffset;
					r32 spritePosX  = rlDrawSprite->spritePos.X*state.texXMul+state.texXOffset;
					r32 spritePosY  = rlDrawSprite->spritePos.Y*state.texYMul+state.texYOffset;

					sprite.vertices[1].texture = HMM_Vec2(spritePosX, spritePosY+spriteSizeY);
					sprite.vertices[0].texture = HMM_Vec2(spritePosX, spritePosY);
					sprite.vertices[3].texture = HMM_Vec2(spritePosX+spriteSizeX, spritePosY);
					sprite.vertices[2].texture = HMM_Vec2(spritePosX+spriteSizeX, spritePosY+spriteSizeY);
					sprite.vertices[4].texture = sprite.vertices[0].texture;
					sprite.vertices[5].texture = sprite.vertices[2].texture;

					sprites[bufferedSprites] = sprite;

					bufferedSprites++;
					break;
				}
			default:
				break;
		}
		header = header->nextEntry;
	}

	glBufferSubData(GL_ARRAY_BUFFER, 0, bufferedSprites*sizeof(ZGLSprite), sprites);
	glDrawArrays(GL_TRIANGLES, 0, bufferedSprites*6);

	DEBUG_TIMER_STOP();
}

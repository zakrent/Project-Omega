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

const char* vertexShader =
"#version 400\n"
"layout(location = 0) in vec2 vp;"
"layout(location = 1) in vec2 texCord;"
"out vec2 TexCord;"
"uniform mat4 mvp;"
"void main() {"
"  TexCord = texCord;"
"  gl_Position = mvp*vec4(vp, 0.0, 1.0);"
"}";

const char* fragmentShader =
"#version 400\n"
"in vec2 TexCord;"
"out vec4 frag_colour;"
"uniform sampler2D spriteSheet;"
"void main() {"
"  frag_colour = texture(spriteSheet, vec2(TexCord.x, 1.0-TexCord.y));"
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

GLState opengl_state_init(){
	//glEnable              ( GL_DEBUG_OUTPUT );
	//glDebugMessageCallback( MessageCallback, 0 );

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

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertexShader, NULL);
	glCompileShader(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragmentShader, NULL);
	glCompileShader(fs);

	state.shader = glCreateProgram();
	glAttachShader(state.shader, fs);
	glAttachShader(state.shader, vs);
	glLinkProgram(state.shader);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	state.mvpLocation = glGetUniformLocation(state.shader, "mvp");

	state.texXMul = 1.0;
	state.texYMul = 1.0;

	state.p = HMM_Orthographic(-1.0*16.0/9.0, 1.0*16.0/9.0, 1.0, -1.0, 0.0, 100.0);
	state.v = HMM_Scale(HMM_Vec3(1.0, 1.0, 1.0));

	glClearColor(1.0, 0.0, 1.0, 0.0);

	return state;
}

void opengl_render_list(RenderList *renderList, GLState state){
	r32 windowSizeData[4];
	glGetFloatv(GL_VIEWPORT, windowSizeData);
	r32 aspectRatio = windowSizeData[2]/windowSizeData[3];
	state.p = HMM_Orthographic(-1.0*aspectRatio, 1.0*aspectRatio, 1.0, -1.0, 0.0, 100.0);

	glUseProgram(state.shader);

	RLEntryHeader *header = renderList->first;
	while(header){
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
					hmm_m4 mvp = HMM_MultiplyMat4(state.p, HMM_MultiplyMat4(state.v, m));
					glUniformMatrix4fv(state.mvpLocation, 1, GL_FALSE, (GLfloat*)&mvp);

					float texturePoints[] = {
						0.0, 0.0,
						1.0, 0.0,
						1.0, 1.0,
						0.0, 0.0,
						1.0, 1.0,
						0.0, 1.0
					};
					for(int i = 0; i < 6; i++){
						texturePoints[i*2]   *= rlDrawSprite->spriteSize.X*state.texXMul;
						texturePoints[i*2+1] *= rlDrawSprite->spriteSize.Y*state.texYMul;
						texturePoints[i*2]   += rlDrawSprite->spritePos.X*state.texXMul;
						texturePoints[i*2+1] += rlDrawSprite->spritePos.Y*state.texYMul;
					}

					glBindBuffer(GL_ARRAY_BUFFER, state.textureVBO);
					glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), texturePoints, GL_STREAM_DRAW);

					glDrawArrays(GL_TRIANGLES, 0, 6);
					break;
				}
			default:
				break;
		}
		header = header->nextEntry;
	}
}

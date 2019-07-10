#include "opengl_renderer.h"
#include "render_list.h"

/*const float points[] = {
	-1.0, -1.0,
	+1.0, -1.0,
	+1.0, +1.0,
	-1.0, -1.0,
	+1.0, +1.0,
	-1.0, +1.0
};*/
const float points[] = {
	-1.0, -1.0, 0.0, 0.0,
	+1.0, -1.0, 1.0, 0.0,
	+1.0, +1.0, 1.0, 1.0,
	-1.0, -1.0, 0.0, 0.0,
	+1.0, +1.0, 1.0, 1.0,
	-1.0, +1.0, 0.0, 1.0
};

typedef struct{
	hmm_v2 position;
	hmm_v2 texture;
} __attribute__((packed)) Vertex;

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
"  frag_colour = texture(spriteSheet, TexCord);"
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return handle;
}

GLState opengl_state_init(){
	glEnable              ( GL_DEBUG_OUTPUT );
	glDebugMessageCallback( MessageCallback, 0 );

	GLState state;
	glGenBuffers(1, &(state.mainVBO));
	glBindBuffer(GL_ARRAY_BUFFER, state.mainVBO);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), points, GL_STATIC_DRAW);
	glGenVertexArrays(1, &(state.mainVAO));
	glBindVertexArray(state.mainVAO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,position));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,texture));

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

	state.mvpLocation = glGetUniformLocation(state.shader, "mvp");

	return state;
}

void opengl_render_list(MemoryArena *renderList, GLState state){
	glClearColor(1.0, 0.0, 0.0, 0.0);
	glUseProgram(state.shader);
	/*glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	oglUniformMatrix4fvglDrawArrays(GL_TRIANGLES, 0, 3);*/
	hmm_m4 mvp = HMM_Scale(HMM_Vec3(0.5, 0.5, 0.5));
	glUniformMatrix4fv(state.mvpLocation, 1, GL_FALSE, (GLfloat*)&mvp);

	u64 offset = 0;
	while(offset < renderList->used){
		u16 *entryType = (renderList->base+offset);
		offset += sizeof(u16);
		switch(*entryType){
			case RL_COLOR_CLEAR:
				{
				glClear(GL_COLOR_BUFFER_BIT);
				break;
				}
			case RL_DRAW_SPRITE:
				{
				RLDrawSprite *rlDrawSprite= (renderList->base+offset);
				offset += sizeof(RLDrawSprite);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				break;
				}
			default:
				break;
		}
	}
}

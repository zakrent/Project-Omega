#version 400
in vec2 Texture;
out vec4 color;
uniform sampler2D spriteSheet;
uniform float realTime;
void main() {
	color = texture(spriteSheet, vec2(Texture.x, Texture.y));
}

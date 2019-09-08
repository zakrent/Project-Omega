#version 400
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texture;
layout(location = 2) in float time;
out vec2 Position;
out vec2 Texture;
out float Time;
void main() {
  Texture = vec2(texture.x, texture.y);
  Position = position;
  Time = time;
  gl_Position = vec4(position.x, position.y, 0.0, 1.0);
}

#version 400
in vec2 Position;
in vec2 Texture;
out vec4 color;
uniform sampler2D spriteSheet;

float rand(vec2 co){
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

//2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise(in vec2 st) {
	vec2 i = floor(st);
	vec2 f = fract(st);

	// Four corners in 2D of a tile
	float a = rand(i);
	float b = rand(i + vec2(1.0, 0.0));
	float c = rand(i + vec2(0.0, 1.0));
	float d = rand(i + vec2(1.0, 1.0));

	// Smooth Interpolation

	// Cubic Hermine Curve.  Same as SmoothStep()
	vec2 u = f*f*(3.0-2.0*f);
	// u = smoothstep(0.,1.,f);

	// Mix 4 coorners percentages
	return mix(a, b, u.x) +
			(c - a)* u.y * (1.0 - u.x) +
			(d - b) * u.x * u.y;
}

void main() {
	float noiseVal = 1.0-0.25*noise(Position*2.0)-0.3*rand(Position);
	//color = vec4(noiseVal, noiseVal, noiseVal, 1.0);
	color = texture(spriteSheet, vec2(Texture.x, Texture.y))*vec4(noiseVal, noiseVal, noiseVal, 1.0);
}

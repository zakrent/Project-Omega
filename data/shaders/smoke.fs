#version 400
in vec2 Position;
in vec2 Texture;
in float Time;
out vec4 color;
uniform sampler2D spriteSheet;
uniform float realTime;

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
	float noiseVal = 0.8*noise(Position*10+vec2(0.0, -2.0*realTime))*noise(Position*15.0+vec2(0.0, -2.0*realTime))+0.2*rand(Position);
	float intensity = texture(spriteSheet, vec2(Texture.x, Texture.y)).a;
	float colorIntensity = mix(0.5, 1.0, noiseVal);
	color = intensity*vec4(0.5*colorIntensity, 0.5*colorIntensity, 0.5*colorIntensity, (1.0 - pow(Time/480.0, 2))*mix(0.5, 1.0, noiseVal));
	//color = vec4(noiseVal, noiseVal, noiseVal, 1.0);
}

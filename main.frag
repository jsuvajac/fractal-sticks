#version 150

uniform float zoom;
uniform float angle;
uniform vec2 offset;
uniform mat4 transform;

in vec4 pos;
// in float intensity;
out vec4 color;

void main() {    
	// color = vec4(1.0, 1.0, 1.0, 1.0);
	// color = vec4(1.0, pos.xy * vec2(2.0) * intensity, 1.0);
	// color = vec4(0.0, min(pos.x, intensity), intensity, 1.0);
	color = vec4( \
		0.0, \
		distance(pos.xy, vec2(0.0)), \
		pos.x/1000, \
		1.0);
}

#version 410

uniform float zoom;
uniform vec2 offset;
in vec4 position;
out vec4 pos;

void main() {
	pos = position * vec4(vec3(1.0), 1.0 / zoom) \
		+ vec4(offset / vec2(1000.0), 0.0, 0.0);
	gl_Position = pos;
}

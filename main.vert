#version 150

uniform float zoom;
uniform float angle;
uniform vec2 offset;
uniform mat4 transform;

in vec4 position;
out vec4 pos;

mat4 rotate2d(float _angle) {
	return mat4( \
		cos(_angle), -sin(_angle), 0.0, 0.0, \
		sin(_angle),  cos(_angle), 0.0, 0.0, \
		0.0, 0.0, 1.0, 0.0, \
		0.0, 0.0, 0.0, 1.0 \
	);
}

void main() {
	// rotate
	pos = rotate2d(angle) * position;
	// x-y offset
	pos.xy += offset;
	// zoom
	pos.w /= zoom;

	gl_Position = pos* transform;
	// intensity = position.w;
}

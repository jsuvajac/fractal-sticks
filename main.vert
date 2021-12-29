#version 150 core

uniform float zoom;
uniform float angle;
uniform vec2 offset;
uniform mat4 transform;
uniform int numVertices;

in vec4 position;
out vec4 pos;
out float intensity;

mat4 rotZ(float _angle) {
	return mat4( \
		cos(_angle), -sin(_angle), 0.0, 0.0, \
		sin(_angle),  cos(_angle), 0.0, 0.0, \
		0.0, 0.0, 1.0, 0.0, \
		0.0, 0.0, 0.0, 1.0 \
	);
}
mat4 rotY(float _angle) {
	return mat4( \
		cos(_angle), 0.0, sin(_angle), 0.0, \
		0.0, 1.0, 0.0, 0.0, \
		-sin(_angle), 0.0, cos(_angle), 0.0, \
		0.0, 0.0, 0.0, 1.0 \
	);
}
mat4 rotX(float _angle) {
	return mat4( \
		1.0, 0.0, 0.0, 0.0, \
		0.0, cos(_angle), -sin(_angle), 0.0, \
		0.0, sin(_angle),  cos(_angle), 0.0, \
		0.0, 0.0, 0.0, 1.0 \
	);
}

mat4 perspective(float near, float far) {
	return mat4( \
		near, 0.0, 0.0, 0.0, \
		0.0, near, 0.0, 0.0, \
		0.0, 0.0, (far + near), -(far*near), \
		0.0, 0.0, 0.0, 1.0 \
	);
}


void main() {
	// rotate
	// pos = rotX(angle* 0.1) * \
	// 	 rotY(angle * 0.1) * \
	// 	 rotZ(angle* 0.5) * \
	// 	 position;

	pos = rotX(angle* 0.3) * \
		 rotY(angle * 0.0) * \
		 rotZ(angle* 0.3) * \
		 position;


	// x-y offset
	pos.xy += offset;
	pos.z -= 5000.0;
	// zoom
	pos.w /= zoom;

	intensity = gl_VertexID;
	gl_Position = pos * transform;
}

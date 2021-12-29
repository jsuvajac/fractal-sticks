#version 150 core

uniform float zoom;
uniform float angle;
uniform vec2 offset;
uniform mat4 transform;
uniform int numVertices;

in vec4 pos;
in float intensity;
out vec4 color;

void main() {    
	// color = vec4(1.0, 1.0, 1.0, 1.0);
	// color = vec4(1.0, pos.xy  * intensity/numVertices, 1.0);

	color = vec4( \
		(numVertices - intensity)/numVertices, \
		0.1, \
		0.5 + intensity/numVertices /2, \
		(numVertices - intensity)/numVertices);

	// green
	// color = vec4( \
	// 	intensity/numVertices, \
	// 	distance(pos.xy, vec2(0.0)), \
	// 	pos.x/100, \
	// 	(numVertices - intensity)/numVertices);
		// 1.0);

	// blue
	// color = vec4(
	// 	0.001 * pos.x, \
	// 	0.001 * pos.y, \
	// 	0.8, \
	// 	intensity/numVertices * 2);
}

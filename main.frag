#version 410

in vec4 pos;
out vec4 color;

void main() {    
	color = vec4(0.4, pos.xy * vec2(2.0), 1.0);
}

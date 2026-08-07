#version 330 core

// texture's coord
in vec2 data0;

out vec4 color;

uniform sampler2D tex;

void main() {
	color = texture(tex, data0);
}

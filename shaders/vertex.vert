#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 indata0;
layout (location = 2) in vec2 indata1;
layout (location = 3) in vec2 indata2;

out vec2 data0;
out vec2 data1;
out vec2 data2;

uniform vec2 size_screen;

void main() {
	gl_Position = vec4(pos.x / size_screen.x * 2 - 1, -pos.y / size_screen.y * 2 + 1, 0.0f, 1.0f);

	data0 = indata0;
	data1 = indata1;
	data2 = indata2;
}

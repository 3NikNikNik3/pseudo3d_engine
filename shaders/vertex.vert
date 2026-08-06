#version 330 core

layout (location = 0) in vec2 pos;

uniform vec2 size_screen;

void main() {
	gl_Position = vec4(pos.x / size_screen.x * 2 - 1, -pos.y / size_screen.y * 2 + 1, 0.0f, 1.0f);
}

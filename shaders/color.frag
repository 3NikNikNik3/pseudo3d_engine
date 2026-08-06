#version 330 core

uniform vec4 color_;

out vec4 color;

void main() {
	color = color_ / 255;
}

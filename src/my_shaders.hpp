/*
	This file is needed, to place shaders in shared library
*/

#pragma once

// like color.frag
#define SHADER_COLOR R"(#version 330 core

uniform vec4 color_;

out vec4 color;

void main() {
	color = color_ / 255;
})"


// like image.frag
#define SHADER_IMAGE R"(#version 330 core

// texture's coord
in vec2 data0;

out vec4 color;

uniform sampler2D tex;

void main() {
	color = texture(tex, data0);
})"


// like floor.frag
#define SHADER_FLOOR R"(#version 330 core

// pos on world
in vec2 data0;
// a
in vec2 data1;
// s_old, s
in vec2 data2;
// now, zero
in vec2 data3;

out vec4 color;

uniform sampler2D tex;

void main() {
	vec2 pos = data0;
	float s_old = data2.x;

	if (data2.y < 1) {
		color = vec4(0, 0, 0, 1);
		return;
	}

	if (s_old < 1) {
		pos += data1 * (1 - s_old);
		s_old = 1;
	}

	color = texture(tex, pos + data1 * (s_old * data2.y / ((1 - data3.x) * s_old + data3.x * data2.y) - s_old));
})"


// like floor_inf.frag
#define SHADER_FLOOR_INF R"(#version 330 core

// pos on world
in vec2 data0;
// a
in vec2 data1;
// s_old, now
in vec2 data2;

out vec4 color;

uniform sampler2D tex;

void main() {
	vec2 pos = data0;
	float s_old = data2.x;

	if (s_old < 1) {
		pos += data1 * (1 - s_old);
		s_old = 1;
	}

	color = texture(tex, pos + data1 * (s_old / data2.y - s_old));
})"


// like vertex.vert
#define SHADER_VERTEX R"(#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 indata0;
layout (location = 2) in vec2 indata1;
layout (location = 3) in vec2 indata2;
layout (location = 4) in vec2 indata3;

out vec2 data0;
out vec2 data1;
out vec2 data2;
out vec2 data3;

uniform vec2 size_screen;

void main() {
	gl_Position = vec4(pos.x / size_screen.x * 2 - 1, -pos.y / size_screen.y * 2 + 1, 0.0f, 1.0f);

	data0 = indata0;
	data1 = indata1;
	data2 = indata2;
	data3 = indata3;
})"

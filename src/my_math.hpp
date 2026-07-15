#pragma once

#define uchar unsigned char

namespace pseudo3d_engine {
	namespace math {
		struct Vec2i {
			int x, y;
		};

		struct Vec2f {
			float x, y;

			static Vec2f from_a(const float a);
		};
	}
}

#pragma once

#define uchar unsigned char

#define EPS 0.000001f

namespace pseudo3d_engine {
	namespace math {
		struct Vec2i {
			int x, y;

			Vec2i operator+(Vec2i v);

			Vec2i operator-(Vec2i v);

			Vec2i operator-=(Vec2i v);
		};

		struct Vec2f {
			float x, y;

			Vec2f(float x, float y);

			explicit Vec2f(const Vec2i &v);

			static Vec2f from_a(const float a);

			Vec2f operator+(Vec2f v);

			Vec2f operator+=(Vec2f v);

			Vec2f operator-(Vec2f v);

			Vec2f operator/(float a);

			Vec2f operator*(float a);
		};

		float len(Vec2f v);

		Vec2f norm(Vec2f v);

		Vec2f rotation(Vec2f v, float a);

		float cross(Vec2f v, Vec2f w);

		float dot(Vec2f v, Vec2f w);
	}
}

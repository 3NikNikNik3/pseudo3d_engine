#include "my_math.hpp"

#include <cmath>

namespace pseudo3d_engine {
	namespace math {
		// Vec2i
		Vec2i Vec2i::operator+(Vec2i v) {
			return {x + v.x, y + v.y};
		}

		Vec2i Vec2i::operator-(Vec2i v) {
			return {x - v.x, y - v.y};
		}

		Vec2i Vec2i::operator-=(Vec2i v) {
			x -= v.x;
			y -= v.y;
			return *this;
		}

		// Vec2f
		Vec2f::Vec2f(float x, float y) : x(x), y(y) {}

		Vec2f::Vec2f(const Vec2i &v) : x(v.x), y(v.y) {}

		Vec2f Vec2f::from_a(const float a) {
			return {(float)cos(a), (float)sin(a)};
		}

		Vec2f Vec2f::operator+(Vec2f v) {
			return {x + v.x, y + v.y};
		}

		Vec2f Vec2f::operator+=(Vec2f v) {
			x += v.x;
			y += v.y;
			return {x, y};
		}

		Vec2f Vec2f::operator-(Vec2f v) {
			return {x - v.x, y - v.y};
		}

		Vec2f Vec2f::operator/(float a) {
			if (a == 0)
				return {0, 0};
			return {x / a, y / a};
		}

		Vec2f Vec2f::operator*(float a) {
			return {x * a, y * a};
		}

		float len(Vec2f v) {
			return sqrt(v.x * v.x + v.y * v.y);
		}

		Vec2f norm(Vec2f v) {
			return v / len(v);
		}

		Vec2f rotation(Vec2f v, float a) {
			return {(float)(v.x * cos(a) + v.y * sin(a)),
				(float)(-v.x * sin(a) + v.y * cos(a))};
		}

		float cross(Vec2f v, Vec2f w) {
			return v.x * w.y - v.y * w.x;
		}
	}
}

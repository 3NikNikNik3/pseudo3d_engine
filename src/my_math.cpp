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

		// Vec2f
		Vec2f::Vec2f(float x, float y) : x(x), y(y) {}

		Vec2f::Vec2f(const Vec2i &v) : x(v.x), y(v.y) {}

		Vec2f Vec2f::from_a(const float a) {
			return {(float)cos(a), (float)sin(a)};
		}

		Vec2f Vec2f::operator+(Vec2f v) {
			return {x + v.x, y + v.y};
		}

		Vec2f Vec2f::operator-(Vec2f v) {
			return {x - v.x, y - v.y};
		}
	}
}

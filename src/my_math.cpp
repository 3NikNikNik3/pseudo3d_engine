#include "my_math.hpp"

#include <cmath>

namespace pseudo3d_engine {
	namespace math {
		Vec2f Vec2f::from_a(const float a) {
			return {(float)cos(a), (float)sin(a)};
		}
	}
}

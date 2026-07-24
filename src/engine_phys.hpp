#pragma once

#include "engine_structs.hpp"

namespace pseudo3d_engine {
	bool what_see(Universe &uni, uchar id_world, math::Vec2f from, math::Vec2f a, std::uint16_t *id_wall, float *s, float *t);
}

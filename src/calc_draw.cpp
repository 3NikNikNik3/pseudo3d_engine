#include "calc_draw.hpp"

namespace pseudo3d_engine {
	namespace calc {
		bool interact_with_wall(Wall &wall, math::Vec2f from, math::Vec2f a, float &s, float &t) {
			if (-EPS < wall.a.x && wall.a.x < EPS) { // wall.a.x == 0
				if (-EPS < a.x && a.x < EPS) // a.x == 0
					return false;
				else {
					const math::Vec2f delta = from - (math::Vec2f)wall.from;

					s = -delta.x / a.x;
					t = delta.y / wall.a.y - (delta.x * a.y) / (a.x * wall.a.y);

					return 0 <= t && t <= 1 && s >= 0; //!binery add t_start, t_end
				}
			} else {
				const float tmp = a.y * wall.a.x - a.x * wall.a.y;
				if (-EPS < tmp && tmp < EPS) // tmp == 0
					return false;
				// else

				const math::Vec2f delta = from - (math::Vec2f)wall.from;

				s = (delta.x * wall.a.y - delta.y * wall.a.x) / tmp;
				t = delta.x / wall.a.x + s;

				return 0 <= t && t <= 1 && s >= 0; //!binery add t_start, t_end
			}
		}

		void draw_line(draw::Window &window, Universe &uni, uchar id_world, math::Vec2f from, math::Vec2f a, int x, int y, int size_y) {
			//!binery add normal search
			float s_min = 100, s, t;
			Wall *ans = nullptr;

			for (int i = 0; i < uni.worlds[id_world].walls_size_now; ++i)
				if (interact_with_wall(uni.worlds[id_world].walls[i], from, a, s, t))
					if (s < s_min) {
						s_min = s;
						ans = &uni.worlds[id_world].walls[i];
					}

			//!binery add normal switch
			if (ans != nullptr) {
				draw::draw_line(window, x, (int)(size_y / 2.0 * (1 - 1 / s_min)), (int)(size_y / s_min), ans->r, ans->g, ans->b);
			}
		}
	}
}

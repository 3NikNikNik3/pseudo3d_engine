#include "engine_structs.hpp"

#include "calc_draw.hpp"

#include <cstdlib>

namespace pseudo3d_engine {
	// World
	World::World(std::uint16_t size) : walls_size(size), node_size(0), wall_ptr_size(0), nodes(nullptr), pwalls(nullptr) {
		walls = (Wall*)std::malloc(sizeof(Wall) * size);
		if (walls == nullptr)
			throw "little memory";
	}

	World::~World() {
		std::free(walls);
		std::free(nodes);
		std::free(pwalls);
	}

	void World::resize_walls(std::uint16_t size) {
		if (size <= walls_size)
			return;
		void *ptr = std::realloc(walls, sizeof(Wall) * size);
		if (ptr == nullptr)
			throw "little memory";
		walls = (Wall*)ptr;

		walls_size = size;
	}

	void World::resize_nodes(std::uint16_t size) {
		if (size < node_size) return;

		if (nodes == nullptr)
			nodes = (node*)std::malloc(sizeof(node) * size);
		else {
			void *ptr = std::realloc(nodes, sizeof(node) * size);
			if (ptr == nullptr)
				throw "little memory";
			nodes = (node*)ptr;
		}

		node_size = size;
	}

	void World::resize_pwalls(std::uint16_t size) {
		if (size < wall_ptr_size) return;

		if (pwalls == nullptr)
			pwalls = (wall_ptr*)std::malloc((sizeof(wall_ptr) * size) << 2);
		else {
			void *ptr = std::realloc(pwalls, (sizeof(wall_ptr) * size) << 2);
			if (ptr == nullptr)
				throw "little memory";
			pwalls = (wall_ptr*)ptr;
		}

		wall_ptr_size = size;
	}

	std::uint16_t get_size(World &world, std::uint16_t id, std::uint16_t s) {
		std::uint16_t sl = s, sr = s;

		if (!(world.nodes[id].left & 0x8000))
			sl = get_size(world, world.nodes[id].left, s + 1);
		if (!(world.nodes[id].right & 0x8000))
			sr = get_size(world, world.nodes[id].right, s + 1);

		if (sl < sr)
			return sr;
		return sl;
	}

	std::uint16_t World::get_size_tree() {
		if (tree_size == 0)
			tree_size = get_size(*this, 0, 1);
		return tree_size;
	}

	// for World::make_tree
	struct list_wall {
		std::uint16_t id_wall, id_next;
		float t_start, t_end;
		bool flag;
	};

	struct node_calc_tree {
		union {
			struct { // not ready
				std::uint16_t len_can, len, id_list;
			};

			struct { // ready
				std::uint16_t id_wall, id_left, id_right;
			};
		};

		// 0 - not ready, 1 - ready
		uchar type;
	};

	void* get_new(void *arr, std::uint16_t &size_real, std::uint16_t &size_now, size_t size, std::uint16_t &res) {
		if (size_real == size_now) {
			size_real <<= 1;
			void *ptr = std::realloc(arr, size_real * size);
			if (ptr == nullptr)
				throw "little memory";
			arr = ptr;
		}

		res = size_now++;
		return arr;
	}

	void add_lwall(node_calc_tree &node, list_wall *lwalls, std::uint16_t id) {
		if (node.type == 2) {
			node.type = 0;
			node.id_list = id;
		} else {
			lwalls[id].id_next = node.id_list;
			node.id_list = id;
		}
		++node.len;
	}

	void make_tree_real(list_wall *&lwalls, std::uint16_t &lwalls_size_real, std::uint16_t &lwalls_size_now, Wall *walls, node_calc_tree *&arr, std::uint16_t id, std::uint16_t &arr_size_now, std::uint16_t &arr_size_real, std::uint16_t back = 0, bool back_use = false) {
		if (arr[id].len <= 4) return;

		std::uint16_t buf = arr[id].id_list, size_buf = 1, chose = rand() % arr[id].len_can;

		// chose middle
		while (lwalls[buf].flag)
			buf = lwalls[buf].id_next;

		for (int i = 0; i < chose;) {
			if (!lwalls[buf].flag)
				++i;
			buf = lwalls[buf].id_next;
		}

		Wall *middle = &walls[lwalls[buf].id_wall];
		const std::uint16_t id_middle = lwalls[buf].id_wall;

		// move walls
		node_calc_tree left, right;
		left.len = left.len_can = right.len = right.len_can = 0;
		left.type = right.type = 2;

		std::uint16_t now = arr[id].id_list;
		for (int i = 0; i < arr[id].len; ++i) {
			std::uint16_t next = lwalls[now].id_next;

			if (lwalls[now].id_wall != id_middle) {
				float res_start = math::cross((math::Vec2f)middle->a, (math::Vec2f)walls[lwalls[now].id_wall].from + (math::Vec2f)walls[lwalls[now].id_wall].a * lwalls[now].t_start - (math::Vec2f)middle->from);
				float res_end = math::cross((math::Vec2f)middle->a, (math::Vec2f)walls[lwalls[now].id_wall].from + (math::Vec2f)walls[lwalls[now].id_wall].a * lwalls[now].t_end - (math::Vec2f)middle->from);

				if (!res_start && !res_end) {
					lwalls[now].id_next = buf;
					buf = now;
					++size_buf;
				} else if (res_start >= 0 && res_end >= 0) {
					add_lwall(left, lwalls, now);
					++left.len_can;
				} else if (res_start <= 0 && res_start <= 0) {
					add_lwall(right, lwalls, now);
					++right.len_can;
				} else { // cut
					float s, t;
					calc::interact_with_wall(walls[lwalls[now].id_wall], 0, 1, (math::Vec2f)middle->from, (math::Vec2f)middle->a, s, t);

					// new wall
					std::uint16_t new_lwall;
					lwalls = (list_wall*)get_new(lwalls, lwalls_size_real, lwalls_size_now, sizeof(lwalls), new_lwall);

					lwalls[new_lwall].id_wall = lwalls[now].id_wall;
					lwalls[new_lwall].t_end = lwalls[now].t_end;
					lwalls[new_lwall].t_start = t;
					lwalls[new_lwall].flag = lwalls[now].flag;

					// old wall
					lwalls[now].t_end = t;

					if (res_start < 0) {
						add_lwall(left, lwalls, new_lwall);
						++left.len_can;

						add_lwall(right, lwalls, now);
						++right.len_can;
					} else {
						add_lwall(right, lwalls, new_lwall);
						++right.len_can;

						add_lwall(left, lwalls, now);
						++left.len_can;
					}
				}
			}

			now = next;
		}

		// buf
		for (int i = 0; i < size_buf; ++i) {
			std::uint16_t next = lwalls[buf].id_next;

			lwalls[buf].flag = true;

			if (left.len <= 3)
				add_lwall(left, lwalls, buf);
			else if (right.len <= 3)
				add_lwall(right, lwalls, buf);
			else if (left.len == 4)
				add_lwall(right, lwalls, buf);
			else if (right.len == 4)
				add_lwall(left, lwalls, buf);
			else if (left.len > right.len)
				add_lwall(right, lwalls, buf);
			else
				add_lwall(left, lwalls, buf);

			buf = next;
		}

		// save
		arr[id].type = 1;
		arr[id].id_wall = id_middle;

		// save left
		std::uint16_t id_left;
		arr = (node_calc_tree*)get_new(arr, arr_size_real, arr_size_now, sizeof(node_calc_tree), id_left);
		arr[id].id_left = id_left;
		arr[id_left] = left;

		// save right
		std::uint16_t id_right;
		arr = (node_calc_tree*)get_new(arr, arr_size_real, arr_size_now, sizeof(node_calc_tree), id_right);
		arr[id].id_right = id_right;
		arr[id_right] = right;

		if (left.len <= 3 || right.len <= 3) {
			make_tree_real(lwalls, lwalls_size_real, lwalls_size_now, walls, arr, id_left, arr_size_now, arr_size_real, id, true);
			make_tree_real(lwalls, lwalls_size_real, lwalls_size_now, walls, arr, id_right, arr_size_now, arr_size_real, id, true);
		} else {
			make_tree_real(lwalls, lwalls_size_real, lwalls_size_now, walls, arr, id_left, arr_size_now, arr_size_real);
			make_tree_real(lwalls, lwalls_size_real, lwalls_size_now, walls, arr, id_right, arr_size_now, arr_size_real);
		}
	}

	void calc_count_make_tree(node_calc_tree *arr, std::uint16_t &count_node, std::uint16_t &count_array, std::uint16_t id = 0) {
		if (!arr[id].type) {
			++count_array;
			return;
		}

		++count_node;

		calc_count_make_tree(arr, count_node, count_array, arr[id].id_left);
		calc_count_make_tree(arr, count_node, count_array, arr[id].id_right);
	}

	void save_tree(node *nodes, std::uint16_t &nodes_last, std::uint16_t id_ans, wall_ptr *pwalls, std::uint16_t &pwalls_last, node_calc_tree *arr, std::uint16_t id, list_wall *lwalls) {
		nodes[id_ans].id_wall = arr[id].id_wall;

		// left
		std::uint16_t id_left = arr[id].id_left;
		if (arr[id_left].type) {
			nodes[id_ans].left = nodes_last;
			++nodes_last;
			save_tree(nodes, nodes_last, nodes_last - 1, pwalls, pwalls_last, arr, id_left, lwalls);
		} else {
			nodes[id_ans].left = 0x8000 | pwalls_last;

			int i = 0;
			for (std::uint16_t now = arr[id_left].id_list; i < arr[id_left].len; ++i) {
				pwalls[pwalls_last * 4 + i].t_start = lwalls[now].t_start;
				pwalls[pwalls_last * 4 + i].t_end = lwalls[now].t_end;
				pwalls[pwalls_last * 4 + i].id_wall = lwalls[now].id_wall;

				now = lwalls[now].id_next;
			}

			if (i != 4)
				pwalls[pwalls_last * 4 + i].t_end = 0;

			++pwalls_last;
		}

		// right
		std::uint16_t id_right = arr[id].id_right;
		if (arr[id_right].type) {
			nodes[id_ans].right = nodes_last;
			++nodes_last;
			save_tree(nodes, nodes_last, nodes_last - 1, pwalls, pwalls_last, arr, id_right, lwalls);
		} else {
			nodes[id_ans].right = 0x8000 | pwalls_last;

			int i = 0;
			for (std::uint16_t now = arr[id_right].id_list; i < arr[id_right].len; ++i) {
				pwalls[pwalls_last * 4 + i].t_start = lwalls[now].t_start;
				pwalls[pwalls_last * 4 + i].t_end = lwalls[now].t_end;
				pwalls[pwalls_last * 4 + i].id_wall = lwalls[now].id_wall;

				now = lwalls[now].id_next;
			}

			if (i != 4)
				pwalls[pwalls_last * 4 + i].t_end = 0;

			++pwalls_last;
		}
	}

	void World::make_tree() {
		if (pwalls != nullptr) return;

		// reading

		std::uint16_t lwalls_size_real = walls_size, lwalls_size_now = walls_size;
		list_wall *lwalls = (list_wall*)std::malloc(walls_size * sizeof(list_wall));
		if (lwalls == nullptr)
			throw "little memory";

		for (int i = 0; i < walls_size; ++i) {
			lwalls[i].id_wall = i;
			lwalls[i].id_next = i + 1;
			lwalls[i].t_start = 0.0f;
			lwalls[i].t_end = 1.0f;
			lwalls[i].flag = 0;
		}

		std::uint16_t arr_size_real = walls_size, arr_size_now = 1;
		node_calc_tree *arr = (node_calc_tree*)std::malloc(walls_size * sizeof(node_calc_tree));
		if (arr == nullptr)
			throw "little memory";

		arr[0].id_list = 0;
		arr[0].len = arr[0].len_can = walls_size;
		arr[0].type = 0;

		// go
		make_tree_real(lwalls, lwalls_size_real, lwalls_size_now, walls, arr, 0, arr_size_now, arr_size_real);

		if (!arr[0].type) {
			std::uint16_t id;
			arr = (node_calc_tree*)get_new(arr, arr_size_real, arr_size_now, sizeof(node_calc_tree), id);

			arr[id].len_can = arr[0].len_can;
			arr[id].len = arr[0].len;
			arr[id].id_list = arr[0].id_list;
			arr[id].type = 0;

			std::uint16_t id_;
			arr = (node_calc_tree*)get_new(arr, arr_size_real, arr_size_now, sizeof(node_calc_tree), id_);

			arr[id_].len_can = 0;
			arr[id_].len = 0;
			arr[id_].id_list = 0;
			arr[id_].type = 0;

			arr[0].id_wall = 0;
			arr[0].id_left = id;
			arr[0].id_right = id_;
			arr[0].type = 1;
		}

		/*// debug info
		std::cout << arr_size_now << ':' << std::endl;
		for (int i = 0; i < arr_size_now; ++i) {
			std::cout << i;
			if (arr[i].type == 0) {
				std::cout << ' ' << arr[i].len << '(' << arr[i].len_can << "): ";
				std::uint16_t now = arr[i].id_list;
				for (int j = 0; j < arr[i].len; ++j) {
					std::cout << lwalls[now].id_wall << ' ' << lwalls[now].t_start << ' ' << lwalls[now].t_end << ',';
					now = lwalls[now].id_next;
				}
				std::cout << std::endl;
			} else {
				std::cout << " wall " << arr[i].id_wall << ' ' << arr[i].id_left << '-' << arr[i].id_right << std::endl;
			}
		}*/

		// save res
		std::uint16_t count_node = 0, count_array = 0;
		calc_count_make_tree(arr, count_node, count_array);

		resize_nodes(count_node);
		resize_pwalls(count_array);

		count_node = 1;
		count_array = 0;
		save_tree(nodes, count_node, 0, pwalls, count_array, arr, 0, lwalls);

		std::free(arr);
		std::free(lwalls);
	}

	// Universe
	Universe::Universe() : worlds(nullptr), size_worlds(0) { }

	Universe::~Universe() {
		delete[] worlds;
	}

	void Universe::add_world(std::uint16_t size) {
		if (worlds == nullptr) {
			size_worlds = 1;
			worlds = new World[1]{size};
		} else {
			void *ptr = std::realloc(worlds, sizeof(World) * (size_worlds + 1));
			if (ptr == nullptr)
				throw "little memory";
			worlds = (World*)ptr;

			worlds[size_worlds++] = World(size);
		}
	}
}

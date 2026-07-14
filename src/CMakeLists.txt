cmake_minimum_required(VERSION 3.10)

project(src)

find_package(SFML 3.0 COMPONENTS Graphics)

add_library(src calc_draw.cpp
		calc_draw.hpp
		calc_phys.cpp
		calc_phys.hpp
		draw.cpp
		draw.hpp
		engine_structs.cpp
		engine_structs.hpp
		engine_functions.cpp
		engine_functions.hpp
		my_math.cpp
		my_math.hpp)

target_link_libraries(src SFML::Graphics SFML::Window)

cmake_minimum_required(VERSION 3.10)

project(src)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_library(src calc_draw.cpp
		draw.cpp
		engine_structs.cpp
		engine_functions.cpp
		engine_phys.cpp
		my_math.cpp)

target_link_libraries(src OpenGL::GL GLEW::GLEW)

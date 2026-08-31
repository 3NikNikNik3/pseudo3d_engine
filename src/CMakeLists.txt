cmake_minimum_required(VERSION 3.10)

project(src)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_library(pseudo3d_engine ${TYPE_LIB}
		calc_draw.cpp
		draw.cpp
		engine_structs.cpp
		engine_functions.cpp
		engine_phys.cpp
		math.cpp)

target_link_libraries(pseudo3d_engine OpenGL::GL GLEW::GLEW)

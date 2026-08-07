cmake_minimum_required(VERSION 3.10)
project(pseudo3d_engine LANGUAGES CXX)

# include stb_image.h
file(DOWNLOAD
	https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
	src/stb_image.h
	SHOW_PROGRESS)

add_subdirectory(src)

if (CMAKE_BUILD_TYPE STREQUAL "DEBUG")
	add_executable(test_pseudo3d src/main_debug.cpp)

	target_link_libraries(test_pseudo3d src)
else()
	add_library(pseudo3d_engine SHARED src/main.cpp)

        target_link_libraries(pseudo3d_engine src)
endif()

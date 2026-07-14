cmake_minimum_required(VERSION 3.10)
project(pseudo3d_engine LANGUAGES CXX)

add_subdirectory(src)

if (CMAKE_BUILD_TYPE STREQUAL "DEBUG")
	add_executable(test_pseudo3d src/main.cpp)

	target_link_libraries(test_pseudo3d src)
else()
	add_library(pseudo3d_engine SHARED)

        target_link_libraries(pseudo3d_engine src)
endif()

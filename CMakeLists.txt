cmake_minimum_required(VERSION 3.10)
project(pseudo3d_engine LANGUAGES CXX)

# include stb_image.h
file(DOWNLOAD
	https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
	src/stb_image.h
	SHOW_PROGRESS)

set(TYPE_LIB STATIC)

if (CMAKE_BUILD_TYPE STREQUAL "RELEASE")
	set(TYPE_LIB SHARED)
endif()

add_subdirectory(src)

if (CMAKE_BUILD_TYPE STREQUAL "DEBUG")
	find_package(SFML 3.0 COMPONENTS Graphics)

	add_executable(test_pseudo3d src/main_debug.cpp)

	target_link_libraries(test_pseudo3d pseudo3d_engine SFML::Window SFML::Graphics)
else()
	include(CMakePackageConfigHelpers)

	install(
		TARGETS pseudo3d_engine
		EXPORT pseudo3d_engine_targets
		LIBRARY DESTINATION lib
		INCLUDES DESTINATION include
	)

	install(
		FILES
			src/engine_structs.hpp
			src/engine_functions.hpp
			src/engine_phys.hpp
			src/math.hpp
		DESTINATION include/pseudo3d_engine
	)

	install(
		EXPORT pseudo3d_engine_targets
		FILE pseudo3d_engine_targets.cmake
		NAMESPACE pseudo3d::
		DESTINATION lib/cmake/pseudo3d_engine
	)

	configure_package_config_file(
		CMakeConfig.cmake.in
		${CMAKE_CURRENT_BINARY_DIR}/pseudo3d_engineConfig.cmake
#		INSTALL_DESTINATION lib/cmake/pseudo3d_engine
		INSTALL_DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
	)

	install(
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/pseudo3d_engineConfig.cmake
		DESTINATION lib/cmake/pseudo3d_engine
	)
endif()

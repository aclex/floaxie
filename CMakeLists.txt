project(floaxie)

cmake_minimum_required(VERSION 2.8.11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/common/Modules/")

add_definitions(-std=c++11)

add_definitions(-Wall)
add_definitions(-Wextra)

include_directories(include)

add_subdirectory(example)
add_subdirectory(src)

if (BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
endif (BUILD_TESTS)

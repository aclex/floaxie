cmake_minimum_required(VERSION 3.13)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/common/Modules/")

include(GitVersion)

git_version(PROJECT_VERSION)

project(floaxie
	VERSION	${PROJECT_VERSION}
	DESCRIPTION "Library for printing and parsing floating point values"
	HOMEPAGE_URL "https://github.com/aclex/floaxie"
	LANGUAGES CXX
	)


add_library(floaxie INTERFACE)
target_include_directories(floaxie INTERFACE include)

target_compile_features(floaxie INTERFACE cxx_std_14)

if (BUILD_EXAMPLES)
	add_subdirectory(example)
endif ()

if (CALC_COVERAGE AND (CMAKE_BUILD_TYPE STREQUAL "Debug") AND CMAKE_COMPILER_IS_GNUCXX)
	set(COVERAGE_ENABLED TRUE)
endif()

if (BUILD_TESTS)
	if (COVERAGE_ENABLED)
		message("Will calculate code coverage.")
		target_compile_options(floaxie INTERFACE --coverage)
		target_link_options(floaxie INTERFACE --coverage)
	endif()

	enable_testing()
	add_subdirectory(test)

endif (BUILD_TESTS)

if (BUILD_DOCUMENTATION)
	add_subdirectory(doc)
endif (BUILD_DOCUMENTATION)

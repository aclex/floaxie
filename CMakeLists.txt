
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
		add_definitions(-g -O0 --coverage)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage -lgcov")
	endif()

	enable_testing()
	add_subdirectory(test)

endif (BUILD_TESTS)

if (BUILD_DOCUMENTATION)
	find_package(Doxygen REQUIRED)
	set(doxyfile_in ${CMAKE_SOURCE_DIR}/doc/Doxyfile)
	set(doxyfile ${CMAKE_BINARY_DIR}/doc/Doxyfile)

	configure_file(${doxyfile_in} ${doxyfile} @ONLY)

	add_custom_target(doc
                      COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
                      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
                      COMMENT "Generating API documentation with Doxygen"
                      VERBATIM)

endif (BUILD_DOCUMENTATION)

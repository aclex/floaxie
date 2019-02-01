project(floaxie)

cmake_minimum_required(VERSION 3.13)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/common/Modules/")

add_library(floaxie INTERFACE)
target_include_directories(floaxie INTERFACE include/)

target_compile_features(floaxie INTERFACE cxx_std_14)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(floaxie INTERFACE -Wall)
	target_compile_options(floaxie INTERFACE -Wextra)
	target_compile_options(floaxie INTERFACE -Wpedantic)
	target_compile_options(floaxie INTERFACE -Werror)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	target_compile_options(floaxie INTERFACE -Wall)
	target_compile_options(floaxie INTERFACE -Wextra)
	target_compile_options(floaxie INTERFACE -pedantic)
	target_compile_options(floaxie INTERFACE -Werror)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options(floaxie INTERFACE "/W4 /wd4244 /wd4805 /wd4389 /wd4267 /WX")
endif ()

add_subdirectory(example)

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

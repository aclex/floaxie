project(floaxie)

cmake_minimum_required(VERSION 2.8.11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/common/Modules/")

add_definitions(-std=c++14)

add_definitions(-Wall)
add_definitions(-Wextra)

include_directories(include)

add_subdirectory(example)

if (BUILD_TESTS)
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

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

include(GNUInstallDirs)
include(AddUninstallTarget)

add_library(floaxie INTERFACE)
target_include_directories(floaxie INTERFACE
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>)

target_compile_features(floaxie INTERFACE cxx_std_14)

include(CMakePackageConfigHelpers)
set(CONFIG_PACKAGE_INSTALL_DIR lib/cmake/${PROJECT_NAME})

write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
	)

install(TARGETS floaxie
	EXPORT floaxie-targets
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

configure_package_config_file(
	${PROJECT_SOURCE_DIR}/cmake/config.cmake.in
	${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
	INSTALL_DESTINATION ${CONFIG_PACKAGE_INSTALL_DIR}
	PATH_VARS CMAKE_INSTALL_FULL_INCLUDEDIR
	)

install(EXPORT ${PROJECT_NAME}-targets DESTINATION ${CONFIG_PACKAGE_INSTALL_DIR})
install(FILES
	${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
	${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
	DESTINATION ${CONFIG_PACKAGE_INSTALL_DIR}
	)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/floaxie DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

add_uninstall_target()

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

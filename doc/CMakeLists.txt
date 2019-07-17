find_package(Doxygen REQUIRED)
set(doxyfile_in ${PROJECT_SOURCE_DIR}/doc/Doxyfile)
set(doxyfile ${PROJECT_BINARY_DIR}/doc/Doxyfile)

configure_file(${doxyfile_in} ${doxyfile} @ONLY)

add_custom_target(doc ALL
				  COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
				  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/doc
				  COMMENT "Generating API documentation with Doxygen"
				  VERBATIM)

install(DIRECTORY ${PROJECT_BINARY_DIR}/doc/html DESTINATION ${CMAKE_INSTALL_DOCDIR})

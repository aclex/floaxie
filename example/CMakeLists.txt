include(ApplyStrictCompileOptions)

apply_strict_compile_options_to_this_directory()

add_executable(ftoa ftoa_main.cpp)
target_link_libraries(ftoa floaxie)

add_executable(ftoa_profile ftoa_profile.cpp)
target_link_libraries(ftoa_profile floaxie)

add_executable(atof atof_main.cpp)
target_link_libraries(atof floaxie)

cmake_minimum_required(VERSION 3.21.2)
project(main VERSION 0.1.0)

add_subdirectory(meta)

add_executable(main)
reflect(${CMAKE_CURRENT_SOURCE_DIR}/main.cpp main)
target_compile_features(main PRIVATE cxx_std_20) 
target_sources(main PRIVATE main.cpp)

target_link_libraries(main PRIVATE meta)
target_include_directories(main PRIVATE meta)

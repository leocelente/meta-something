include(FetchContent)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(LibClang REQUIRED)


add_executable(tool tool/tool.cpp tool/tool.hpp)
target_link_libraries(tool PRIVATE ${CLANG_LIBS})
target_include_directories(tool PRIVATE ${CLANG_INCLUDEDIR})

macro(reflect FILE)
set(tool_exec ${CMAKE_CURRENT_BINARY_DIR}/meta/tool)
add_custom_command(
  OUTPUT "main.gen.cpp"
  COMMENT "Generating C++ MicroKernel"
  COMMAND ${tool_exec} ${FILE} > main.gen.cpp
  DEPENDS tool 
  VERBATIM
)

add_library(meta main.gen.cpp)
add_dependencies(meta tool)
target_include_directories(meta PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
endmacro()

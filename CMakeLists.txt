cmake_minimum_required(VERSION 3.15)

project(TetrisProject C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(SDL2_LIB_DIR C:/DEVSOFT/SDL2/lib)
set(SDL2_INCLUDE_DIR C:/DEVSOFT/SDL2/include)
set(SDL2_BIN C:/DEVSOFT/SDL2/bin)

set(TESTS_DIR ${CMAKE_SOURCE_DIR}/tests)

include_directories(${SDL2_INCLUDE_DIR})
link_directories(${SDL2_LIB_DIR})

add_executable(tetris 
    src/tetris.c
    src/main.c
)

target_link_libraries(tetris 
    mingw32 
    SDL2main 
    SDL2 
)

add_custom_target(format
    COMMENT "Formatting source files"
    COMMAND clang-format.exe -i ${CMAKE_SOURCE_DIR}/src/main.c -style=microsoft
)
# add_dependencies(tetris format)


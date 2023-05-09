cmake_minimum_required(VERSION 3.25.2)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall  -Wextra -pedantic")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(stego VERSION 0.1)

add_subdirectory(src)

add_custom_target(run
    COMMAND ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)

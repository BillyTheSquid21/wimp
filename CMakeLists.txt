cmake_minimum_required(VERSION 3.5)
project(WIMP C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/library/${CMAKE_BUILD_TYPE})

add_subdirectory(wimp)
add_subdirectory(client)
add_subdirectory(server)
cmake_minimum_required(VERSION 3.5)
project(dependencies)

set(DEPENDENCIES_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(PLIBSYS_TESTS FALSE)
set(PLIBSYS_BUILD_DOC FALSE)
set(PLIBSYS_BUILD_STATIC OFF)

add_subdirectory(plibsys)
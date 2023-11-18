cmake_minimum_required(VERSION 3.5)

set(WIMP_SOURCE_FILES socket.c socket.h)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${PROJECT_NAME} STATIC ${WIMP_SOURCE_FILES})
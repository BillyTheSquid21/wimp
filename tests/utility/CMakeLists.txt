cmake_minimum_required(VERSION 3.5)

set(WIMP_SOURCE_FILES wimp_test.h wimp_test.c)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${PROJECT_NAME} STATIC ${WIMP_SOURCE_FILES})
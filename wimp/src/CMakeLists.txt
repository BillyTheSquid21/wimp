cmake_minimum_required(VERSION 3.5)

set(WIMP_SOURCE_FILES uni_socket.c uni_socket.h)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${PROJECT_NAME} STATIC ${WIMP_SOURCE_FILES})

if (WIN32)
	target_link_libraries(${PROJECT_NAME} ws2_32)
endif()
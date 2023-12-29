cmake_minimum_required(VERSION 3.5)

set(WIMP_SOURCE_FILES wimp_reciever.c wimp_reciever.h wimp_process.h wimp_process.c wimp_process_table.h wimp_process_table.c wimp_server.h wimp_server.c utility/HashString.h utility/HashString.c)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${PROJECT_NAME} STATIC ${WIMP_SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${DEPENDENCIES_DIRECTORY}/plibsys/src)

if (WIN32)
	target_link_libraries(${PROJECT_NAME} ws2_32)
endif()

add_dependencies(${PROJECT_NAME} plibsys)
target_link_libraries(${PROJECT_NAME} plibsys)
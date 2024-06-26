cmake_minimum_required(VERSION 3.5)

add_definitions(-DWIMP_EXPORTS)

set(WIMP_SOURCE_FILES wimp_core.h wimp_reciever.c wimp_reciever.h wimp_process.h wimp_process.c wimp_process_table.h wimp_process_table.c wimp_server.h wimp_server.c wimp_instruction.h wimp_instruction.c wimp_debug.h wimp_log.h wimp_log.c wimp_data.h wimp_data.c utility/HashString.h utility/HashString.c utility/thread_local.h utility/sds.h utility/sds.c utility/sdsalloc.h utility/simple_arena.h utility/simple_arena.c)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${PROJECT_NAME} SHARED ${WIMP_SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "wimp"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}"
)

target_include_directories(${PROJECT_NAME} PRIVATE ${DEPENDENCIES_DIRECTORY}/plibsys/src)

if (WIN32)
	target_link_libraries(${PROJECT_NAME} ws2_32)
endif()

add_dependencies(${PROJECT_NAME} plibsys)
target_link_libraries(${PROJECT_NAME} plibsys)
cmake_minimum_required(VERSION 3.5)
project(WIMP-Test-01)

add_executable(${PROJECT_NAME} 1_SEND_RECIEVE_LOOP.c)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/wimp/src ${CMAKE_SOURCE_DIR}/tests/utility)

add_dependencies(${PROJECT_NAME} wimp WIMP-Test)
target_link_libraries(${PROJECT_NAME} wimp WIMP-Test)

# Set the linking options
if (WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE "-shared")
endif()
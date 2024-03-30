cmake_minimum_required(VERSION 3.5)

add_executable(${PROJECT_NAME}_b 5_SHARED_DATA_SPACE_CHILD.c)

target_include_directories(${PROJECT_NAME}_b PRIVATE ${CMAKE_SOURCE_DIR}/wimp/src ${CMAKE_SOURCE_DIR}/tests/utility)

add_dependencies(${PROJECT_NAME}_b wimp WIMP-Test)
target_link_libraries(${PROJECT_NAME}_b wimp WIMP-Test)
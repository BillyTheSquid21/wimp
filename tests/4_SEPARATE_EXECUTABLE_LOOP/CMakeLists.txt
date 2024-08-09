cmake_minimum_required(VERSION 3.5)
project(WIMP-Test-04)

# Master
add_executable(${PROJECT_NAME}_a 4_SEPARATE_EXECUTABLE_LOOP_MASTER.c)

target_include_directories(${PROJECT_NAME}_a PRIVATE ${CMAKE_SOURCE_DIR}/wimp/src ${CMAKE_SOURCE_DIR}/tests/utility)

add_dependencies(${PROJECT_NAME}_a wimp WIMP-Test)
target_link_libraries(${PROJECT_NAME}_a wimp WIMP-Test)

# Child
add_executable(${PROJECT_NAME}_b 4_SEPARATE_EXECUTABLE_LOOP_CHILD.c)

target_include_directories(${PROJECT_NAME}_b PRIVATE ${CMAKE_SOURCE_DIR}/wimp/src ${CMAKE_SOURCE_DIR}/tests/utility)

add_dependencies(${PROJECT_NAME}_b wimp WIMP-Test)
target_link_libraries(${PROJECT_NAME}_b wimp WIMP-Test)
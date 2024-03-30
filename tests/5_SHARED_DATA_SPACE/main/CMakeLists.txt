cmake_minimum_required(VERSION 3.5)

add_executable(${PROJECT_NAME}_a 5_SHARED_DATA_SPACE_MAIN.c)

target_include_directories(${PROJECT_NAME}_a PRIVATE ${CMAKE_SOURCE_DIR}/wimp/src ${CMAKE_SOURCE_DIR}/tests/utility)

add_dependencies(${PROJECT_NAME}_a wimp WIMP-Test)
target_link_libraries(${PROJECT_NAME}_a wimp WIMP-Test)
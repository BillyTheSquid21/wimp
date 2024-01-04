cmake_minimum_required(VERSION 3.5)
project(WIMP-Test-02)

add_executable(${PROJECT_NAME} 2_INSTRUCTION_BRUTE_FORCE_TIME.c)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/wimp/src ${CMAKE_SOURCE_DIR}/tests/utility)

add_dependencies(${PROJECT_NAME} wimp WIMP-Test)
target_link_libraries(${PROJECT_NAME} wimp WIMP-Test)
cmake_minimum_required(VERSION 3.5)
project(WIMP-Test)

add_subdirectory(utility)
add_subdirectory(1_SEND_RECIEVE_LOOP)
add_subdirectory(2_INSTRUCTION_BRUTE_FORCE_TIME)
add_subdirectory(3_MASTER_CHILD_ROUTING)
add_subdirectory(4_SEPARATE_EXECUTABLE_LOOP)
add_subdirectory(5_SHARED_DATA_SPACE)

set(TEST_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
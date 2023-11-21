cmake_minimum_required(VERSION 3.5)

add_executable(${PROJECT_NAME} server.c)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/wimp/src)

add_dependencies(${PROJECT_NAME} wimp)
target_link_libraries(${PROJECT_NAME} wimp)
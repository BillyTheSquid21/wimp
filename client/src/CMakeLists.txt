cmake_minimum_required(VERSION 3.5)

add_executable(${PROJECT_NAME} client.c)
target_include_directories(${PROJECT_NAME} PRIVATE "D:/Repositories/Wills Interesting Message Protocol/wimp/src")

add_dependencies(${PROJECT_NAME} wimp)
target_link_libraries(${PROJECT_NAME} wimp)
cmake_minimum_required(VERSION 3.21)
project(LevelSketch)

message("Building ${PROJECT_NAME} with configuration: ${CMAKE_BUILD_TYPE}")

add_subdirectory(Source)

file(COPY Content DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Source)

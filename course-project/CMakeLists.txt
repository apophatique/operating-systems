cmake_minimum_required(VERSION 3.13)

project(os-course-project C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-lpthread -lX11")

SET(PROJECT_SOURCES_DIR ${PROJECT_SOURCE_DIR}/src)

FILE(GLOB_RECURSE SOURCES ${PROJECT_SOURCES_DIR}/*.c)

add_executable(${PROJECT_NAME} ${SOURCES})

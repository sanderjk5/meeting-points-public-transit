cmake_minimum_required(VERSION 3.2)

project(meeting-points-public-transit)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

set(INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)

set(DATA_HANDLING_HEADERS_DIR ${PROJECT_SOURCE_DIR}/src/data-handling)

include_directories(${DATA_HEADERS_DIR})

add_subdirectory(src)



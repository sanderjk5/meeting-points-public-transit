cmake_minimum_required(VERSION 3.2)
project(meeting-points-public-transit)

add_subdirectory(data-handling)
set(SOURCE_FILES 
    main.cpp
)

add_executable(meeting-points-public-transit ${SOURCE_FILES})
install(TARGETS meeting-points-public-transit DESTINATION ${INSTALL_BIN_DIR})

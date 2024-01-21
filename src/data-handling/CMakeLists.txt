cmake_minimum_required(VERSION 3.2)
project(data-handling)

set(SOURCE_FILES
    importer.h
    importer.cpp
)

add_library(data-handling SHARED STATIC ${SOURCE_FILES})

install(TARGETS data-handling DESTINATION ${INSTALL_LIB_DIR})
install(FILES division.h DESTINATION ${INSTALL_INCLUDE_DIR})
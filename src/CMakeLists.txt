cmake_minimum_required(VERSION 3.2)
project(main-project)

add_subdirectory(data-handling)
set(SOURCE_FILES 
    main.cpp
)

add_executable(main-project ${SOURCE_FILES})
target_link_libraries(main-project data-handling)
install(TARGETS main-project DESTINATION ${INSTALL_BIN_DIR})

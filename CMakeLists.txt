cmake_minimum_required(VERSION 3.0.0)
project(CommandOptionsCmake VERSION 0.1.0 LANGUAGES C)

include(CTest)
enable_testing()

add_executable(testcmdopts main.c)

add_subdirectory(command_options)

target_link_libraries(testcmdopts PUBLIC command_options)
target_include_directories(testcmdopts PUBLIC "{PROJECT_BINARY_DIR}" "{PROJECT_SOURCE_DIR}/command_options")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

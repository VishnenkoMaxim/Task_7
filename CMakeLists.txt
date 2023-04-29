cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 17)
set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})

project(Task_7 VERSION ${PROJECT_VERSION})

configure_file(version.h.in version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

link_directories(/usr/local/lib)
link_directories(${CMAKE_BINARY_DIR})

add_executable(bulk main.cpp)

target_link_libraries(bulk PRIVATE stdc++ m)

set_target_properties(bulk PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_compile_options(bulk PRIVATE -Wall -Wextra -pedantic)

install(TARGETS bulk RUNTIME DESTINATION bin)
set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Vishnenko Maxim")

include(CPack)
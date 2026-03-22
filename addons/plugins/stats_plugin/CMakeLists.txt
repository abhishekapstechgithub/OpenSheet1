cmake_minimum_required(VERSION 3.20)
project(stats_plugin)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(stats_plugin SHARED
    stats_plugin.cpp
)

target_link_libraries(stats_plugin PRIVATE Qt6::Core)
target_include_directories(stats_plugin PRIVATE
    ${CMAKE_SOURCE_DIR}/../../..   # OpenSheet root
)

set_target_properties(stats_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

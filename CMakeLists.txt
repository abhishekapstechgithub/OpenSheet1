cmake_minimum_required(VERSION 3.20)
project(OpenSheet VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS
    Core Widgets Gui Charts PrintSupport Sql Network Xml
)

# Find SQLite3
find_package(SQLite3 REQUIRED)

# Optional Python support for plugins
find_package(Python3 COMPONENTS Interpreter Development QUIET)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /MP)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

# Include cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Global include dirs
include_directories(
    ${CMAKE_SOURCE_DIR}/office/engine
    ${CMAKE_SOURCE_DIR}/office/core
    ${CMAKE_SOURCE_DIR}/office/ui
    ${CMAKE_SOURCE_DIR}/office/charts
    ${CMAKE_SOURCE_DIR}/office/formulas
    ${CMAKE_SOURCE_DIR}/plugins
)

# Sub-modules
add_subdirectory(office/engine)
add_subdirectory(office/core)
add_subdirectory(office/ui)
add_subdirectory(office/charts)
add_subdirectory(office/formulas)
add_subdirectory(plugins)
add_subdirectory(tests)

# Main executable
add_executable(opensheet
    main.cpp
    resources/resources.qrc
)

target_link_libraries(opensheet PRIVATE
    os_engine
    os_core
    os_ui
    os_charts
    os_formulas
    os_plugins
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Charts
    Qt6::PrintSupport
    Qt6::Sql
    Qt6::Network
    Qt6::Xml
    SQLite::SQLite3
)

if(Python3_FOUND)
    target_link_libraries(opensheet PRIVATE Python3::Python)
    target_compile_definitions(opensheet PRIVATE OPENSHEET_PYTHON_SUPPORT)
endif()

# Install rules
install(TARGETS opensheet DESTINATION bin)
install(DIRECTORY themes/    DESTINATION themes)
install(DIRECTORY resources/ DESTINATION resources)
install(DIRECTORY localization/ DESTINATION localization)
install(DIRECTORY addons/    DESTINATION addons)

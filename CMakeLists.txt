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
# vcpkg provides unofficial::sqlite3::sqlite3; system cmake provides SQLite::SQLite3
find_package(unofficial-sqlite3 CONFIG QUIET)
if(unofficial-sqlite3_FOUND)
    set(SQLITE3_TARGET unofficial::sqlite3::sqlite3)
    message(STATUS "Using vcpkg SQLite3: unofficial::sqlite3::sqlite3")
else()
    find_package(SQLite3 REQUIRED)
    set(SQLITE3_TARGET SQLite::SQLite3)
    message(STATUS "Using system SQLite3: SQLite::SQLite3")
endif()

# Optional Python support for plugins
find_package(Python3 COMPONENTS Interpreter Development QUIET)

# Build options
option(OPENSHEET_BUILD_TESTS    "Build unit tests"          ON)
option(OPENSHEET_PYTHON_SUPPORT "Enable Python plugin loader" AUTO)
option(OPENSHEET_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

# Include cmake modules
# Include project cmake helpers directly (do NOT add cmake/ to CMAKE_MODULE_PATH
# because that would cause any FindXxx.cmake files there to intercept find_package calls)
include(cmake/OpenSheetConfig.cmake)
include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)

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
    ${SQLITE3_TARGET}
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

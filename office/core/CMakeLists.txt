add_library(os_core STATIC
    file_manager.cpp
    file_system/csv_reader.cpp
    file_system/xlsx_reader.cpp
    file_system/opensheet_format.cpp
    settings_manager.cpp
    crash_recovery.cpp
    print_manager.cpp
    localization_manager.cpp
    logger.cpp
    theme_manager.cpp
)

# Qt6CorePrivate gives us access to QZipReader/QZipWriter private headers
# used in opensheet_format.cpp and xlsx_reader.cpp
find_package(Qt6 QUIET COMPONENTS CorePrivate)

target_link_libraries(os_core PUBLIC
    os_engine
    Qt6::Core
    Qt6::Xml
    Qt6::Network
    Qt6::PrintSupport
    $<IF:$<TARGET_EXISTS:unofficial::sqlite3::sqlite3>,unofficial::sqlite3::sqlite3,SQLite::SQLite3>
)

if(TARGET Qt6::CorePrivate)
    target_link_libraries(os_core PRIVATE Qt6::CorePrivate)
    target_compile_definitions(os_core PRIVATE OPENSHEET_HAS_QT_CORE_PRIVATE)
endif()

target_include_directories(os_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

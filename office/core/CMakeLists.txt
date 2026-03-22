add_library(os_core STATIC
    file_system/file_manager.cpp
    file_system/csv_reader.cpp
    file_system/xlsx_reader.cpp
    file_system/opensheet_format.cpp
    settings_manager.cpp
    crash_recovery.cpp
)
target_link_libraries(os_core PUBLIC
    os_engine Qt6::Core Qt6::Xml Qt6::Network SQLite::SQLite3
)
target_include_directories(os_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

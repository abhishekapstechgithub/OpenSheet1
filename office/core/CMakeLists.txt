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

target_link_libraries(os_core PUBLIC
    os_engine
    Qt6::Core
    Qt6::Xml
    Qt6::Network
    Qt6::PrintSupport
    $<IF:$<TARGET_EXISTS:unofficial::sqlite3::sqlite3>,unofficial::sqlite3::sqlite3,SQLite::SQLite3>
)

# Qt private ZIP headers (qzipreader_p.h / qzipwriter_p.h) are available
# whenever Qt development files are installed (which they always are on CI).
# We define OPENSHEET_HAS_QT_CORE_PRIVATE unconditionally and link Qt6::CorePrivate
# only if that target exists (it exists in Qt 6 dev installations).
# This avoids calling find_package(Qt6 CorePrivate) which can trigger
# infinite recursion with vcpkg's find_package wrapper.
if(TARGET Qt6::CorePrivate)
    target_link_libraries(os_core PRIVATE Qt6::CorePrivate)
    target_compile_definitions(os_core PRIVATE OPENSHEET_HAS_QT_CORE_PRIVATE)
else()
    # On CI, Qt6 dev is installed via aqtinstall which includes private headers.
    # Force the define so ZIP support is compiled in.
    target_compile_definitions(os_core PRIVATE OPENSHEET_HAS_QT_CORE_PRIVATE)
    # Add the Qt private include path manually
    target_include_directories(os_core PRIVATE
        "${Qt6Core_PRIVATE_INCLUDE_DIR}"
    )
endif()

target_include_directories(os_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

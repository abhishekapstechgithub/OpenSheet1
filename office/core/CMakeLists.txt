# os_core: header-only implementations but Q_OBJECT classes need MOC compilation units
set(_core_src ${CMAKE_CURRENT_SOURCE_DIR})

add_library(os_core STATIC
    ${_core_src}/settings_manager.cpp
    ${_core_src}/crash_recovery.cpp
    ${_core_src}/logger.cpp
    ${_core_src}/theme_manager.cpp
    ${_core_src}/localization_manager.cpp
    ${_core_src}/print_manager.cpp
)

target_link_libraries(os_core PUBLIC
    os_engine
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Xml
    Qt6::Network
    Qt6::PrintSupport
    $<IF:$<TARGET_EXISTS:unofficial::sqlite3::sqlite3>,unofficial::sqlite3::sqlite3,SQLite::SQLite3>
)

if(TARGET Qt6::CorePrivate)
    target_link_libraries(os_core PRIVATE Qt6::CorePrivate)
    target_compile_definitions(os_core PRIVATE OPENSHEET_HAS_QT_CORE_PRIVATE)
else()
    target_compile_definitions(os_core PRIVATE OPENSHEET_HAS_QT_CORE_PRIVATE)
    target_include_directories(os_core PRIVATE "${Qt6Core_PRIVATE_INCLUDE_DIR}")
endif()

target_include_directories(os_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

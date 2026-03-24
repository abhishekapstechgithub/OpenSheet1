# os_core is fully header-only. No Q_OBJECT macros in any header,
# so no MOC compilation units are needed.
add_library(os_core INTERFACE)

target_link_libraries(os_core INTERFACE
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
    target_link_libraries(os_core INTERFACE Qt6::CorePrivate)
    target_compile_definitions(os_core INTERFACE OPENSHEET_HAS_QT_CORE_PRIVATE)
else()
    target_compile_definitions(os_core INTERFACE OPENSHEET_HAS_QT_CORE_PRIVATE)
    target_include_directories(os_core INTERFACE "${Qt6Core_PRIVATE_INCLUDE_DIR}")
endif()

target_include_directories(os_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# os_core is now header-only — all implementations inlined into .h files.
# We use INTERFACE so dependents get the include path and link libraries
# without needing any .cpp compilation units.
add_library(os_core INTERFACE)

target_link_libraries(os_core INTERFACE
    os_engine
    Qt6::Core
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

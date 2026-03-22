add_library(os_plugins STATIC
    plugin_manager.cpp
)
target_link_libraries(os_plugins PUBLIC os_engine Qt6::Core)
target_include_directories(os_plugins PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

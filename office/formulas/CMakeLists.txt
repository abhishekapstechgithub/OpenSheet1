add_library(os_formulas STATIC
    formula_registry.cpp
)
target_link_libraries(os_formulas PUBLIC os_engine Qt6::Core)
target_include_directories(os_formulas PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
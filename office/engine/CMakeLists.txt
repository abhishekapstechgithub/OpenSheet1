add_library(os_engine STATIC
    cell.cpp
    sheet.cpp
    workbook.cpp
    formula_parser.cpp
    dependency_graph.cpp
    recalc_engine.cpp
    cell_range.cpp
    named_ranges.cpp
)
target_link_libraries(os_engine PUBLIC Qt6::Core)
target_include_directories(os_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

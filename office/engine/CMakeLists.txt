set(_eng ${CMAKE_CURRENT_SOURCE_DIR})

add_library(os_engine STATIC
    ${_eng}/cell.cpp
    ${_eng}/cell_range.cpp
    ${_eng}/sheet.cpp
    ${_eng}/workbook.cpp
    ${_eng}/formula_parser.cpp
    ${_eng}/dependency_graph.cpp
    ${_eng}/recalc_engine.cpp
    ${_eng}/named_ranges.cpp
    ${_eng}/number_formatter.cpp
    ${_eng}/auto_fill.cpp
    ${_eng}/pivot_table.cpp
    ${_eng}/data_validation.cpp
)

target_link_libraries(os_engine PUBLIC Qt6::Core)
target_include_directories(os_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

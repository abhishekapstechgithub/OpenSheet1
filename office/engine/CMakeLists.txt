add_library(os_engine STATIC
    cell.cpp
    cell_range.cpp
    sheet.cpp
    workbook.cpp
    formula_parser.cpp
    dependency_graph.cpp
    recalc_engine.cpp
    named_ranges.cpp
    number_formatter.cpp
    auto_fill.cpp
    pivot_table.cpp
    data_validation.cpp
)

target_link_libraries(os_engine PUBLIC Qt6::Core)
target_include_directories(os_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# undo_commands.h is header-only (all QUndoCommand subclasses are inline)
# formula_registry lives in office/formulas/, not here

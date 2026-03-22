add_library(os_ui STATIC
    main_window.cpp
    ribbon_bar.cpp
    spreadsheet_view.cpp
    formula_bar.cpp
    sheet_tabs.cpp
    status_bar.cpp
    side_panel.cpp
    find_replace_dialog.cpp
    cell_format_dialog.cpp
    insert_function_dialog.cpp
    chart_dialog.cpp
)
target_link_libraries(os_ui PUBLIC
    os_engine os_core os_charts
    Qt6::Widgets Qt6::Gui Qt6::Charts Qt6::PrintSupport
)
target_include_directories(os_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(_ui ${CMAKE_CURRENT_SOURCE_DIR})

add_library(os_ui STATIC
    ${_ui}/main_window.cpp
    ${_ui}/ribbon_bar.cpp
    ${_ui}/spreadsheet_view.cpp
    ${_ui}/formula_bar.cpp
    ${_ui}/sheet_tabs.cpp
    ${_ui}/status_bar.cpp
    ${_ui}/side_panel.cpp
    ${_ui}/find_replace_dialog.cpp
    ${_ui}/cell_format_dialog.cpp
    ${_ui}/insert_function_dialog.cpp
    ${_ui}/chart_dialog.cpp
)

target_link_libraries(os_ui PUBLIC
    os_engine os_core os_charts
    Qt6::Widgets Qt6::Gui Qt6::Charts Qt6::PrintSupport
)
target_include_directories(os_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

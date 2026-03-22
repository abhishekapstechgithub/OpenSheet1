add_library(os_charts STATIC
    chart_base.cpp
)

# Qt6::Svg is only available if the qtsvg module is installed separately.
# Since we only install qtcharts, link against what's available.
# chart_base.cpp uses QPainter (Qt6::Widgets/Gui) and Qt6::Charts only.
target_link_libraries(os_charts PUBLIC
    os_engine
    Qt6::Widgets
    Qt6::Gui
    Qt6::Charts
)

target_include_directories(os_charts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

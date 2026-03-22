add_library(os_charts STATIC
    chart_base.cpp
)
target_link_libraries(os_charts PUBLIC
    os_engine Qt6::Widgets Qt6::Gui Qt6::Charts Qt6::Svg
)
target_include_directories(os_charts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

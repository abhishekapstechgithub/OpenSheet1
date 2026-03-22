enable_testing()
find_package(Qt6 REQUIRED COMPONENTS Test Widgets)

macro(opensheet_add_test TARGET)
    add_executable(${TARGET} ${ARGN})
    target_link_libraries(${TARGET} PRIVATE
        os_engine os_core os_charts os_formulas os_plugins
        Qt6::Test Qt6::Widgets Qt6::Core
    )
    target_include_directories(${TARGET} PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/office/engine
        ${CMAKE_SOURCE_DIR}/office/core
        ${CMAKE_SOURCE_DIR}/office/charts
        ${CMAKE_SOURCE_DIR}/office/formulas
        ${CMAKE_SOURCE_DIR}/plugins
    )
    add_test(NAME ${TARGET} COMMAND ${TARGET})
    set_tests_properties(${TARGET} PROPERTIES TIMEOUT 60)
endmacro()

opensheet_add_test(test_cell              test_cell.cpp)
opensheet_add_test(test_sheet             test_sheet.cpp)
opensheet_add_test(test_workbook          test_workbook.cpp)
opensheet_add_test(test_formula_parser    test_formula_parser.cpp)
opensheet_add_test(test_dependency_graph  test_dependency_graph.cpp)
opensheet_add_test(test_csv               test_csv.cpp)
opensheet_add_test(test_xlsx              test_xlsx.cpp)
opensheet_add_test(test_opensheet_format  test_opensheet_format.cpp)
opensheet_add_test(test_charts            test_charts.cpp)

set_tests_properties(test_cell test_sheet test_workbook
    test_formula_parser test_dependency_graph PROPERTIES LABELS "engine")
set_tests_properties(test_csv test_xlsx test_opensheet_format PROPERTIES LABELS "fileio")
set_tests_properties(test_charts PROPERTIES LABELS "charts")

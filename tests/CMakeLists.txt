enable_testing()
find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(test_formula_parser test_formula_parser.cpp)
target_link_libraries(test_formula_parser PRIVATE
    os_engine os_core Qt6::Test Qt6::Core
)
add_test(NAME FormulaParserTests COMMAND test_formula_parser)

add_executable(test_engine test_engine.cpp)
target_link_libraries(test_engine PRIVATE
    os_engine os_core Qt6::Test Qt6::Core
)
add_test(NAME EngineTests COMMAND test_engine)

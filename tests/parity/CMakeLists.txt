sc_add_test(tst_parity_engine
    SOURCES tst_parity_engine.cpp
    LIBS scengine Qt6::Core
    LABELS parity
)
sc_add_test(tst_parity_core
    SOURCES tst_parity_core.cpp
    LIBS sccore
    LABELS parity
)
target_compile_definitions(tst_parity_core PRIVATE SC_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

set_tests_properties(tst_parity_engine PROPERTIES
    TIMEOUT 10800
    ENVIRONMENT "${SC_TEST_ENV};QTEST_FUNCTION_TIMEOUT=10800000"
)

sc_add_test(tst_parity_generator
    SOURCES tst_parity_generator.cpp
    LIBS sccore
    LABELS parity
)
target_compile_definitions(tst_parity_generator PRIVATE SC_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
set_tests_properties(tst_parity_generator PROPERTIES
    TIMEOUT 3600
    ENVIRONMENT "${SC_TEST_ENV};QTEST_FUNCTION_TIMEOUT=3600000"
)

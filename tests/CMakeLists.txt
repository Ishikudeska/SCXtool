get_filename_component(SC_COMPILER_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
set(SC_QT_BIN "${QT6_INSTALL_PREFIX}/${QT6_INSTALL_BINS}")
set(SC_TEST_ENV_PATH
    "PATH=path_list_prepend:${SC_QT_BIN}"
    "PATH=path_list_prepend:${SC_COMPILER_BIN}"
)
set(SC_TEST_ENV "QT_FORCE_STDERR_LOGGING=1")

function(sc_share_test_pch name base)
    foreach(lib IN LISTS ARGN)
        if(lib MATCHES "^Qt6::" AND NOT lib STREQUAL "Qt6::Core")
            return()
        endif()
    endforeach()
    file(RELATIVE_PATH folder "${PROJECT_SOURCE_DIR}/tests" "${CMAKE_CURRENT_SOURCE_DIR}")
    set(pch "sc_test_pch_${folder}_${base}")
    if(NOT TARGET ${pch})
        add_library(${pch} OBJECT "${PROJECT_SOURCE_DIR}/tests/pch.cpp")
        set_target_properties(${pch} PROPERTIES AUTOMOC OFF)
        target_link_libraries(${pch} PRIVATE Qt6::Test ${base})
        sc_set_warnings(${pch})
        sc_precompile(${pch} <QSignalSpy> <QTemporaryDir> <QTest>)
    endif()
    target_precompile_headers(${name} REUSE_FROM ${pch})
    set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/${name}_autogen/mocs_compilation.cpp"
        PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endfunction()

function(sc_add_test name)
    cmake_parse_arguments(T "" "" "SOURCES;LIBS;LABELS" ${ARGN})
    add_executable(${name} ${T_SOURCES})
    target_link_libraries(${name} PRIVATE Qt6::Test ${T_LIBS})
    sc_set_warnings(${name})
    if(SC_USE_PCH)
        sc_share_test_pch(${name} ${T_LIBS})
    endif()
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES
        ENVIRONMENT "${SC_TEST_ENV}"
        ENVIRONMENT_MODIFICATION "${SC_TEST_ENV_PATH}"
        LABELS "${T_LABELS}"
    )
endfunction()

add_subdirectory(engine)
add_subdirectory(core)
add_subdirectory(app)
add_subdirectory(parity)

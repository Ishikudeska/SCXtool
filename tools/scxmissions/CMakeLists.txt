add_executable(scxmissions main.cpp)
target_link_libraries(scxmissions PRIVATE sccore)
sc_set_warnings(scxmissions)
target_compile_definitions(scxmissions PRIVATE SC_PATCHES_DIR="${PROJECT_SOURCE_DIR}/resources/patches")

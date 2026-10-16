add_executable(scxgen main.cpp)
target_link_libraries(scxgen PRIVATE sccore)
sc_set_warnings(scxgen)

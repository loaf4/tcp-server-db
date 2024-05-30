add_library(Server_impl STATIC
    ${CMAKE_CURRENT_LIST_DIR}/server_impl.cpp)

target_include_directories(Server_impl PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(Server_impl PRIVATE spdlog::spdlog)

set(SRC
    ${CMAKE_CURRENT_LIST_DIR}/client.cpp
    ${CMAKE_CURRENT_LIST_DIR}/server.cpp)

add_executable(client ${CMAKE_CURRENT_LIST_DIR}/client.cpp)

add_executable(server ${CMAKE_CURRENT_LIST_DIR}/server.cpp)
target_link_libraries(server PRIVATE spdlog::spdlog)

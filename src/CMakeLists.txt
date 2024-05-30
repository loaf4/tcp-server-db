add_executable(Client ${CMAKE_CURRENT_LIST_DIR}/client.cpp)
target_link_libraries(Client PRIVATE Client_impl)

add_executable(Server ${CMAKE_CURRENT_LIST_DIR}/server.cpp)
target_link_libraries(Server PRIVATE Server_impl)

add_subdirectory(client)
add_subdirectory(server)


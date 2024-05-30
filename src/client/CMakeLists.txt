add_library(Client_impl STATIC 
    ${CMAKE_CURRENT_LIST_DIR}/client_impl.cpp)

target_include_directories(Client_impl PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

cmake_minimum_required(VERSION 3.10)
project(redis_impl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-Wall")

include(FetchContent)

#-------------------
# spdlog
#-------------------
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)

#-------------------
# Boost
#-------------------
find_package(Boost COMPONENTS REQUIRED system)

add_subdirectory(src)


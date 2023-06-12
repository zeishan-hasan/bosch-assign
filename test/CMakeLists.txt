cmake_minimum_required(VERSION 3.16.3)

set(This tests)

set(Sources
    tests.cpp
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
add_executable(${This} ${Sources})
target_link_libraries(${This} PUBLIC
    gtest_main
    bosch-assign
)

add_test(
    NAME ${This}
    COMMAND ${This}
)
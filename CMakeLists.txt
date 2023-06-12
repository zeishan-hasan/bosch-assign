cmake_minimum_required(VERSION 3.16.3)

# Below is setting the project folder name
# 'gtest-example' to 'This':
set(This bosch-assign)

# set(This jun-9-1006am)

project(${This} CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

enable_testing()

set(Headers
    bosch-assign.hpp
)

set(Sources 
    bosch-assign.cpp
)

add_library(${This} STATIC ${Sources} ${Headers})

add_subdirectory(test)
add_subdirectory(googletest)
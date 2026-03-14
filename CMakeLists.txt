cmake_minimum_required(VERSION 3.16)
project(orderbook CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(orderbook
    src/main.cpp
    src/order_book.cpp
)

target_include_directories(orderbook PRIVATE include)

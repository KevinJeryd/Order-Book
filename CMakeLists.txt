cmake_minimum_required(VERSION 3.16)
project(orderbook CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(orderbook
    src/main.cpp
    src/order_book.cpp
)

target_include_directories(orderbook PRIVATE include)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

add_executable(orderbook_tests
    tests/order_book_test.cpp
    src/order_book.cpp
)

target_include_directories(orderbook_tests PRIVATE include)
target_link_libraries(orderbook_tests gtest_main)

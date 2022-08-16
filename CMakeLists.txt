cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)

project(CppConcurrency)

add_executable(ConcurrencyBenchmark
    Main.cpp
    ThreadInstanceAllocator.cpp
)
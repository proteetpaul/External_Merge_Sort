cmake_minimum_required(VERSION 3.8)
project(external_merge_sort C CXX)

set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(src)
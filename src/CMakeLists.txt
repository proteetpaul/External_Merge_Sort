if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_library(merge_sort SHARED
            Assert.cpp  
            defs.cpp    defs.h
            Filter.cpp  Filter.h    
            Iterator.h  Iterator.cpp
            Record.h    
            Scan.h  Scan.cpp
            Sort.h  Sort.cpp 
            Witness.cpp Witness.h
            Sorter.h Sorter.cpp tree.h)

set_property(TARGET merge_sort PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(merge_sort PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test Test.cpp)
target_include_directories(test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test merge_sort)
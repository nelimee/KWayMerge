cmake_minimum_required(VERSION 3.10)

project(KWayMerge)

#######################################
# Compiler configuration
#######################################
set(CMAKE_CXX_STANDARD 17)
file(GLOB_RECURSE
     ALL_SOURCE_FILES
     "${CMAKE_SOURCE_DIR}/tests/*.cpp"
     "${CMAKE_SOURCE_DIR}/tests/*.hpp"
     "${CMAKE_SOURCE_DIR}/tests/*.h"
     "${CMAKE_SOURCE_DIR}/include/merge.h"
     )
set(HEADER_ONLY_LIBRARY_HEADER_FILES
    ${CMAKE_SOURCE_DIR}/include/merge.h)

#######################################
# General settings
#######################################

include_directories(${CMAKE_SOURCE_DIR}/include)

#######################################
# OpenMP support
#######################################
find_package(OpenMP)


#######################################
# Unit-testing with Catch2
#######################################
set(CATCH_SINGLE_INCLUDE_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/include)
set(CATCH_SINGLE_INCLUDE_FILE ${CATCH_SINGLE_INCLUDE_DIRECTORY}/catch.hpp)
include_directories(${CATCH_SINGLE_INCLUDE_DIRECTORY})


#######################################
# Declare example executable
#######################################
set(EXAMPLE_SOURCE_FILES
    example.cpp)
add_executable(KWayMergeExample ${EXAMPLE_SOURCE_FILES} ${HEADER_ONLY_LIBRARY_HEADER_FILES})


#######################################
# Declare tests
#######################################
set(SEQUENTIAL_TEST_MAIN_FILE
    tests/sequential.cpp)
file(GLOB_RECURSE
     SEQUENTIAL_TEST_FILES
     "${CMAKE_SOURCE_DIR}/tests/sequential/*.cpp")
add_executable(KWayMergeSequentialTest
               ${SEQUENTIAL_TEST_MAIN_FILE}
               ${SEQUENTIAL_TEST_FILES}
               ${HEADER_ONLY_LIBRARY_HEADER_FILES})

# The parallel test is just the sequential one compiled with OpenMP support.
add_executable(KWayMergeParallelTest
               ${SEQUENTIAL_TEST_MAIN_FILE}
               ${SEQUENTIAL_TEST_FILES}
               ${HEADER_ONLY_LIBRARY_HEADER_FILES})
target_compile_options(KWayMergeParallelTest PUBLIC ${OpenMP_CXX_FLAGS})
target_link_libraries(KWayMergeParallelTest ${OpenMP_CXX_LIBRARIES})

cmake_minimum_required(VERSION 3.10)

project(KWayMerge)

#######################################
# Compiler configuration
#######################################
set(CMAKE_CXX_STANDARD 11)
file(GLOB_RECURSE
        KWAY_MERGE_ALL_SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/merge.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.cpp"
        )
file(GLOB_RECURSE
        KWAY_MERGE_HEADER_ONLY_LIBRARY_HEADER_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/merge.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)


#######################################
# General settings
#######################################
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)

    #######################################
    # OpenMP support
    #######################################
    find_package(OpenMP)


    #######################################
    # Declare example executable
    #######################################
    set(EXAMPLE_SOURCE_FILES
            example.cpp)
    add_executable(KWayMergeExample
            ${EXAMPLE_SOURCE_FILES}
            ${KWAY_MERGE_HEADER_ONLY_LIBRARY_HEADER_FILES}
            )

    add_subdirectory(tests)
    add_subdirectory(benchmarks)

else ()
    add_library(KWayMerge INTERFACE)
    target_include_directories(KWayMerge INTERFACE ${KWAY_MERGE_HEADER_ONLY_LIBRARY_HEADER_FILES})
endif ()

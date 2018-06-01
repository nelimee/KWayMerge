#######################################
# Micro-benchmarking with Google Benchmark
#######################################
include(ExternalProject)
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)
ExternalProject_Add(gbench
        GIT_REPOSITORY https://github.com/google/benchmark
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DBENCHMARK_DOWNLOAD_DEPENDENCIES=ON
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        )
include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)
file(GLOB_RECURSE
        BENCHMARK_INCLUDE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/include/*"
        )

find_package(Threads)

#######################################
# Declare benchmarks
#######################################
set(BENCHMARK_TEST_MAIN_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/sequential.cpp)
file(GLOB_RECURSE
        BENCHMARK_TEST_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/sequential/*.cpp")

add_executable(KWayMergeSequentialBenchmark
        ${BENCHMARK_TEST_MAIN_FILE}
        ${BENCHMARK_TEST_FILES}
        ${KWAY_MERGE_HEADER_ONLY_LIBRARY_HEADER_FILES})
target_link_libraries(KWayMergeSequentialBenchmark benchmark ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(KWayMergeSequentialBenchmark gbench)
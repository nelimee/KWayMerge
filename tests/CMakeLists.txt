#######################################
# Unit-testing with Catch2
#######################################
set(CATCH_SINGLE_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)
# Prepare "Catch" library for other executables
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_SINGLE_INCLUDE_DIRECTORY})


#######################################
# Declare tests
#######################################
set(VALIDITY_TEST_MAIN_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/validity.cpp)
file(GLOB_RECURSE
        VALIDITY_TEST_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/validity/*.cpp")

add_executable(KWayMergeSequentialValidityTest
        ${VALIDITY_TEST_MAIN_FILE}
        ${VALIDITY_TEST_FILES}
        ${KWAY_MERGE_HEADER_ONLY_LIBRARY_HEADER_FILES})
target_link_libraries(KWayMergeSequentialValidityTest Catch)


# The parallel test is just the sequential one compiled with OpenMP support.
add_executable(KWayMergeParallelValidityTest
        ${VALIDITY_TEST_MAIN_FILE}
        ${VALIDITY_TEST_FILES}
        ${KWAY_MERGE_HEADER_ONLY_LIBRARY_HEADER_FILES})
target_compile_options(KWayMergeParallelValidityTest PUBLIC ${OpenMP_CXX_FLAGS})
target_link_libraries(KWayMergeParallelValidityTest
        ${OpenMP_CXX_LIBRARIES}
        Catch)
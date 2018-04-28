cmake_minimum_required(VERSION 3.10)

project(KWayMerge)

#######################################
# Compiler configuration
#######################################
set(CMAKE_CXX_STANDARD 17)
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
     include/merge.h
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
	# Unit-testing with Catch2
	#######################################
	set(CATCH_SINGLE_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/include)
	# Prepare "Catch" library for other executables
	add_library(Catch INTERFACE)
	target_include_directories(Catch INTERFACE ${CATCH_SINGLE_INCLUDE_DIRECTORY})

	#######################################
	# Declare example executable
	#######################################
	set(EXAMPLE_SOURCE_FILES
	    example.cpp)
	add_executable(KWayMergeExample ${EXAMPLE_SOURCE_FILES} ${KWAY_MERGE_HEADER_ONLY_LIBRARY_HEADER_FILES})


	#######################################
	# Declare tests
	#######################################
	set(VALIDITY_TEST_MAIN_FILE
	    ${CMAKE_CURRENT_SOURCE_DIR}/tests/validity.cpp)
	file(GLOB_RECURSE
	     VALIDITY_TEST_FILES
	     "${CMAKE_CURRENT_SOURCE_DIR}/tests/validity/*.cpp")

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
else ()
	add_library(KWayMerge INTERFACE)
	target_include_directories(KWayMerge INTERFACE ${KWAY_MERGE_HEADER_ONLY_LIBRARY_HEADER_FILES})
endif ()

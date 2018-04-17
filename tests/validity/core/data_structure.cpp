#include "catch.hpp"

#include "define.hpp"

#include <list>
#include <deque>
#include <set>
#include <unordered_set>

#include "merge.h"


#define KWAYMERGE_TEST_EXTERNAL_DATA_STRUCTURE(EXTERNAL_DATA_STRUCTURE, EXTERNAL_DATA_STRUCTURE_STRING, EMPLACE_BACK_METHOD_NAME) \
TEST_CASE("ExternalContainer == " EXTERNAL_DATA_STRUCTURE_STRING ", InternalContainer == std::vector", "[core][data-structure][random]") { \
\
KWAYMERGE_TEST_BUILD_RANDOM_CONTAINER_OF_CONTAINER_OF_DOUBLES(array,       /*name*/\
                                                              std::vector, /*internal container*/\
                                                              EXTERNAL_DATA_STRUCTURE,  /*external container*/\
                                                              2,           /*external size*/\
                                                              10,          /*internal size min*/\
                                                              10,          /*internal size max*/\
                                                              0.0,         /*double min*/\
                                                              1.0,         /*double max*/\
                                                              EMPLACE_BACK_METHOD_NAME /*emplace_back method name*/) \
\
std::size_t const expected_size{20};\
std::vector<double> result_kway_merge = merge_arrays(array);\
\
REQUIRE(result_kway_merge.size() == expected_size);\
REQUIRE(std::is_sorted(result_kway_merge.begin(), result_kway_merge.end()));\
}\


KWAYMERGE_TEST_EXTERNAL_DATA_STRUCTURE(std::list, "std::list", emplace_back)

KWAYMERGE_TEST_EXTERNAL_DATA_STRUCTURE(std::vector, "std::vector", emplace_back)

KWAYMERGE_TEST_EXTERNAL_DATA_STRUCTURE(std::deque, "std::deque", emplace_back)


TEST_CASE("ExternalContainer == std::forward_list, InternalContainer == std::vector",
          "[core][data-structure][random]") {

	KWAYMERGE_TEST_BUILD_RANDOM_FORWARD_LIST_OF_CONTAINER_OF_DOUBLES(array,             /*name*/
	                                                                 std::vector,       /*internal container*/
	/*std::forward_list,*/
	                                                                 2,                 /*external size*/
	                                                                 10,                /*internal size min*/
	                                                                 10,                /*internal size max*/
	                                                                 0.0,               /*double min*/
	                                                                 1.0)               /*double max*/

	std::size_t const expected_size{20};
	std::vector<double> result_kway_merge = merge_arrays(array);

	REQUIRE(result_kway_merge.size() == expected_size);
	REQUIRE(std::is_sorted(result_kway_merge.begin(), result_kway_merge.end()));

}

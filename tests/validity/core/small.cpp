#include "catch.hpp"
#include "define.hpp"
#include "merge.h"


TEST_CASE("merging 2 small arrays", "[core][small][random]") {

	KWAYMERGE_TEST_BUILD_RANDOM_CONTAINER_OF_CONTAINER_OF_DOUBLES(array,       /*name*/
	                                                              std::vector, /*internal container*/
	                                                              std::vector, /*external container*/
	                                                              2,           /*external size*/
	                                                              100,         /*internal size min*/
	                                                              200,         /*internal size max*/
	                                                              0.0,         /*double min*/
	                                                              1.0,         /*double max*/
	                                                              emplace_back /*emplace_back method name*/)

	std::size_t const expected_size{array[0].size() + array[1].size()};
	std::vector<double> result_kway_merge = merge_arrays(array);

	REQUIRE(result_kway_merge.size() == expected_size);
	REQUIRE(std::is_sorted(result_kway_merge.begin(), result_kway_merge.end()));
}

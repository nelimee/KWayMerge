#include "catch.hpp"

#include "merge.h"
#include "define.hpp"

TEST_CASE("comparison with std::merge", "[core][comparison][random]") {

	KWAYMERGE_TEST_BUILD_RANDOM_CONTAINER_OF_CONTAINER_OF_DOUBLES(array,       /*name*/
	                                                              std::vector, /*internal container*/
	                                                              std::vector, /*external container*/
	                                                              2,           /*external size*/
	                                                              100,         /*internal size min*/
	                                                              200,         /*internal size max*/
	                                                              0.0,         /*double min*/
	                                                              1.0,         /*double max*/
	                                                              emplace_back /*emplace_back method name*/)

	std::vector<double> result_std_merge(array[0].size() + array[1].size());
	std::merge(array[0].begin(), array[0].end(), array[1].begin(), array[1].end(), result_std_merge.begin());

	std::vector<double> result_kway_merge = merge_arrays(array);

	REQUIRE(std::is_sorted(result_std_merge.begin(), result_std_merge.end()));
	REQUIRE(std::is_sorted(result_kway_merge.begin(), result_kway_merge.end()));

	REQUIRE(std::equal(result_std_merge.begin(), result_std_merge.end(), result_kway_merge.begin()));
}

TEST_CASE("comparison with std::sort", "[core][comparison][random]") {

	KWAYMERGE_TEST_BUILD_RANDOM_CONTAINER_OF_CONTAINER_OF_DOUBLES(array,       /*name*/
	                                                              std::vector, /*internal container*/
	                                                              std::vector, /*external container*/
	                                                              2,           /*external size*/
	                                                              100,         /*internal size min*/
	                                                              200,         /*internal size max*/
	                                                              0.0,         /*double min*/
	                                                              1.0,         /*double max*/
	                                                              emplace_back /*emplace_back method name*/)

	std::vector<double> result_std_sort(array[0].size() + array[1].size());
	auto first_free = std::copy(array[0].begin(), array[0].end(), result_std_sort.begin());
	std::copy(array[1].begin(), array[1].end(), first_free);
	std::sort(result_std_sort.begin(), result_std_sort.end());

	std::vector<double> result_kway_merge = merge_arrays(array);

	REQUIRE(std::is_sorted(result_std_sort.begin(), result_std_sort.end()));
	REQUIRE(std::is_sorted(result_kway_merge.begin(), result_kway_merge.end()));

	REQUIRE(std::equal(result_std_sort.begin(), result_std_sort.end(), result_kway_merge.begin()));
}


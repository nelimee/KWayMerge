#include "catch.hpp"

#include "define.hpp"

#include "merge.h"

TEST_CASE("merging 2 arrays with 1 empty", "[core][empty][random]") {

	KWAYMERGE_TEST_BUILD_RANDOM_CONTAINER_OF_CONTAINER_OF_DOUBLES(array,       /*name*/
	                                                              std::vector, /*internal container*/
	                                                              std::vector, /*external container*/
	                                                              2,           /*external size*/
	                                                              100,         /*internal size min*/
	                                                              200,         /*internal size max*/
	                                                              0.0,         /*double min*/
	                                                              1.0,         /*double max*/
	                                                              emplace_back /*emplace_back method name*/)

	array[0].clear(); // The first array is empty

	std::size_t const expected_size{array[0].size() + array[1].size()};
	std::vector<double> result_kway_merge = merge_arrays(array);

	REQUIRE(result_kway_merge.size() == expected_size);
	REQUIRE(std::is_sorted(result_kway_merge.begin(), result_kway_merge.end()));
	REQUIRE(std::equal(array[1].begin(), array[1].end(), result_kway_merge.begin()));
}

TEST_CASE("merging 2 arrays with 2 empty", "[core][empty][random]") {

	KWAYMERGE_TEST_BUILD_RANDOM_CONTAINER_OF_CONTAINER_OF_DOUBLES(array,       /*name*/
	                                                              std::vector, /*internal container*/
	                                                              std::vector, /*external container*/
	                                                              2,           /*external size*/
	                                                              100,         /*internal size min*/
	                                                              200,         /*internal size max*/
	                                                              0.0,         /*double min*/
	                                                              1.0,         /*double max*/
	                                                              emplace_back /*emplace_back method name*/)

	array[0].clear(); // The first array is empty
	array[1].clear(); // The second array is empty

	std::vector<double> result_kway_merge = merge_arrays(array);

	REQUIRE(result_kway_merge.empty());
}

TEST_CASE("merging 100 arrays with randomized empty ones", "[core][empty][random]") {

	KWAYMERGE_TEST_BUILD_RANDOM_CONTAINER_OF_CONTAINER_OF_DOUBLES(array,       /*name*/
	                                                              std::vector, /*internal container*/
	                                                              std::vector, /*external container*/
	                                                              100,         /*external size*/
	                                                              10,          /*internal size min*/
	                                                              20,          /*internal size max*/
	                                                              0.0,         /*double min*/
	                                                              1.0,         /*double max*/
	                                                              emplace_back /*emplace_back method name*/)

	std::random_device r;
	std::default_random_engine random_engine(r());
	std::uniform_int_distribution<std::size_t> random_index(0, 100);

	std::size_t const number_of_arrays_to_empty{random_index(random_engine)};
	std::size_t i{0};
	while (i < number_of_arrays_to_empty) {
		std::size_t const random_i{random_index(random_engine)};
		if (!array[random_i].empty()) {
			array[random_i].clear();
			++i;
		}
	}

	std::vector<double> result_kway_merge = merge_arrays(array);

	auto add_size = [](std::size_t const &previous_size, std::vector<double> const &elem) {
		return previous_size + elem.size();
	};
	std::size_t const number_of_elements{
			std::accumulate(array.begin(), array.end(), static_cast<std::size_t>(0), add_size)};

	REQUIRE(result_kway_merge.size() == number_of_elements);
	REQUIRE(std::is_sorted(result_kway_merge.begin(), result_kway_merge.end()));

}

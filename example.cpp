
#include <chrono>
#include <random>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <forward_list>

#include "include/merge.h"
#include "tests/include/helpers.hpp"

int main() {

	constexpr unsigned vector_number{1U<<8U};
	constexpr unsigned min_double_per_vector{100000}, max_double_per_vector{200000};

	std::random_device r;
	std::default_random_engine random_engine(r());
	std::uniform_int_distribution<std::size_t> random_size(min_double_per_vector, max_double_per_vector);

	std::vector<std::vector<double>> arrays_vector;
	for(unsigned i{0}; i < vector_number; ++i)
		arrays_vector.emplace_back(randomly_sorted(random_size(random_engine)));

	std::cout << "Generation over!" << std::endl;

	auto const result_vector = merge_arrays(arrays_vector);

	return !std::is_sorted(result_vector.begin(), result_vector.end());
}

#ifndef KWAYMERGE_DEFINE_HPP
#define KWAYMERGE_DEFINE_HPP

#include <random>
#include <cstdint>

#include "helpers.hpp"


#define KWAYMERGE_TEST_BUILD_RANDOM_CONTAINER_OF_CONTAINER_OF_DOUBLES(CONTAINER_NAME, INTERNAL_CONTAINER, EXTERNAL_CONTAINER, EXTERNAL_SIZE, INTERNAL_SIZE_MIN, INTERNAL_SIZE_MAX, VALUE_MIN, VALUE_MAX) \
std::random_device r_##CONTAINER_NAME; \
std::default_random_engine random_engine_##CONTAINER_NAME(r_##CONTAINER_NAME()); \
std::uniform_int_distribution<std::size_t> random_size_##CONTAINER_NAME((INTERNAL_SIZE_MIN), (INTERNAL_SIZE_MAX)); \
EXTERNAL_CONTAINER<INTERNAL_CONTAINER<double>> CONTAINER_NAME; \
for(std::size_t i{0}; i < (EXTERNAL_SIZE); ++i) \
	(CONTAINER_NAME).emplace_back(randomly_sorted(random_size_##CONTAINER_NAME(random_engine_##CONTAINER_NAME), VALUE_MIN, VALUE_MAX)); \


#endif //KWAYMERGE_DEFINE_HPP
